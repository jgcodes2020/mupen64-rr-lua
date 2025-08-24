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
     * \param text The text to display in the editbox.
     * \param caption The caption of the dialog window.
     * \return The text if the user clicked OK, or std::nullopt if the user clicked Cancel.
     */
    std::optional<std::wstring> show(const std::wstring& text, const std::wstring& caption);
} // namespace TextEditDialog
