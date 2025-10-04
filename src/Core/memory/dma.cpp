/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <CommonPCH.h>
#include "dma.h"
#include "flashram.h"
#include "memory.h"
#include "pif.h"
#include "savestates.h"
#include "summercart.h"
#include <Core.h>
#include <r4300/debugger.h>
#include <r4300/interrupt.h>
#include <r4300/macros.h>
#include <r4300/ops.h>
#include <r4300/r4300.h>
#include <r4300/rom.h>

static bool validate_dma()
{
    if (si_register.si_pif_addr_wr64b != 0x1FC007C0)
    {
        critical_stop(std::format(L"Unknown SI DMA PIF address {:#08x}", si_register.si_pif_addr_wr64b));
        return false;
    }

    return true;
}

void dma_pi_read()
{
    uint32_t longueur;
    int32_t i;

    if (pi_register.pi_cart_addr_reg >= 0x08000000 && pi_register.pi_cart_addr_reg < 0x08010000)
    {
        if (use_flashram != 1)
        {
            fseek(g_sram_file, 0, SEEK_SET);
            fread(sram, 1, 0x8000, g_sram_file);

            for (i = 0; i < (pi_register.pi_rd_len_reg & 0xFFFFFF) + 1; i++)
                sram[((pi_register.pi_cart_addr_reg - 0x08000000) + i) ^ S8] =
                    ((unsigned char *)rdram)[(pi_register.pi_dram_addr_reg + i) ^ S8];

            fseek(g_sram_file, 0, SEEK_SET);
            fwrite(sram, 1, 0x8000, g_sram_file);
            use_flashram = -1;
        }
        else
            dma_write_flashram();
    }
    else if (g_core->cfg->use_summercart)
    {
        longueur = (pi_register.pi_rd_len_reg & 0xFFFFFF) + 1;
        if (pi_register.pi_cart_addr_reg >= 0x1ffe0000 && pi_register.pi_cart_addr_reg < 0x1fff0000)
        {
            for (i = 0; i < longueur; i++)
            {
                uint32_t dram = (pi_register.pi_dram_addr_reg + i);
                uint32_t cart = (pi_register.pi_cart_addr_reg + i) - 0x1ffe0000;
                if (dram > 0x7FFFFF || cart > 0x1FFF) break;
                summercart.buffer[cart ^ S8] = ((char *)rdram)[dram ^ S8];
            }
        }
        else if (pi_register.pi_cart_addr_reg >= 0x10000000 && pi_register.pi_cart_addr_reg < 0x14000000 &&
                 summercart.cfg_rom_write)
        {
            for (i = 0; i < longueur; i++)
            {
                uint32_t dram = (pi_register.pi_dram_addr_reg + i);
                uint32_t cart = (pi_register.pi_cart_addr_reg + i) - 0x10000000;
                if (dram > 0x7FFFFF || cart > 0x3FFFFFF) break;
                rom[cart ^ S8] = ((char *)rdram)[dram ^ S8];
            }
        }
        pi_register.read_pi_status_reg |= 1;
        update_count();
        add_interrupt_event(PI_INT, longueur / 8);
        return;
    }
    else
        g_core->log_warn(L"unknown dma read");

    pi_register.read_pi_status_reg |= 1;
    update_count();
    add_interrupt_event(PI_INT, 0x1000 /*pi_register.pi_rd_len_reg*/);
}

void dma_pi_write()
{
    uint32_t longueur;
    int32_t i;

    if (pi_register.pi_cart_addr_reg < 0x10000000)
    {
        if (pi_register.pi_cart_addr_reg >= 0x08000000 && pi_register.pi_cart_addr_reg < 0x08010000)
        {
            if (use_flashram != 1)
            {
                fseek(g_sram_file, 0, SEEK_SET);
                fread(sram, 1, 0x8000, g_sram_file);

                for (i = 0; i < (pi_register.pi_wr_len_reg & 0xFFFFFF) + 1; i++)
                    ((unsigned char *)rdram)[(pi_register.pi_dram_addr_reg + i) ^ S8] =
                        sram[(((pi_register.pi_cart_addr_reg - 0x08000000) & 0xFFFF) + i) ^ S8];
                use_flashram = -1;
            }
            else
                dma_read_flashram();
        }
        else if (pi_register.pi_cart_addr_reg >= 0x06000000 && pi_register.pi_cart_addr_reg < 0x08000000)
        {
        }
        else
            g_core->log_warn(std::format(L"unknown dma write:{:#06x}", (int32_t)pi_register.pi_cart_addr_reg));

        pi_register.read_pi_status_reg |= 1;
        update_count();
        add_interrupt_event(PI_INT, /*pi_register.pi_wr_len_reg*/ 0x1000);

        return;
    }

    longueur = (pi_register.pi_wr_len_reg & 0xFFFFFF) + 1;

    if (g_core->cfg->use_summercart && pi_register.pi_cart_addr_reg >= 0x1ffe0000 &&
        pi_register.pi_cart_addr_reg < 0x1fff0000)
    {
        for (i = 0; i < longueur; i++)
        {
            uint32_t dram = (pi_register.pi_dram_addr_reg + i);
            uint32_t cart = (pi_register.pi_cart_addr_reg + i) - 0x1ffe0000;
            if (dram > 0x7FFFFF || cart > 0x1FFF) break;
            ((char *)rdram)[dram ^ S8] = summercart.buffer[cart ^ S8];
        }
        pi_register.read_pi_status_reg |= 1;
        update_count();
        add_interrupt_event(PI_INT, longueur / 8);
        return;
    }

    if (pi_register.pi_cart_addr_reg >= 0x1fc00000) // for paper mario
    {
        pi_register.read_pi_status_reg |= 1;
        update_count();
        add_interrupt_event(PI_INT, 0x1000);
        return;
    }

    i = (pi_register.pi_cart_addr_reg - 0x10000000) & 0x3FFFFFF;
    longueur = (i + longueur) > rom_size ? (rom_size - i) : longueur;
    longueur =
        (pi_register.pi_dram_addr_reg + longueur) > 0x7FFFFF ? (0x7FFFFF - pi_register.pi_dram_addr_reg) : longueur;

    if (i > rom_size || pi_register.pi_dram_addr_reg > 0x7FFFFF)
    {
        pi_register.read_pi_status_reg |= 3;
        update_count();
        add_interrupt_event(PI_INT, longueur / 8);
        return;
    }

    if (!interpcore)
    {
        for (i = 0; i < longueur; i++)
        {
            uint32_t rdram_address1 = pi_register.pi_dram_addr_reg + i + 0x80000000;
            uint32_t rdram_address2 = pi_register.pi_dram_addr_reg + i + 0xa0000000;

            ((unsigned char *)rdram)[(pi_register.pi_dram_addr_reg + i) ^ S8] =
                !g_ctx.dbg_get_dma_read_enabled()
                    ? 0xFF
                    : rom[(((pi_register.pi_cart_addr_reg - 0x10000000) & 0x3FFFFFF) + i) ^ S8];

            if (!invalid_code[rdram_address1 >> 12])
                if (blocks[rdram_address1 >> 12]->block[(rdram_address1 & 0xFFF) / 4].ops != NOTCOMPILED)
                    invalid_code[rdram_address1 >> 12] = 1;

            if (!invalid_code[rdram_address2 >> 12])
                if (blocks[rdram_address2 >> 12]->block[(rdram_address2 & 0xFFF) / 4].ops != NOTCOMPILED)
                    invalid_code[rdram_address2 >> 12] = 1;
        }
    }
    else
    {
        for (i = 0; i < longueur; i++)
        {
            ((unsigned char *)rdram)[(pi_register.pi_dram_addr_reg + i) ^ S8] =
                !g_ctx.dbg_get_dma_read_enabled()
                    ? 0xFF
                    : rom[(((pi_register.pi_cart_addr_reg - 0x10000000) & 0x3FFFFFF) + i) ^ S8];
        }
    }

    /*for (i=0; i<=((longueur+0x800)>>12); i++)
      invalid_code[(((pi_register.pi_dram_addr_reg&0xFFFFFF)|0x80000000)>>12)+i] = 1;*/

    if ((debug_count + core_Count) < 0x100000)
    {
        switch (CIC_Chip)
        {
        case 1:
        case 2:
        case 3:
        case 6:
            rdram[0x318 / 4] = 0x800000;
            break;
        case 5:
            rdram[0x3F0 / 4] = 0x800000;
            break;
        }
    }

    pi_register.read_pi_status_reg |= 3;
    update_count();
    add_interrupt_event(PI_INT, longueur / 8);
    return;
}

void dma_sp_write()
{
    int32_t i;
    if ((sp_register.sp_mem_addr_reg & 0x1000) > 0)
    {
        for (i = 0; i < ((sp_register.sp_rd_len_reg & 0xFFF) + 1); i++)
            ((unsigned char *)(SP_IMEM))[((sp_register.sp_mem_addr_reg & 0xFFF) + i) ^ S8] =
                ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF) + i) ^ S8];
    }
    else
    {
        for (i = 0; i < ((sp_register.sp_rd_len_reg & 0xFFF) + 1); i++)
            ((unsigned char *)(SP_DMEM))[((sp_register.sp_mem_addr_reg & 0xFFF) + i) ^ S8] =
                ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF) + i) ^ S8];
    }
}

void dma_sp_read()
{
    int32_t i;
    if ((sp_register.sp_mem_addr_reg & 0x1000) > 0)
    {
        for (i = 0; i < ((sp_register.sp_wr_len_reg & 0xFFF) + 1); i++)
            ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF) + i) ^ S8] =
                ((unsigned char *)(SP_IMEM))[((sp_register.sp_mem_addr_reg & 0xFFF) + i) ^ S8];
    }
    else
    {
        for (i = 0; i < ((sp_register.sp_wr_len_reg & 0xFFF) + 1); i++)
            ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF) + i) ^ S8] =
                ((unsigned char *)(SP_DMEM))[((sp_register.sp_mem_addr_reg & 0xFFF) + i) ^ S8];
    }
}

void dma_si_write()
{
    if (!validate_dma())
    {
        return;
    }
    if (!check_register_validity(&si_register))
    {
        critical_stop(L"Invalid SI register contents in dma_si_write");
        return;
    }
    for (int32_t i = 0; i < (64 / 4); i++) PIF_RAM[i] = std::byteswap(rdram[si_register.si_dram_addr / 4 + i]);
    update_pif_write();
    update_count();
    add_interrupt_event(SI_INT, /*0x100*/ 0x900);
}

void dma_si_read()
{
    if (!validate_dma())
    {
        return;
    }

    update_pif_read();

    if (!check_register_validity(&si_register))
    {
        critical_stop(L"Invalid SI register contents in dma_si_read");
        return;
    }

    for (int32_t i = 0; i < (64 / 4); i++) rdram[si_register.si_dram_addr / 4 + i] = std::byteswap(PIF_RAM[i]);

    if (!g_st_skip_dma) // st already did this, see savestates.cpp, we still copy pif ram tho because it has new inputs
    {
        update_count();
        add_interrupt_event(SI_INT, /*0x100*/ 0x900);
    }
    g_st_skip_dma = false;
}
