/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <core_types.h>
#include <Hotkey.h>

struct t_config
{

    enum class PresenterType
    {
        DirectComposition,
        GDI
    };

    enum class EncoderType
    {
        VFW,
        FFmpeg
    };

    enum class StatusbarLayout
    {
        Classic,
        Modern,
        ModernWithReadOnly
    };

    /// <summary>
    /// The core config.
    /// </summary>
    core_cfg core{};

    /// <summary>
    /// The new version of Mupen64 currently ignored by the update checker.
    /// <para></para>
    /// L"" means no ignored version.
    /// </summary>
    std::wstring ignored_version;

    /// <summary>
    /// The current savestate slot index (0-9).
    /// </summary>
    int32_t st_slot;

    /// <summary>
    /// Whether emulation will pause when the main window loses focus
    /// </summary>
    int32_t is_unfocused_pause_enabled;

    /// <summary>
    /// Whether the statusbar is enabled
    /// </summary>
    int32_t is_statusbar_enabled = 1;

    /// <summary>
    /// Whether the statusbar is allowed to scale its segments down.
    /// </summary>
    int32_t statusbar_scale_down = 1;

    /// <summary>
    /// Whether the statusbar is allowed to scale its segments up.
    /// </summary>
    int32_t statusbar_scale_up;

    /// <summary>
    /// The statusbar layout.
    /// </summary>
    int32_t statusbar_layout = (int32_t)StatusbarLayout::Modern;

    std::wstring rom_directory = L".\\roms\\";
    std::wstring plugins_directory = L".\\plugin\\";
    std::wstring saves_directory = L".\\save\\";
    std::wstring screenshots_directory = L".\\screenshots\\";
    std::wstring backups_directory = L".\\backups\\";

    /// <summary>
    /// The recently opened roms' paths
    /// </summary>
    std::vector<std::wstring> recent_rom_paths;

    /// <summary>
    /// Whether recently opened rom path collection is paused
    /// </summary>
    int32_t is_recent_rom_paths_frozen;

    /// <summary>
    /// The recently opened movies' paths
    /// </summary>
    std::vector<std::wstring> recent_movie_paths;

    /// <summary>
    /// Whether recently opened movie path collection is paused
    /// </summary>
    int32_t is_recent_movie_paths_frozen;

    /// <summary>
    /// Whether the rom browser will recursively search for roms beginning in the specified directories
    /// </summary>
    int32_t is_rombrowser_recursion_enabled;

    /// <summary>
    /// The strategy to use when capturing video
    /// <para/>
    /// 0 = Use the video plugin's readScreen or read_video (MGE)
    /// 1 = Internal capture of window
    /// 2 = Internal capture of screen cropped to window
    /// 3 = Use the video plugin's readScreen or read_video (MGE) composited with lua scripts
    /// </summary>
    int32_t capture_mode = 3;

    int32_t stop_capture_at_movie_end;

    /// <summary>
    /// The presenter to use for Lua scripts
    /// </summary>
    int32_t presenter_type = (int32_t)PresenterType::DirectComposition;

    /// <summary>
    /// Enables lazy Lua renderer initialization. Greatly speeds up start and stop times for certain scripts.
    /// </summary>
    int32_t lazy_renderer_init = 1;

    /// <summary>
    /// The encoder to use for capturing.
    /// </summary>
    int32_t encoder_type = (int32_t)EncoderType::VFW;

    /// <summary>
    /// The delay (in milliseconds) before capturing the window
    /// <para/>
    /// May be useful when capturing other windows alongside mupen
    /// </summary>
    int32_t capture_delay;

    /// <summary>
    /// FFmpeg post-stream option format string which is used when capturing using the FFmpeg encoder type
    /// </summary>
    std::wstring ffmpeg_final_options =
        L"-y -f rawvideo -pixel_format bgr24 -video_size %dx%d -framerate %d -i %s "
        L"-f s16le -sample_rate %d -ac 2 -channel_layout stereo -i %s "
        L"-c:v libx264 -preset veryfast -tune zerolatency -crf 23 -c:a aac -b:a 128k -vf \"vflip\" -f mp4 %s";

    /// <summary>
    /// FFmpeg binary path
    /// </summary>
    std::wstring ffmpeg_path = L"C:\\ffmpeg\\bin\\ffmpeg.exe";

    /// <summary>
    /// The audio-video synchronization mode
    /// <para/>
    /// 0 - No Sync
    /// 1 - Audio Sync
    /// 2 - Video Sync
    /// </summary>
    int32_t synchronization_mode = 1;

    /// <summary>
    /// When enabled, mupen won't change the working directory to its current path at startup
    /// </summary>
    int32_t keep_default_working_directory;

    /// <summary>
    /// Whether a low-latency dispatcher implementation is used. Greatly improves performance when Lua scripts are
    /// running. Disable if you DirectInput-based plugins aren't working as expected.
    /// </summary>
    int32_t fast_dispatcher = 1;

    /// <summary>
    /// The lua script path
    /// </summary>
    std::wstring lua_script_path;

    /// <summary>
    /// The recently opened lua scripts' paths
    /// </summary>
    std::vector<std::wstring> recent_lua_script_paths;

    /// <summary>
    /// Whether recently opened lua script path collection is paused
    /// </summary>
    int32_t is_recent_scripts_frozen;

    /// <summary>
    /// Whether piano roll edits are constrained to the column they started on
    /// </summary>
    int32_t piano_roll_constrain_edit_to_column;

    /// <summary>
    /// Maximum size of the undo/redo stack.
    /// </summary>
    int32_t piano_roll_undo_stack_size = 100;

    /// <summary>
    /// Whether the piano roll will try to keep the selection visible when the frame changes
    /// </summary>
    int32_t piano_roll_keep_selection_visible;

    /// <summary>
    /// Whether the piano roll will try to keep the playhead visible when the frame changes
    /// </summary>
    int32_t piano_roll_keep_playhead_visible;

    /// <summary>
    /// The path of the currently selected video plugin
    /// </summary>
    std::wstring selected_video_plugin;

    /// <summary>
    /// The path of the currently selected audio plugin
    /// </summary>
    std::wstring selected_audio_plugin;

    /// <summary>
    /// The path of the currently selected input plugin
    /// </summary>
    std::wstring selected_input_plugin;

    /// <summary>
    /// The path of the currently selected RSP plugin
    /// </summary>
    std::wstring selected_rsp_plugin;

    /// <summary>
    /// The last known value of the record movie dialog's "start type" field
    /// </summary>
    int32_t last_movie_type = 1; // (MOVIE_START_FROM_SNAPSHOT)

    /// <summary>
    /// The last known value of the record movie dialog's "author" field
    /// </summary>
    std::wstring last_movie_author;

    /// <summary>
    /// The main window's X position
    /// </summary>
    int32_t window_x = ((int)0x80000000);

    /// <summary>
    /// The main window's Y position
    /// </summary>
    int32_t window_y = ((int)0x80000000);

    /// <summary>
    /// The main window's width
    /// </summary>
    int32_t window_width = 640;

    /// <summary>
    /// The main window's height
    /// </summary>
    int32_t window_height = 480;

    /// <summary>
    /// The width of rombrowser columns by index
    /// </summary>
    std::vector<std::int32_t> rombrowser_column_widths = {24, 240, 240, 120};

    /// <summary>
    /// The index of the currently sorted column, or -1 if none is sorted
    /// </summary>
    int32_t rombrowser_sorted_column;

    /// <summary>
    /// Whether the selected column is sorted in an ascending order
    /// </summary>
    int32_t rombrowser_sort_ascending = 1;

    /// <summary>
    /// A map of persistent path dialog IDs and the respective value
    /// </summary>
    std::map<std::wstring, std::wstring> persistent_folder_paths;

    /// <summary>
    /// The last selected settings tab's index.
    /// </summary>
    int32_t settings_tab;

    /// <summary>
    /// Whether VCR displays frame information relative to frame 0, not 1
    /// </summary>
    int32_t vcr_0_index;

    /// <summary>
    /// Increments the current slot when saving savestate to slot
    /// </summary>
    int32_t increment_slot;

    /// <summary>
    /// Whether automatic update checking is enabled.
    /// </summary>
    int32_t automatic_update_checking = 1;

    /// <summary>
    /// Whether mupen will avoid showing modals and other elements which require user interaction
    /// </summary>
    int32_t silent_mode = 0;

    /// <summary>
    /// The current seeker input value
    /// </summary>
    std::wstring seeker_value;

    /// <summary>
    /// The multi-frame advance index.
    /// </summary>
    int32_t multi_frame_advance_count = 2;

    /// <summary>
    /// A map of dialog IDs to their default choices for silent mode.
    /// </summary>
    std::map<std::wstring, std::wstring> silent_mode_dialog_choices;

    /// <summary>
    /// A map of trusted Lua script paths. If a Lua script path is present in this map, it will be trusted.
    /// </summary>
    std::map<std::wstring, std::wstring> trusted_lua_paths;

    /// <summary>
    /// A map of fully-qualified action paths to a hotkey assigned to them.
    /// </summary>
    std::map<std::wstring, Hotkey::t_hotkey> hotkeys;

    /// <summary>
    /// A map of fully-qualified action paths to the hotkey which was assigned to them the first time the action was
    /// assigned a hotkey.
    /// </summary>
    std::map<std::wstring, Hotkey::t_hotkey> inital_hotkeys;
};

extern t_config g_config;
extern const t_config g_default_config;

namespace Config
{
/**
 * \brief Initializes the subsystem.
 */
void init();

/**
 * \brief Saves the current config state to the config file.
 */
void save();

/**
 * \brief Applies the current config state and saves it to the config file.
 */
void apply_and_save();

/**
 * \brief Restores the config state from the config file.
 */
void load();

/**
 * \brief Gets the path to the plugin directory based on the current configuration.
 */
std::filesystem::path plugin_directory();

/**
 * \brief Gets the path to the save directory based on the current configuration.
 */
std::filesystem::path save_directory();

/**
 * \brief Gets the path to the screenshot directory based on the current configuration.
 */
std::filesystem::path screenshot_directory();

/**
 * \brief Gets the path to the backup directory based on the current configuration.
 */
std::filesystem::path backup_directory();
} // namespace Config
