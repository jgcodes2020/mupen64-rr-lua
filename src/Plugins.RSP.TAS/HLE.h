/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#define S 1
#define S8 3

/*
 * Audio flags
 */

#define A_INIT 0x01
#define A_CONTINUE 0x00
#define A_LOOP 0x02
#define A_OUT 0x02
#define A_LEFT 0x02
#define A_RIGHT 0x00
#define A_VOL 0x04
#define A_RATE 0x00
#define A_AUX 0x08
#define A_NOAUX 0x00
#define A_MAIN 0x00
#define A_MIX 0x10

extern core_rsp_info rsp;

typedef struct
{
    unsigned long type;
    unsigned long flags;

    unsigned long ucode_boot;
    unsigned long ucode_boot_size;

    unsigned long ucode;
    unsigned long ucode_size;

    unsigned long ucode_data;
    unsigned long ucode_data_size;

    unsigned long dram_stack;
    unsigned long dram_stack_size;

    unsigned long output_buff;
    unsigned long output_buff_size;

    unsigned long data_ptr;
    unsigned long data_size;

    unsigned long yield_data_ptr;
    unsigned long yield_data_size;
} OSTask_t;

void jpg_uncompress(OSTask_t* task);

extern uint32_t inst1, inst2;
extern uint16_t AudioInBuffer, AudioOutBuffer, AudioCount;
extern uint16_t AudioAuxA, AudioAuxC, AudioAuxE;
extern uint32_t loopval; // Value set by A_SETLOOP : Possible conflict with SETVOLUME???
