﻿/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <ActionManager.h>
#include <lua/presenters/Presenter.h>

/**
 * \brief Represents a Lua rendering context.
 */
struct t_lua_rendering_context
{
    // The current presenter, or null
    Presenter *presenter{};

    // The Direct2D overlay control handle
    HWND d2d_overlay_hwnd{};

    // The GDI/GDI+ overlay control handle
    HWND gdi_overlay_hwnd{};

    bool has_gdi_content{};

    HDC gdi_front_dc{};

    // The DC for GDI/GDI+ drawings
    // This DC is special, since commands can be issued to it anytime and it's never cleared
    HDC gdi_back_dc{};

    // The bitmap for GDI/GDI+ drawings
    HBITMAP gdi_bmp{};

    // Dimensions of the drawing surfaces
    D2D1_SIZE_U dc_size{};

    // The DirectWrite factory, whose lifetime is the renderer's
    IDWriteFactory *dw_factory{};

    // The cache for DirectWrite text layouts
    MicroLRU::Cache<uint64_t, IDWriteTextLayout *> dw_text_layouts{};

    // The cache for DirectWrite text size measurements
    MicroLRU::Cache<uint64_t, DWRITE_TEXT_METRICS> dw_text_sizes{};

    // The stack of render targets. The top is used for D2D calls.
    std::stack<ID2D1RenderTarget *> d2d_render_target_stack{};

    // Pool of GDI+ images
    std::unordered_map<size_t, Gdiplus::Bitmap *> image_pool{};

    // Amount of generated images, just used to generate uids for image pool
    size_t image_pool_index{};

    // Whether to ignore create_renderer() and ensure_d2d_renderer_created() calls. Used to avoid tearing down and
    // re-creating a renderer when stopping a script.
    bool ignore_create_renderer{};

    HDC loadscreen_dc{};
    HBITMAP loadscreen_bmp{};

    HBRUSH brush{};
    HPEN pen{};
    HFONT font{};
    COLORREF col, bkcol{};
    int bkmode{};
};

/**
 * \brief Describes a Lua instance.
 */
struct t_lua_environment
{
    using destroying_func = std::function<void(const t_lua_environment *env)>;
    using print_func = std::function<void(const t_lua_environment *env, const std::wstring &text)>;

    std::filesystem::path path;
    lua_State *L;
    t_lua_rendering_context rctx;
    bool started{};

    // All the actions registered by the script. Stored so we can remove them when the script is destroyed.
    std::vector<ActionManager::action_path> registered_actions{};

    destroying_func destroying{};
    print_func print{};
};
