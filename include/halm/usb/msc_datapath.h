/*
 * halm/usb/msc_datapath.h
 * Copyright (C) 2017 xent
 * Project is distributed under the terms of the MIT License
 */

#ifndef HALM_USB_MSC_DATAPATH_H_
#define HALM_USB_MSC_DATAPATH_H_
/*----------------------------------------------------------------------------*/
#include <xcore/error.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/*----------------------------------------------------------------------------*/
struct Msc;
struct MscQueryHandler;
/*----------------------------------------------------------------------------*/
BEGIN_DECLS

enum Result datapathInit(struct MscQueryHandler *, struct Msc *,
    void (*)(struct Msc *));
void datapathDeinit(struct MscQueryHandler *);
enum Result datapathStatus(const struct MscQueryHandler *);

bool datapathReceiveControl(struct MscQueryHandler *, void *, size_t);
bool datapathSendResponseAndStatus(struct MscQueryHandler *,
    const void *, size_t, uint32_t, uint32_t, uint8_t);
bool datapathSendResponse(struct MscQueryHandler *,
    const void *, size_t);
bool datapathSendStatus(struct MscQueryHandler *,
    uint32_t, uint32_t, uint8_t);
bool datapathReceiveAndWriteData(struct MscQueryHandler *, void *, size_t,
    uint64_t, size_t);
bool datapathReadAndSendData(struct MscQueryHandler *, void *, size_t,
    uint64_t, size_t);

END_DECLS
/*----------------------------------------------------------------------------*/
#endif /* HALM_USB_MSC_DATAPATH_H_ */
