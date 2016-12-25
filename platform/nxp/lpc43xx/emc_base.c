/*
 * emc_base.c
 * Copyright (C) 2016 xent
 * Project is distributed under the terms of the GNU General Public License v3.0
 */

#include <halm/platform/nxp/emc_base.h>
/*----------------------------------------------------------------------------*/
const struct PinGroupEntry emcAddressPins[] = {
    {
        /* EMC_A5:EMC_A7 are mapped to P1_0:P1_2 */
        .begin = PIN(PORT_1, 0),
        .end = PIN(PORT_1, 2),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_A13:EMC_A11 are mapped to P2_0:P2_2 */
        .begin = PIN(PORT_2, 0),
        .end = PIN(PORT_2, 2),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_A10 is mapped to P2_6 */
        .begin = PIN(PORT_2, 6),
        .end = PIN(PORT_2, 6),
        .channel = 0,
        .value = 2
    }, {
        /*
         * EMC_A9:EMC_A8 are mapped to P2_7:P2_8
         * EMC_A0:EMC_A4 are mapped to P2_9:P2_13
         */
        .begin = PIN(PORT_2, 7),
        .end = PIN(PORT_2, 13),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_A15:EMC_A14 are mapped to P6_7:P6_8 */
        .begin = PIN(PORT_6, 7),
        .end = PIN(PORT_6, 8),
        .channel = 0,
        .value = 1
    }, {
        /* EMC_A23 is mapped to PA_4 */
        .begin = PIN(PORT_A, 4),
        .end = PIN(PORT_A, 4),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_A17:EMC_A16 are mapped to PD_15:PD_16 */
        .begin = PIN(PORT_D, 15),
        .end = PIN(PORT_D, 16),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_A18:EMC_A22 are mapped to PE_0:PE_4 */
        .begin = PIN(PORT_E, 0),
        .end = PIN(PORT_E, 4),
        .channel = 0,
        .value = 3
    }, {
        .begin = 0,
        .end = 0
    }
};

const pinNumber emcAddressPinMap[] = {
    PIN(PORT_2, 9),  PIN(PORT_2, 10), PIN(PORT_2, 11), PIN(PORT_2, 12),
    PIN(PORT_2, 13), PIN(PORT_1, 0),  PIN(PORT_1, 1),  PIN(PORT_1, 2),
    PIN(PORT_2, 8),  PIN(PORT_2, 7),  PIN(PORT_2, 6),  PIN(PORT_2, 2),
    PIN(PORT_2, 1),  PIN(PORT_2, 0),  PIN(PORT_6, 8),  PIN(PORT_6, 7),
    PIN(PORT_D, 16), PIN(PORT_D, 15), PIN(PORT_E, 0),  PIN(PORT_E, 1),
    PIN(PORT_E, 2),  PIN(PORT_E, 3),  PIN(PORT_E, 4),  PIN(PORT_A, 4)
};

const struct PinGroupEntry emcControlPins[] = {
    {
        /* EMC_OE, EMC_BLS0, EMC_CS0, EMC_WE */
        .begin = PIN(PORT_1, 3),
        .end = PIN(PORT_1, 6),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_DYCS1, EMC_CKEOUT1 */
        .begin = PIN(PORT_6, 1),
        .end = PIN(PORT_6, 2),
        .channel = 0,
        .value = 1
    }, {
        /* EMC_CS1, EMC_CAS, EMC_RAS */
        .begin = PIN(PORT_6, 3),
        .end = PIN(PORT_6, 5),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_BLS1 */
        .begin = PIN(PORT_6, 6),
        .end = PIN(PORT_6, 6),
        .channel = 0,
        .value = 1
    }, {
        /* EMC_DYCS0, EMC_DQMOUT1, EMC_CKEOUT0, EMC_DQMOUT0 */
        .begin = PIN(PORT_6, 9),
        .end = PIN(PORT_6, 12),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_DQMOUT2, EMC_CKEOUT2 */
        .begin = PIN(PORT_D, 0),
        .end = PIN(PORT_D, 1),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_BLS3, EMC_CS3, EMC_CS2, EMC_BLS2, EMC_DYCS2 */
        .begin = PIN(PORT_D, 10),
        .end = PIN(PORT_D, 14),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_DQMOUT3, EMC_DYCS3, EMC_CKEOUT3 */
        .begin = PIN(PORT_E, 13),
        .end = PIN(PORT_E, 15),
        .channel = 0,
        .value = 3
    }, {
        /* EMC_CLK0, EMC_CLK1, EMC_CLK2, EMC_CLK3 */
        .begin = PIN(PORT_CLK, 0),
        .end = PIN(PORT_CLK, 3),
        .channel = 0,
        .value = 0
    }, {
        .begin = 0,
        .end = 0
    }
};

const struct EmcPinDescription emcControlPinMap = {
    .cas = PIN(PORT_6, 4),
    .oe  = PIN(PORT_1, 3),
    .ras = PIN(PORT_6, 5),
    .we  = PIN(PORT_1, 6),

    .bls = {
        PIN(PORT_1, 4),
        PIN(PORT_6, 6),
        PIN(PORT_D, 13),
        PIN(PORT_D, 10)
    },
    .ckeout = {
        PIN(PORT_6, 11),
        PIN(PORT_6, 2),
        PIN(PORT_D, 1),
        PIN(PORT_E, 15)
    },
    .clk = {
        PIN(PORT_CLK, 0),
        PIN(PORT_CLK, 1),
        PIN(PORT_CLK, 2),
        PIN(PORT_CLK, 3)
    },
    .cs = {
        PIN(PORT_1, 5),
        PIN(PORT_6, 3),
        PIN(PORT_D, 12),
        PIN(PORT_D, 11)
    },
    .dqmout = {
        PIN(PORT_6, 12),
        PIN(PORT_6, 10),
        PIN(PORT_D, 0),
        PIN(PORT_E, 13)
    },
    .dycs = {
        PIN(PORT_6, 9),
        PIN(PORT_6, 1),
        PIN(PORT_D, 14),
        PIN(PORT_E, 14)
    }
};

const struct PinGroupEntry emcDataPins[] = {
    {
        /* EMC_D0:EMC_D7 are mapped to P1_7:P1_14 */
        .begin = PIN(PORT_1, 7),
        .end = PIN(PORT_1, 14),
        .channel = 0,
        .value = 3
    }, {
        /*
         * EMC_D8:EMC_D11 are mapped to P5_4:P5_7
         * EMC_D12:EMC_D15 are mapped to P5_0:P5_3
         */
        .begin = PIN(PORT_5, 0),
        .end = PIN(PORT_5, 7),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_D16:EMC_D23 are mapped to PD_2:PD_9 */
        .begin = PIN(PORT_D, 2),
        .end = PIN(PORT_D, 9),
        .channel = 0,
        .value = 2
    }, {
        /* EMC_D24:EMC_D31 are mapped to PE_5:PE_12 */
        .begin = PIN(PORT_1, 5),
        .end = PIN(PORT_1, 6),
        .channel = 0,
        .value = 3
    }, {
        .begin = 0,
        .end = 0
    }
};

const pinNumber emcDataPinMap[] = {
    PIN(PORT_1, 7),  PIN(PORT_1, 8),  PIN(PORT_1, 9),  PIN(PORT_1, 10),
    PIN(PORT_1, 11), PIN(PORT_1, 12), PIN(PORT_1, 13), PIN(PORT_1, 14),
    PIN(PORT_5, 4),  PIN(PORT_5, 5),  PIN(PORT_5, 6),  PIN(PORT_5, 7),
    PIN(PORT_5, 0),  PIN(PORT_5, 1),  PIN(PORT_5, 2),  PIN(PORT_5, 3),
    PIN(PORT_D, 2),  PIN(PORT_D, 3),  PIN(PORT_D, 4),  PIN(PORT_D, 5),
    PIN(PORT_D, 6),  PIN(PORT_D, 7),  PIN(PORT_D, 8),  PIN(PORT_D, 9),
    PIN(PORT_E, 5),  PIN(PORT_E, 6),  PIN(PORT_E, 7),  PIN(PORT_E, 8),
    PIN(PORT_E, 9),  PIN(PORT_E, 10), PIN(PORT_E, 11), PIN(PORT_E, 12)
};
