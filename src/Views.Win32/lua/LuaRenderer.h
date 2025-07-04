﻿#pragma once

/**
 * \brief A module responsible for implementing Lua rendering-related functionality.
 */
namespace LuaRenderer
{
    constexpr uint32_t LUA_GDI_COLOR_MASK = RGB(255, 0, 255);

    /**
     * \brief Initializes the subsystem.
     */
    void init();

    /**
     * \brief Creates a new rendering context with the default values.
     */
    t_lua_rendering_context default_rendering_context();

    /**
     * \brief Invalidates all visual layers of all running Lua scripts.
     * \remarks Must be called from the UI thread.
     */
    void invalidate_visuals();

    /**
     * \brief Forces an immediate repaint of all visual layers of all running Lua scripts.
     * \remarks Must be called from the UI thread.
     */
    void repaint_visuals();

    /**
     * \brief Initializes a Lua rendering context. Does nothing if the renderer is initialized.
     */
    void create_renderer(t_lua_rendering_context*, t_lua_environment*);

    /**
     * \brief Prepares a Lua rendering context for deinitialization. Does nothing if the renderer isn't initialized.
     */
    void pre_destroy_renderer(t_lua_rendering_context*);

    /**
     * \brief Deinitializes a Lua rendering context. Does nothing if the renderer isn't initialized.
     */
    void destroy_renderer(t_lua_rendering_context*);

    /**
     * \brief Ensures that the D2D renderer is created for a Lua environment. Does nothing if the renderer already exists.
     */
    void ensure_d2d_renderer_created(t_lua_rendering_context*);

    /**
     * \brief Tells the renderer that GDI content is present in the rendering context.
     */
    void mark_gdi_content_present(t_lua_rendering_context*);

    /**
     * \brief Resets the loadscreen graphics.
     */
    void loadscreen_reset(t_lua_rendering_context*);

    /**
     * \brief Gets a brush containing a color that, when drawn to the GDI back dc, will be interpreted as an alpha mask by the renderer.
     */
    HBRUSH alpha_mask_brush();
} // namespace LuaRenderer
