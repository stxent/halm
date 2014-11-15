/*
 * sdmmc_base.c
 * Copyright (C) 2014 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <string.h>
#include <delay.h>
#include <platform/platform_defs.h>
#include <platform/nxp/sdmmc_base.h>
#include <platform/nxp/sdmmc_defs.h>
#include <platform/nxp/lpc43xx/clocking.h>
#include <platform/nxp/lpc43xx/system.h>
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *, const void *);
static void sdioDeinit(void *);
static enum result sdioCallback(void *, void (*)(void *), void *);
static enum result sdioGet(void *, enum ifOption, void *);
static enum result sdioSet(void *, enum ifOption, const void *);
static uint32_t sdioRead(void *, uint8_t *, uint32_t);
static uint32_t sdioWrite(void *, const uint8_t *, uint32_t);
/*----------------------------------------------------------------------------*/
static const struct InterfaceClass sdioTable = {
    .size = sizeof(struct SdmmcBase),
    .init = sdioInit,
    .deinit = sdioDeinit,

    .callback = sdioCallback,
    .get = sdioGet,
    .set = sdioSet,
    .read = sdioRead,
    .write = sdioWrite
};
/*----------------------------------------------------------------------------*/
const struct InterfaceClass * const SdmmcBase = &sdioTable;
/*----------------------------------------------------------------------------*/
#define FIFOSZ 32 //FIXME

#define OCR_ALL_READY           BIT(31)
#define OCR_HC_CCS              BIT(30)
#define OCR_VOLTAGE_RANGE_MSK   0x00FF8000

#define CARD_TYPE_SD    BIT(0)
#define CARD_TYPE_4BIT  BIT(1)
#define CARD_TYPE_8BIT  BIT(2)
#define CARD_TYPE_HC    (OCR_HC_CCS)

#define SD_SEND_IF_ARG          0x000001AA
#define SD_SEND_IF_ECHO_MSK     0x000000FF
#define SD_SEND_IF_RESP         0x000000AA
/*----------------------------------------------------------------------------*/
#define CMD_MASK_RESP       (0x3UL<<28)
#define CMD_RESP(r)         (((r) & 0x3)<<28)
#define CMD_RESP_R0         (0<<28)
#define CMD_RESP_R1         (1<<28)
#define CMD_RESP_R2         (2<<28)
#define CMD_RESP_R3         (3<<28)
#define CMD_BIT_AUTO_STOP   (1<<24)
#define CMD_BIT_APP         (1<<23)
#define CMD_BIT_INIT        (1<<22)
#define CMD_BIT_BUSY        (1<<21)
#define CMD_BIT_LS          (1<<20) /* Low speed, used during acquire */
#define CMD_BIT_DATA        (1<<19)
#define CMD_BIT_WRITE       (1<<18)
#define CMD_BIT_STREAM      (1<<17)
#define CMD_MASK_CMD        (0xff)
#define CMD_SHIFT_CMD       (0)
/*----------------------------------------------------------------------------*/
bool sendLowLevelCommand(uint32_t cmd, uint32_t arg)
{
  int32_t tmo = 50;

  /* set command arg reg*/
  LPC_SDMMC->CMDARG = arg;
  LPC_SDMMC->CMD = CMD_START | cmd;

  /* Poll until command is accepted by the CIU */
  while (--tmo && (LPC_SDMMC->CMD & CMD_START))
    udelay(100);

  return tmo < 1;
}
/*----------------------------------------------------------------------------*/
void sdmmcSetClock(uint32_t speed)
{
  /* compute SD/MMC clock dividers */
  uint32_t div;

  div = ((clockFrequency(SdioClock) / speed) + 2) >> 1;

  if ((div == LPC_SDMMC->CLKDIV) && LPC_SDMMC->CLKENA)
    return; /* Closest speed is already set */

  /* disable clock */
  LPC_SDMMC->CLKENA = 0;

  /* User divider 0 */
  LPC_SDMMC->CLKSRC = CLKSRC_CLK_SOURCE(0);

  /* inform CIU */
  sendLowLevelCommand(CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);

  /* Set divider 0 to desired value */
  LPC_SDMMC->CLKDIV = (LPC_SDMMC->CLKDIV & CLKDIV_CLK_DIVIDER_MASK(0))
      | CLKDIV_CLK_DIVIDER(0, div);

  /* inform CIU */
  sendLowLevelCommand(CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);

  /* enable clock */
  LPC_SDMMC->CLKENA = CLKENA_CCLK_ENABLE;

  /* inform CIU */
  sendLowLevelCommand(CMD_UPDATE_CLOCK_REGISTERS
      | CMD_WAIT_PRVDATA_COMPLETE, 0);
}
/*----------------------------------------------------------------------------*/
static bool waitExit = false;
/*----------------------------------------------------------------------------*/
static uint32_t interruptHandler(uint32_t rinsts)
{
  irqDisable(SDIO_IRQ);
  waitExit = true;
  return 0;
}
/*----------------------------------------------------------------------------*/
void evSetup(uint32_t bits)
{
  irqClearPending(SDIO_IRQ);
  waitExit = false;
  LPC_SDMMC->INTMASK = bits;
  irqEnable(SDIO_IRQ);
}
/*----------------------------------------------------------------------------*/
static uint32_t waitCallback(uint32_t bits)
{
  uint32_t status;

  while (waitExit == 0);

  /* Get status and clear interrupts */
  status = LPC_SDMMC->RINTSTS;
  LPC_SDMMC->RINTSTS = status;
  LPC_SDMMC->INTMASK = 0;

  return status;
}
/*----------------------------------------------------------------------------*/
void getResponse(uint32_t *resp)
{
  resp[0] = LPC_SDMMC->RESP0;
  resp[1] = LPC_SDMMC->RESP1;
  resp[2] = LPC_SDMMC->RESP2;
  resp[3] = LPC_SDMMC->RESP3;
}
/*----------------------------------------------------------------------------*/
#define SD_INT_ERROR (INTMASK_RE | INTMASK_RCRC | INTMASK_DCRC | \
    INTMASK_RTO | INTMASK_DTO | INTMASK_HTO | INTMASK_FRUN | INTMASK_HLE | \
    INTMASK_SBE | INTMASK_EBE)
/*----------------------------------------------------------------------------*/
/* class 1 */
#define MMC_GO_IDLE_STATE         0   /* bc                          */
#define MMC_SEND_OP_COND          1   /* bcr  [31:0]  OCR        R3  */
#define MMC_ALL_SEND_CID          2   /* bcr                     R2  */
#define MMC_SET_RELATIVE_ADDR     3   /* ac   [31:16] RCA        R1  */
#define MMC_SET_DSR               4   /* bc   [31:16] RCA            */
#define MMC_SELECT_CARD           7   /* ac   [31:16] RCA        R1  */
#define MMC_SEND_EXT_CSD          8   /* bc                      R1  */
#define MMC_SEND_CSD              9   /* ac   [31:16] RCA        R2  */
#define MMC_SEND_CID             10   /* ac   [31:16] RCA        R2  */
#define MMC_STOP_TRANSMISSION    12   /* ac                      R1b */
#define MMC_SEND_STATUS          13   /* ac   [31:16] RCA        R1  */
#define MMC_GO_INACTIVE_STATE    15   /* ac   [31:16] RCA            */

/* class 2 */
#define MMC_SET_BLOCKLEN         16   /* ac   [31:0]  block len  R1  */
#define MMC_READ_SINGLE_BLOCK    17   /* adtc [31:0]  data addr  R1  */
#define MMC_READ_MULTIPLE_BLOCK  18   /* adtc [31:0]  data addr  R1  */

/* class 3 */
#define MMC_WRITE_DAT_UNTIL_STOP 20   /* adtc [31:0]  data addr  R1  */

/* class 4 */
#define MMC_SET_BLOCK_COUNT      23   /* adtc [31:0]  data addr  R1  */
#define MMC_WRITE_BLOCK          24   /* adtc [31:0]  data addr  R1  */
#define MMC_WRITE_MULTIPLE_BLOCK 25   /* adtc                    R1  */
#define MMC_PROGRAM_CID          26   /* adtc                    R1  */
#define MMC_PROGRAM_CSD          27   /* adtc                    R1  */

/* class 6 */
#define MMC_SET_WRITE_PROT       28   /* ac   [31:0]  data addr  R1b */
#define MMC_CLR_WRITE_PROT       29   /* ac   [31:0]  data addr  R1b */
#define MMC_SEND_WRITE_PROT      30   /* adtc [31:0]  wpdata addr R1  */

/* class 5 */
#define MMC_ERASE_GROUP_START    35   /* ac   [31:0]  data addr  R1  */
#define MMC_ERASE_GROUP_END      36   /* ac   [31:0]  data addr  R1  */
#define MMC_ERASE                37   /* ac                      R1b */

/* class 9 */
#define MMC_FAST_IO              39   /* ac   <Complex>          R4  */
#define MMC_GO_IRQ_STATE         40   /* bcr                     R5  */

/* class 7 */
#define MMC_LOCK_UNLOCK          42   /* adtc                    R1b */

/* class 8 */
#define MMC_APP_CMD              55   /* ac   [31:16] RCA        R1  */
#define MMC_GEN_CMD              56   /* adtc [0]     RD/WR      R1b */

/* SD commands                           type  argument     response */
/* class 8 */
/* This is basically the same command as for MMC with some quirks. */
#define SD_SEND_RELATIVE_ADDR     3   /* ac                      R6  */
#define SD_CMD8                   8   /* bcr  [31:0]  OCR        R3  */

/* Application commands */
#define SD_APP_SET_BUS_WIDTH      6   /* ac   [1:0]   bus width  R1   */
#define SD_APP_OP_COND           41   /* bcr  [31:0]  OCR        R1 (R4)  */
#define SD_APP_SEND_SCR          51   /* adtc                    R1   */

#define CMD(c,r)        ( ((c) &  CMD_MASK_CMD) | CMD_RESP((r)) )

#define CMD_IDLE            CMD(MMC_GO_IDLE_STATE,0) | CMD_BIT_LS    | CMD_BIT_INIT
#define CMD_SD_OP_COND      CMD(SD_APP_OP_COND,1)      | CMD_BIT_LS | CMD_BIT_APP
#define CMD_SD_SEND_IF_COND CMD(SD_CMD8,1)      | CMD_BIT_LS
#define CMD_MMC_OP_COND     CMD(MMC_SEND_OP_COND,3)    | CMD_BIT_LS | CMD_BIT_INIT
#define CMD_ALL_SEND_CID    CMD(MMC_ALL_SEND_CID,2)    | CMD_BIT_LS
#define CMD_MMC_SET_RCA     CMD(MMC_SET_RELATIVE_ADDR,1) | CMD_BIT_LS
#define CMD_SD_SEND_RCA     CMD(SD_SEND_RELATIVE_ADDR,1) | CMD_BIT_LS
#define CMD_SEND_CSD        CMD(MMC_SEND_CSD,2) | CMD_BIT_LS
#define CMD_SEND_EXT_CSD    CMD(MMC_SEND_EXT_CSD,1) | CMD_BIT_LS | CMD_BIT_DATA
#define CMD_DESELECT_CARD   CMD(MMC_SELECT_CARD,0)
#define CMD_SELECT_CARD     CMD(MMC_SELECT_CARD,1)
#define CMD_SET_BLOCKLEN    CMD(MMC_SET_BLOCKLEN,1)
#define CMD_SEND_STATUS     CMD(MMC_SEND_STATUS,1)
#define CMD_READ_SINGLE     CMD(MMC_READ_SINGLE_BLOCK,1) | CMD_BIT_DATA
#define CMD_READ_MULTIPLE   CMD(MMC_READ_MULTIPLE_BLOCK,1) | CMD_BIT_DATA | CMD_BIT_AUTO_STOP
#define CMD_SD_SET_WIDTH    CMD(SD_APP_SET_BUS_WIDTH,1)| CMD_BIT_APP
#define CMD_STOP            CMD(MMC_STOP_TRANSMISSION,1) | CMD_BIT_BUSY
#define CMD_WRITE_SINGLE    CMD(MMC_WRITE_BLOCK,1) | CMD_BIT_DATA | CMD_BIT_WRITE
#define CMD_WRITE_MULTIPLE  CMD(MMC_WRITE_MULTIPLE_BLOCK,1) | CMD_BIT_DATA | CMD_BIT_WRITE | CMD_BIT_AUTO_STOP

#define MMC_SECTOR_SIZE     512
/*----------------------------------------------------------------------------*/
static int32_t sdmmcExecuteCommand(struct SdmmcBase *device,
    uint32_t cmd, uint32_t arg, uint32_t wait_status)
{
  int32_t step = (cmd & CMD_BIT_APP) ? 2 : 1;
  int32_t status = 0;
  uint32_t cmd_reg = 0;

  if (!wait_status)
    wait_status = (cmd & CMD_MASK_RESP) ? INTMASK_CDONE : INTMASK_DTO;

  /* Clear the interrupts & FIFOs*/
  if (cmd & CMD_BIT_DATA)
  {
    /* reset all blocks */
    LPC_SDMMC->CTRL |= CTRL_FIFO_RESET;

    /* wait till resets clear */
    while (LPC_SDMMC->CTRL & CTRL_FIFO_RESET);

    /* Clear interrupt status */
    LPC_SDMMC->RINTSTS = 0xFFFFFFFF;
  }

  /* also check error conditions */
  wait_status |= INTMASK_EBE | INTMASK_SBE | INTMASK_HLE |
      INTMASK_RTO | INTMASK_RCRC | INTMASK_RE;
  if (wait_status & INTMASK_DTO)
    wait_status |= INTMASK_FRUN | INTMASK_HTO | INTMASK_DTO | INTMASK_DCRC;

  while (step)
  {
    sdmmcSetClock(cmd & CMD_BIT_LS ? 400000 : 400000);

    /* Clear the interrupts */
    LPC_SDMMC->RINTSTS = 0xFFFFFFFF;
    evSetup(wait_status);

    switch (step)
    {
      case 1: /* Execute command */
        cmd_reg =
            ((cmd & CMD_MASK_CMD) >> CMD_SHIFT_CMD)
            | ((cmd & CMD_BIT_INIT) ? CMD_SEND_INITIALIZATION : 0)
            | ((cmd & CMD_BIT_DATA) ? (CMD_DATA_EXPECTED | CMD_WAIT_PRVDATA_COMPLETE) : 0)
            | (((cmd & CMD_MASK_RESP) == CMD_RESP_R2) ? CMD_RESPONSE_LENGTH : 0)
            | ((cmd & CMD_MASK_RESP) ? CMD_RESPONSE_EXPECT : 0)
            | ((cmd & CMD_BIT_WRITE) ? CMD_READ_WRITE : 0)
            | ((cmd & CMD_BIT_STREAM) ? CMD_TRANSFER_MODE : 0)
            | ((cmd & CMD_BIT_BUSY) ? CMD_STOP_ABORT_CMD : 0)
            | ((cmd & CMD_BIT_AUTO_STOP) ? CMD_SEND_AUTO_STOP : 0)
            | CMD_START;

        /* Wait for previous data to finish for select/deselect commands */
        if (((cmd & CMD_MASK_CMD) >> CMD_SHIFT_CMD) == MMC_SELECT_CARD)
        {
          cmd_reg |= CMD_WAIT_PRVDATA_COMPLETE;
        }

        /* Wait for command to be accepted by CIU */
        if (sendLowLevelCommand(cmd_reg, arg) == 0)
          --step;
        break;

      case 0:
        return 0;

      case 2: /* APP prefix */
        cmd_reg = MMC_APP_CMD | CMD_RESPONSE_EXPECT |
            ((cmd & CMD_BIT_INIT) ? CMD_SEND_INITIALIZATION : 0) |
            CMD_START;

        if (sendLowLevelCommand(cmd_reg, device->rca << 16) == 0)
          --step;
        break;
    }

    /* Wait for command response */
    status = waitCallback(wait_status);

    /* Return an error if there is a timeout */
    if (status & SD_INT_ERROR)
      return status;

    if (status & INTMASK_CDONE)
    {
      switch (cmd & CMD_MASK_RESP)
      {
        case 0:
          break;

        case CMD_RESP_R1:
        case CMD_RESP_R3:
            getResponse(device->response);
          break;

        case CMD_RESP_R2:
          getResponse(device->response);
          break;
      }
    }
  }

  return 0;
}
/*----------------------------------------------------------------------------*/
static uint32_t prvGetBits(int32_t start, int32_t end, uint32_t *data)
{
  uint32_t v;
  uint32_t i = end >> 5;
  uint32_t j = start & 0x1F;

  if (i == start >> 5)
    v = data[i] >> j;
  else
    v = (data[i] << (32 - j)) | (data[start >> 5] >> j);

  return v & ((1 << (end - start + 1)) - 1);
}
/*----------------------------------------------------------------------------*/
static void processCsd(struct SdmmcBase *device)
{
  int32_t status = 0;
  int32_t c_size = 0;
  int32_t c_size_mult = 0;
  int32_t mult = 0;

  /* compute block length based on CSD response */
  device->block_len = 1 << prvGetBits(80, 83, device->csd);

  if ((device->card_type & CARD_TYPE_HC) &&
    (device->card_type & CARD_TYPE_SD))
  {
    /* See section 5.3.3 CSD Register (CSD Version 2.0) of SD2.0 spec
       an explanation for the calculation of these values */
    c_size = prvGetBits(48, 63, (uint32_t*)device->csd) + 1;
    device->blocknr = c_size << 10; /* 512 byte blocks */
  }
  else
  {
    /* See section 5.3 of the 4.1 revision of the MMC specs for
       an explanation for the calculation of these values */
    c_size = prvGetBits(62, 73, (uint32_t*)device->csd);
    c_size_mult = prvGetBits(47, 49, (uint32_t*)device->csd);
    mult = 1 << (c_size_mult + 2);
    device->blocknr = (c_size + 1) * mult;

    /* adjust blocknr to 512/block */
    if (device->block_len > MMC_SECTOR_SIZE)
      device->blocknr = device->blocknr *
        (device->block_len >> 9);

    /* get extended CSD for newer MMC cards CSD spec >= 4.0*/
    if (((device->card_type & CARD_TYPE_SD) == 0) &&
      (prvGetBits(122, 125, (uint32_t*)device->csd) >= 4))
    {
      /* put card in trans state */
      status = sdmmcExecuteCommand(device, CMD_SELECT_CARD, device->rca << 16, 0);

      /* set block size and byte count */
      LPC_SDMMC->BLKSIZ = MMC_SECTOR_SIZE;
      LPC_SDMMC->BYTCNT = MMC_SECTOR_SIZE;

      /* send EXT_CSD command */
//      sdif_dma_setup((uint32_t) device->ext_csd, MMC_SECTOR_SIZE); //FIXME
      status = sdmmcExecuteCommand(device, CMD_SEND_EXT_CSD, 0, 0 | INTMASK_DTO);
      if ((status & SD_INT_ERROR) == 0)
      {
        /* check EXT_CSD_VER is greater than 1.1 */
        if ((device->ext_csd[48] & 0xFF) > 1)
          device->blocknr = device->ext_csd[53]; /* bytes 212:215 represent sec count */

        /* switch to 52MHz clock if card type is set to 1 or else set to 26MHz */
        if ((device->ext_csd[49] & 0xFF) == 1)
        {
          /* for type 1 MMC cards high speed is 52MHz */
          device->speed = 400000;//MMC_HIGH_BUS_MAX_CLOCK;
        }
        else
        {
          /* for type 0 MMC cards high speed is 26MHz */
          device->speed = 400000;//MMC_LOW_BUS_MAX_CLOCK;
        }
      }
    }
  }

  device->device_size = device->blocknr << 9; /* blocknr * 512 */
}
/*----------------------------------------------------------------------------*/
int32_t cardAcquire(struct SdmmcBase *device)
{
  int32_t status;
  int32_t tries = 0;
  uint32_t ocr = OCR_VOLTAGE_RANGE_MSK;
  uint32_t r;
  int32_t state = 0;
  uint32_t command = 0;

  device->block_len = 0;
  device->blocknr = 0;
  device->card_type = 0;
  device->rca = 0;
  device->speed = 0;
  device->device_size = 0;
  memset(device->response, 0, sizeof(device->response));
  memset(device->cid, 0, sizeof(device->cid));
  memset(device->csd, 0, sizeof(device->csd));
  memset(device->ext_csd, 0, sizeof(device->ext_csd));

  /* clear card type */
  LPC_SDMMC->CTYPE = 0;

  device->speed = 400000;

  status = sdmmcExecuteCommand(device, CMD_IDLE, 0, INTMASK_CDONE);

  while (state < 100)
  {
    switch (state)
    {
      case 0:     /* Setup for SD */
        /* check if it is SDHC card */
        status = sdmmcExecuteCommand(device, CMD_SD_SEND_IF_COND, SD_SEND_IF_ARG, 0);
        if (!(status & INTMASK_RTO))
        {
          /* check response has same echo pattern */
          if ((device->response[0] & SD_SEND_IF_ECHO_MSK) == SD_SEND_IF_RESP)
            ocr |= OCR_HC_CCS;
        }

        ++state;
        command = CMD_SD_OP_COND;
        tries = 50; //FIXME Retries

        /* assume SD card */
        device->card_type |= CARD_TYPE_SD;
        device->speed = 400000;//FIXME SD_MAX_CLOCK;
        break;

//      case 10:      /* Setup for MMC */
//        /* start fresh for MMC crds */
//        device->card_type &= ~CARD_TYPE_SD;
//        status = sdmmcExecuteCommand(device, CMD_IDLE, 0, MCI_INT_CMD_DONE);
//        command = CMD_MMC_OP_COND;
//        tries = INIT_OP_RETRIES;
//        ocr |= OCR_HC_CCS;
//        ++state;
//
//        /* for MMC cards high speed is 20MHz */
//        device->speed = MMC_MAX_CLOCK;
//        break;

      case 1:
      case 11:
        status = sdmmcExecuteCommand(device, command, 0, 0);
        if (status & INTMASK_RTO)
          state += 9;   /* Mode unavailable */
        else
          ++state;
        break;

      case 2:     /* Initial OCR check  */
      case 12:
        ocr = device->response[0] | (ocr & OCR_HC_CCS);
        if (ocr & OCR_ALL_READY)
          ++state;
        else
          state += 2;
        break;

      case 3:     /* Initial wait for OCR clear */
      case 13:
        while ((ocr & OCR_ALL_READY) && --tries > 0)
        {
          mdelay(10); //FIXME MS_ACQUIRE_DELAY
          status = sdmmcExecuteCommand(device, command, 0, 0);
          ocr = device->response[0] | (ocr & OCR_HC_CCS);
        }
        if (ocr & OCR_ALL_READY)
          state += 7;
        else
          ++state;
        break;

      case 14:
        /* for MMC cards set high capacity bit */
        ocr |= OCR_HC_CCS;
      case 4:     /* Assign OCR */
        tries = 1000; //SET_OP_RETRIES;
        ocr &= OCR_VOLTAGE_RANGE_MSK | OCR_HC_CCS;  /* Mask for the bits we care about */
        do
        {
          mdelay(10); //FIXME MS_ACQUIRE_DELAY
          status = sdmmcExecuteCommand(device, command, ocr, 0);
          r = device->response[0];
        }
        while (!(r & OCR_ALL_READY) && --tries > 0);

        if (r & OCR_ALL_READY)
        {
          /* is it high capacity card */
          device->card_type |= (r & OCR_HC_CCS);
          ++state;
        }
        else
          state += 6;
        break;

      case 5:     /* CID polling */
      case 15:
        status = sdmmcExecuteCommand(device, CMD_ALL_SEND_CID, 0, 0);
        memcpy(device->cid, device->response, sizeof(device->cid));
        ++state;
        break;

      case 6:     /* RCA send, for SD get RCA */
        status = sdmmcExecuteCommand(device, CMD_SD_SEND_RCA, 0, 0);
        device->rca = (device->response[0]) >> 16; //FIXME Magic
        ++state;
        break;

        //TODO Remove for SD
      case 16:      /* RCA assignment for MMC set to 1 */
        device->rca = 1;
        status = sdmmcExecuteCommand(device, CMD_MMC_SET_RCA, device->rca << 16, 0);
        ++state;
        break;

      case 7:
      case 17:
        status = sdmmcExecuteCommand(device, CMD_SEND_CSD, device->rca << 16, 0);
        memcpy(device->csd, device->response, sizeof(device->csd));
        state = 100;
        break;

      default:
        state += 100; /* break from while loop */
        break;
    }
  }

  /* Compute card size, block size and no. of blocks
     based on CSD response received. */
  if (device->cid[0] != 0) { //prv_card_acquired
    processCsd(device);

//    /* Setup card data width and block size (once) */
//    if (prv_set_trans_state() != 0)
//      return 0;
//    if (prv_set_card_params() != 0)
//      return 0;
  }

  return device->cid[0] != 0;
}
/*----------------------------------------------------------------------------*/
void SDIO_ISR(void)
{
  interruptHandler(LPC_SDMMC->RINTSTS);
}
/*----------------------------------------------------------------------------*/
static enum result sdioInit(void *object, const void *configBase)
{
  const struct SdmmcBaseConfig * const config = configBase;
  struct SdmmcBase * const device = object;
  enum result res;

  device->position = 0;

  struct Pin pin;

  pin = pinInit(PIN(PORT_1, 6)); //CMD
  pinInput(pin);
  pinSetFunction(pin, 7);

  pin = pinInit(PIN(PORT_1, 9)); //DAT0
  pinInput(pin);
  pinSetFunction(pin, 7);

  pin = pinInit(PIN(PORT_1, 10)); //DAT1
  pinInput(pin);
  pinSetFunction(pin, 7);

  pin = pinInit(PIN(PORT_1, 11)); //DAT2
  pinInput(pin);
  pinSetFunction(pin, 7);

  pin = pinInit(PIN(PORT_1, 12)); //DAT3
  pinInput(pin);
  pinSetFunction(pin, 7);

  pin = pinInit(PIN(PORT_CLK, 0)); //CLK
  pinInput(pin);
  pinSetFunction(pin, 4);

  sysClockEnable(CLK_M4_SDIO);
  sysClockEnable(CLK_SDIO);
  sysResetEnable(RST_SDIO);

  /* Software reset */
  LPC_SDMMC->BMOD = BMOD_SWR;

  /* Reset all blocks */
  const uint32_t resetMask = CTRL_CONTROLLER_RESET | CTRL_FIFO_RESET
      | CTRL_DMA_RESET;

  LPC_SDMMC->CTRL = resetMask;
  while (LPC_SDMMC->CTRL & resetMask);

  /* Internal DMA setup */
  LPC_SDMMC->CTRL = CTRL_USE_INTERNAL_DMAC | CTRL_INT_ENABLE;
  LPC_SDMMC->INTMASK = 0;

  LPC_SDMMC->RINTSTS = 0xFFFFFFFF;
  LPC_SDMMC->TMOUT = 0xFFFFFFFF;

  LPC_SDMMC->FIFOTH = FIFOTH_TX_WMARK(FIFOSZ / 2)
      | FIFOTH_RX_WMARK(FIFOSZ / 2 - 1) | FIFOTH_DMA_MTS(BURST_SIZE_4);

  /* Enable internal DMA */
  LPC_SDMMC->BMOD = BMOD_DE | BMOD_DSL(4) | BMOD_PBL(BURST_SIZE_4);

  LPC_SDMMC->CLKENA = 0;
  LPC_SDMMC->CLKSRC = 0;

  //FIXME
  /* Enable power */
//  LPC_SDMMC->PWREN = 1;

  cardAcquire(device);

  return E_OK;
}
/*----------------------------------------------------------------------------*/
static void sdioDeinit(void *object __attribute__((unused)))
{

}
/*----------------------------------------------------------------------------*/
static enum result sdioCallback(void *object __attribute__((unused)),
    void (*callback)(void *) __attribute__((unused)),
    void *argument __attribute__((unused)))
{
  return E_ERROR;
}
/*----------------------------------------------------------------------------*/
static enum result sdioGet(void *object, enum ifOption option, void *data)
{
  struct SdmmcBase * const device = object;

  switch (option)
  {
    case IF_ADDRESS:
      *(uint64_t *)data = device->position;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static enum result sdioSet(void *object, enum ifOption option,
    const void *data)
{
  struct SdmmcBase * const device = object;

  switch (option)
  {
    case IF_ADDRESS:
      /* TODO Add boundary check */
      device->position = *(const uint64_t *)data;
      return E_OK;

    default:
      return E_ERROR;
  }
}
/*----------------------------------------------------------------------------*/
static uint32_t sdioRead(void *object, uint8_t *buffer, uint32_t length)
{
  struct SdmmcBase * const device = object;


}
/*----------------------------------------------------------------------------*/
static uint32_t sdioWrite(void *object, const uint8_t *buffer, uint32_t length)
{
  struct SdmmcBase * const device = object;


}
