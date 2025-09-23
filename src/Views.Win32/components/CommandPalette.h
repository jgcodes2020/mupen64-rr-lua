/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/**
 * \brief A module responsible for implementing a command palette.
 */
namespace CommandPalette
{
/**
 * \brief Shows the command palette.
 */
void show();

/**
 * \brief Gets the HWND of the command palette window. Might be invalid.
 */
HWND hwnd();
} // namespace CommandPalette
