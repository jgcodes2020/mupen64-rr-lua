/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stdafx.h"
#include <Core.h>
#include <r4300/cop1_helpers.h>
#include <r4300/r4300.h>
#include <r4300/recomph.h>
#include <r4300/x86/assemble.h>
#include <r4300/x86/gcop1_helpers.h>

static void patch_jump(uint32_t addr, uint32_t target)
{
    int32_t diff = target - addr;
    assert(-128 <= diff && diff < 128);
    (*inst_pointer)[addr - 1] = (unsigned char)(diff & 0xFF);
}

static void gencall_noret(void (*fn)())
{
    mov_m32_imm32((uint32_t *)(&PC), (uint32_t)(dst));
    mov_reg32_imm32(EAX, (uint32_t)fn);
    call_reg32(EAX);
    ud2();
}

/**
 * Given the fp stack:
 * ST(0) = x
 * ST(1) = largest denormal (float/double)
 * Assert that x is not denormal or nan, ending with ST(0) and ST(1) popped.
 *
 * If the assert fires, an additional 'stackBase' values are popped.
 */
void gencheck_float_input_valid(int32_t stackBase)
{
    // if abs(x) > largest denormal, goto A
    fabs_();         // ST(0) = abs(ST(0))
    fucomi_fpreg(1); // compare ST(0) <=> ST(1)
    fstp_fpreg(1);   // pop ST(1)
    ja_rj(0);
    uint32_t jump1 = code_length;

    // if abs(x) == 0, goto A
    fldz();           // push zero
    fucomip_fpreg(1); // compare ST(0) <=> ST(1), pop
    je_rj(0);
    uint32_t jump2 = code_length;

    for (int32_t i = 0; i < stackBase + 1; i++) fstp_fpreg(0); // pop
    gencall_noret(fail_float_input);

    // A:
    patch_jump(jump1, code_length);
    patch_jump(jump2, code_length);
    fstp_fpreg(0); // pop
}

/**
 * Given the fp stack:
 * ST(0) = largest denormal (float/double)
 * ST(1) = x
 * Assert that x is not nan, and flush denormal x to zero, leaving a
 * replacement value on the fp stack.
 */
void gencheck_float_output_valid()
{
    // if abs(x) > largest denormal, goto DONE
    fld_fpreg(1);     // duplicate ST(1)
    fabs_();          // ST(0) = abs(ST(0))
    fucomip_fpreg(1); // compare ST(0) <=> ST(1), pop
    fstp_fpreg(0);    // pop
    ja_rj(0);
    uint32_t jump1 = code_length;

    jp_rj(0); // if unordered (i.e. x is nan), goto FAIL
    uint32_t jump2 = code_length;

    // Replace the (denormal or zero) result by zero (see CHECK_OUTPUT in
    // cop1_helpers.h for reasoning)

    fldz();           // push zero
    fucomip_fpreg(1); // compare ST(0) <=> ST(1), pop

    je_rj(0); // if equal (x = 0 or -0), goto DONE
    uint32_t jump3 = code_length;

    ja_rj(0); // if 0 > x, goto NEGATIVE
    uint32_t jump4 = code_length;

    // POSITIVE:
    fstp_fpreg(0);    // pop
    fldz();           // push zero
    jmp_imm_short(0); // goto DONE
    uint32_t jump5 = code_length;

    // FAIL:
    patch_jump(jump2, code_length);
    fstp_fpreg(0); // pop
    gencall_noret(fail_float_output);

    // NEGATIVE:
    patch_jump(jump4, code_length);
    fstp_fpreg(0); // pop
    fldz();        // push zero
    fchs();        // negate it

    // DONE:
    patch_jump(jump1, code_length);
    patch_jump(jump3, code_length);
    patch_jump(jump5, code_length);
}

/**
 * Assert that the last x87 operation did not result in a (masked) Invalid
 * Operation exception. fclex must be called immediately before the operation.
 *
 * Clobbers eax.
 */
void gencheck_float_conversion_valid()
{
    if (!g_core->cfg->float_exception_emulation) return;

    fstsw_ax();
    test_al_imm8(1); // Invalid Operation bit
    je_rj(0);        // jump if not set (i.e. ZF = 1)
    uint32_t jump1 = code_length;

    gencall_noret(fail_float_convert);

    patch_jump(jump1, code_length);
}
