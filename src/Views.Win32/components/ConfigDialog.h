/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/**
 * \brief A module responsible for implementing configuration dialogs.
 */
namespace ConfigDialog
{
    /**
     * \brief Initializes the subsystem.
     */
    void init();

    /**
     * \brief Shows the application settings dialog.
     */
    void show_app_settings();
} // namespace ConfigDialog
