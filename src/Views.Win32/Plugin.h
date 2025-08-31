/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/**
 * \brief Plugin functions used solely in the view.
 */
struct view_plugin_funcs {
    CHANGEWINDOW video_change_window = nullptr;
};

class Plugin {
public:
    /**
     * \brief Tries to create a plugin from the given path
     * \param path The path to a plugin
     * \return The operation status along with a pointer to the plugin. The pointer will be invalid if the first pair element isn't an empty string.
     */
    static std::pair<std::wstring, std::unique_ptr<Plugin>> create(std::filesystem::path path);

    Plugin() = default;
    ~Plugin();

    /**
     * \brief Opens the plugin configuration dialog
     */
    void config();

    /**
     * \brief Opens the plugin test dialog
     */
    void test();

    /**
     * \brief Opens the plugin about dialog
     */
    void about();

    /**
     * \brief Loads the plugin's exported functions into the globals and calls the initiate function.
     */
    void initiate();

    /**
     * \brief Gets the plugin's path
     */
    auto path() const
    {
        return m_path;
    }

    /**
     * \brief Gets the plugin's name
     */
    auto name() const
    {
        return m_name;
    }

    /**
     * \brief Gets the plugin's type
     */
    auto type() const
    {
        return m_type;
    }

    /**
     * \brief Gets the plugin's version
     */
    auto version() const
    {
        return m_version;
    }

private:
    std::filesystem::path m_path;
    std::string m_name;
    core_plugin_type m_type;
    uint16_t m_version;
    HMODULE m_module;
};

/**
 * Represents the result of a plugin discovery operation.
 */
typedef struct
{
    /**
     * The discovered plugins matching the plugin API surface.
     */
    std::vector<std::unique_ptr<Plugin>> plugins;

    /**
     * Vector of discovered plugins and their results.
     */
    std::vector<std::pair<std::filesystem::path, std::wstring>> results;

} t_plugin_discovery_result;

extern view_plugin_funcs g_view_plugin_funcs;

/// <summary>
/// Initializes dummy info used by per-plugin functions
/// </summary>
void setup_dummy_info();

/**
 * \brief A module providing utility functions related to plugins.
 */
namespace PluginUtil
{
    /**
     * \brief Discovers plugins in the given directory.
     * \param directory The directory to search for plugins in.
     * \return The plugin discovery result.
     */
    t_plugin_discovery_result discover_plugins(const std::filesystem::path& directory);

    /**
     * \brief Gets the extended function set for video plugins.
     */
    core_plugin_extended_funcs video_extended_funcs();
    /**
     * \brief Gets the extended function set for audio plugins.
     */
    core_plugin_extended_funcs audio_extended_funcs();
    /**
     * \brief Gets the extended function set for input plugins.
     */
    core_plugin_extended_funcs input_extended_funcs();
    /**
     * \brief Gets the extended function set for RSP plugins.
     */
    core_plugin_extended_funcs rsp_extended_funcs();

} // namespace PluginUtil
