/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stdafx.h"
#include <Core.h>
#include <r4300/r4300.h>
#include <r4300/macros.h>
#include <r4300/ops.h>
#include <r4300/interrupt.h>

void MFC0()
{
    switch (PC->f.r.nrd)
    {
    case 1:
        critical_stop(L"MFC0 unimplemented operation");
        break;
    default:
        rrt32 = reg_cop0[PC->f.r.nrd];
        sign_extended(core_rrt);
    }
    PC++;
}

void MTC0()
{
    switch (PC->f.r.nrd)
    {
    case 0: // Index
        core_Index = core_rrt & 0x8000003F;
        if ((core_Index & 0x3F) > 31)
        {
            critical_stop(L"TLB Index too high in MTC0");
        }
        break;
    case 1: // Random
        break;
    case 2: // EntryLo0
        core_EntryLo0 = core_rrt & 0x3FFFFFFF;
        break;
    case 3: // EntryLo1
        core_EntryLo1 = core_rrt & 0x3FFFFFFF;
        break;
    case 4: // Context
        core_Context = (core_rrt & 0xFF800000) | (core_Context & 0x007FFFF0);
        break;
    case 5: // PageMask
        core_PageMask = core_rrt & 0x01FFE000;
        break;
    case 6: // Wired
        core_Wired = core_rrt;
        core_Random = 31;
        break;
    case 8: // BadVAddr
        break;
    case 9: // Count
        update_count();
        if (next_interrupt <= core_Count)
            gen_interrupt();
        debug_count += core_Count;
        translate_event_queue(core_rrt & 0xFFFFFFFF);
        core_Count = core_rrt & 0xFFFFFFFF;
        debug_count -= core_Count;
        break;
    case 10: // EntryHi
        core_EntryHi = core_rrt & 0xFFFFE0FF;
        break;
    case 11: // Compare
        update_count();
        remove_event(COMPARE_INT);
        add_interrupt_event_count(COMPARE_INT, (uint32_t)core_rrt);
        core_Compare = core_rrt;
        core_Cause = core_Cause & 0xFFFF7FFF; // Timer interrupt is clear
        break;
    case 12: // Status
        if ((core_rrt & 0x04000000) != (core_Status & 0x04000000))
        {
            if (core_rrt & 0x04000000)
            {
                int32_t i;
                for (i = 0; i < 32; i++)
                {
                    reg_cop1_double[i] = (double*)&reg_cop1_fgr_64[i];
                    reg_cop1_simple[i] = (float*)&reg_cop1_fgr_64[i];
                }
            }
            else
            {
                int32_t i;
                for (i = 0; i < 32; i++)
                {
                    if (!(i & 1))
                        reg_cop1_double[i] = (double*)&reg_cop1_fgr_64[i >> 1];
                    reg_cop1_simple[i] = (float*)&reg_cop1_fgr_64[i >> 1] + (i & 1);
                }
            }
        }
        core_Status = core_rrt;
        PC++;
        check_interrupt();
        update_count();
        if (next_interrupt <= core_Count)
            gen_interrupt();
        PC--;
        break;
    case 13: // Cause
        if (core_rrt == 0)
        {
            core_Cause = core_rrt;
        }
        else
        {
            critical_stop(L"MTC0 core_rrt != 0 cause");
        }
        break;
    case 14: // EPC
        core_EPC = core_rrt;
        break;
    case 15: // PRevID
        break;
    case 16: // Config
        core_Config_cop0 = core_rrt;
        break;
    case 18: // WatchLo
        core_WatchLo = core_rrt & 0xFFFFFFFF;
        break;
    case 19: // WatchHi
        core_WatchHi = core_rrt & 0xFFFFFFFF;
        break;
    case 27: // CacheErr
        break;
    case 28: // TagLo
        core_TagLo = core_rrt & 0x0FFFFFC0;
        break;
    case 29: // TagHi
        core_TagHi = 0;
        break;
    default:
        critical_stop(std::format(L"unknown mtc0 write : {}\n", PC->f.r.nrd));
        break;
    }
    PC++;
}
