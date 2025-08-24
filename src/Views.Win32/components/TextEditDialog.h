/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/**
 * \brief A module responsible for implementing a text editing dialog.
 */
namespace TextEditDialog
{
    /**
     * \brief Shows the about dialog with the specified text.
     * \param text The text to display in the about dialog.
     * \return The text if the user clicked OK, or std::nullopt if the user clicked Cancel.
     */
    std::optional<std::wstring> show(std::wstring text);
} // namespace TextEditDialog
