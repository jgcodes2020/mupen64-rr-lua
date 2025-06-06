/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stdafx.h"
#include <Core.h>
#include <IIOHelperService.h>
#include <libdeflate.h>
#include <include/core_api.h>
#include <memory/flashram.h>
#include <memory/memory.h>
#include <memory/savestates.h>
#include <memory/summercart.h>
#include <r4300/interrupt.h>
#include <r4300/r4300.h>
#include <r4300/rom.h>
#include <r4300/vcr.h>

// st that comes from no delay fix mupen, it has some differences compared to new st:
// - one frame of input is "embedded", that is the pif ram holds already fetched controller info.
// - execution continues at exception handler (after input poll) at 0x80000180.
bool g_st_old;
bool g_st_skip_dma{};

/// Represents a task to be performed by the savestate system.
struct t_savestate_task {
    /// The job to perform.
    core_st_job job;

    /// The savestate's source or target medium.
    core_st_medium medium;

    /// Callback to invoke when the task finishes. Mustn't be null.
    core_st_callback callback;

    /// The task's parameters. Only one field in the struct is valid at a time.
    core_st_job_params params{};

    /// Whether warnings, such as those about ROM compatibility, shouldn't be shown.
    bool ignore_warnings;
};

// The task vector mutex. Locked when accessing the task vector.
std::recursive_mutex g_task_mutex;

// The task vector, which contains the task queue to be performed by the savestate system.
std::vector<t_savestate_task> g_tasks;

// Demarcator for new screenshot section
char screen_section[] = "SCR";

// Buffer used for storing flashram data during loading
char g_flashram_buf[1024]{};

// Buffer used for storing event queue data during loading
char g_event_queue_buf[1024]{};

// Buffer used for storing st data up to event queue
uint8_t g_first_block[0xA02BB4 - 32]{};

// The undo savestate buffer.
std::vector<uint8_t> g_undo_savestate;

void get_paths_for_task(const t_savestate_task& task, std::filesystem::path& st_path, std::filesystem::path& sd_path)
{
    sd_path = g_core->get_saves_directory() / (const char*)ROM_HEADER.nom;
    sd_path.replace_extension(".vhd");
}


void load_memory_from_buffer(uint8_t* p)
{
    g_core->io_service->memread(&p, &rdram_register, sizeof(core_rdram_reg));
    g_core->io_service->memread(&p, &MI_register, sizeof(core_mips_reg));
    g_core->io_service->memread(&p, &pi_register, sizeof(core_pi_reg));
    g_core->io_service->memread(&p, &sp_register, sizeof(core_sp_reg));
    g_core->io_service->memread(&p, &rsp_register, sizeof(core_rsp_reg));
    g_core->io_service->memread(&p, &si_register, sizeof(core_si_reg));
    g_core->io_service->memread(&p, &vi_register, sizeof(core_vi_reg));
    g_core->io_service->memread(&p, &ri_register, sizeof(core_ri_reg));
    g_core->io_service->memread(&p, &ai_register, sizeof(core_ai_reg));
    g_core->io_service->memread(&p, &dpc_register, sizeof(core_dpc_reg));
    g_core->io_service->memread(&p, &dps_register, sizeof(core_dps_reg));
    g_core->io_service->memread(&p, rdram, 0x800000);
    g_core->io_service->memread(&p, SP_DMEM, 0x1000);
    g_core->io_service->memread(&p, SP_IMEM, 0x1000);
    g_core->io_service->memread(&p, PIF_RAM, 0x40);

    char buf[4 * 32];
    g_core->io_service->memread(&p, buf, 24);
    load_flashram_infos(buf);

    g_core->io_service->memread(&p, tlb_LUT_r, 0x100000);
    g_core->io_service->memread(&p, tlb_LUT_w, 0x100000);

    g_core->io_service->memread(&p, &llbit, 4);
    g_core->io_service->memread(&p, reg, 32 * 8);
    for (int32_t i = 0; i < 32; i++)
    {
        g_core->io_service->memread(&p, reg_cop0 + i, 4);
        g_core->io_service->memread(&p, buf, 4); // for compatibility with old versions purpose
    }
    g_core->io_service->memread(&p, &lo, 8);
    g_core->io_service->memread(&p, &hi, 8);
    g_core->io_service->memread(&p, reg_cop1_fgr_64, 32 * 8);
    g_core->io_service->memread(&p, &FCR0, 4);
    g_core->io_service->memread(&p, &FCR31, 4);
    g_core->io_service->memread(&p, tlb_e, 32 * sizeof(tlb));
    if (!dynacore && interpcore)
        g_core->io_service->memread(&p, &interp_addr, 4);
    else
    {
        uint32_t target_addr;
        g_core->io_service->memread(&p, &target_addr, 4);
        for (char& i : invalid_code)
            i = 1;
        jump_to(target_addr)
    }

    g_core->io_service->memread(&p, &next_interrupt, 4);
    g_core->io_service->memread(&p, &next_vi, 4);
    g_core->io_service->memread(&p, &vi_field, 4);
}

std::vector<uint8_t> generate_savestate()
{
    std::vector<uint8_t> b;

    b.reserve(0xB624F0);

    memset(g_flashram_buf, 0, sizeof(g_flashram_buf));
    memset(g_event_queue_buf, 0, sizeof(g_event_queue_buf));

    vcr_freeze_info freeze{};
    uint32_t movie_active = vcr_freeze(freeze);

    // NOTE: This saving needs to be done **after** the fixing block, as it is now. See previous regression in f9d58f639c798cbc26bbb808b1c3dbd834ffe2d9.
    save_flashram_infos(g_flashram_buf);
    const int32_t event_queue_len = save_eventqueue_infos(g_event_queue_buf);

    g_core->io_service->vecwrite(b, rom_md5, 32);
    g_core->io_service->vecwrite(b, &rdram_register, sizeof(core_rdram_reg));
    g_core->io_service->vecwrite(b, &MI_register, sizeof(core_mips_reg));
    g_core->io_service->vecwrite(b, &pi_register, sizeof(core_pi_reg));
    g_core->io_service->vecwrite(b, &sp_register, sizeof(core_sp_reg));
    g_core->io_service->vecwrite(b, &rsp_register, sizeof(core_rsp_reg));
    g_core->io_service->vecwrite(b, &si_register, sizeof(core_si_reg));
    g_core->io_service->vecwrite(b, &vi_register, sizeof(core_vi_reg));
    g_core->io_service->vecwrite(b, &ri_register, sizeof(core_ri_reg));
    g_core->io_service->vecwrite(b, &ai_register, sizeof(core_ai_reg));
    g_core->io_service->vecwrite(b, &dpc_register, sizeof(core_dpc_reg));
    g_core->io_service->vecwrite(b, &dps_register, sizeof(core_dps_reg));
    g_core->io_service->vecwrite(b, rdram, 0x800000);
    g_core->io_service->vecwrite(b, SP_DMEM, 0x1000);
    g_core->io_service->vecwrite(b, SP_IMEM, 0x1000);
    g_core->io_service->vecwrite(b, PIF_RAM, 0x40);
    g_core->io_service->vecwrite(b, g_flashram_buf, 24);
    g_core->io_service->vecwrite(b, tlb_LUT_r, 0x100000);
    g_core->io_service->vecwrite(b, tlb_LUT_w, 0x100000);
    g_core->io_service->vecwrite(b, &llbit, 4);
    g_core->io_service->vecwrite(b, reg, 32 * 8);
    for (size_t i = 0; i < 32; i++)
        g_core->io_service->vecwrite(b, reg_cop0 + i, 8); // *8 for compatibility with old versions purpose
    g_core->io_service->vecwrite(b, &lo, 8);
    g_core->io_service->vecwrite(b, &hi, 8);
    g_core->io_service->vecwrite(b, reg_cop1_fgr_64, 32 * 8);
    g_core->io_service->vecwrite(b, &FCR0, 4);
    g_core->io_service->vecwrite(b, &FCR31, 4);
    g_core->io_service->vecwrite(b, tlb_e, 32 * sizeof(tlb));
    if (!dynacore && interpcore)
        g_core->io_service->vecwrite(b, &interp_addr, 4);
    else
        g_core->io_service->vecwrite(b, &PC->addr, 4);
    g_core->io_service->vecwrite(b, &next_interrupt, 4);
    g_core->io_service->vecwrite(b, &next_vi, 4);
    g_core->io_service->vecwrite(b, &vi_field, 4);
    g_core->io_service->vecwrite(b, g_event_queue_buf, event_queue_len);
    g_core->io_service->vecwrite(b, &movie_active, sizeof(movie_active));
    if (movie_active)
    {
        g_core->io_service->vecwrite(b, &freeze.size, sizeof(freeze.size));
        g_core->io_service->vecwrite(b, &freeze.uid, sizeof(freeze.uid));
        g_core->io_service->vecwrite(b, &freeze.current_sample, sizeof(freeze.current_sample));
        g_core->io_service->vecwrite(b, &freeze.current_vi, sizeof(freeze.current_vi));
        g_core->io_service->vecwrite(b, &freeze.length_samples, sizeof(freeze.length_samples));
        g_core->io_service->vecwrite(b, freeze.input_buffer.data(), freeze.input_buffer.size() * sizeof(core_buttons));
    }

    if (vr_get_mge_available() && g_core->cfg->st_screenshot)
    {
        int32_t width;
        int32_t height;
        g_core->plugin_funcs.video_get_video_size(&width, &height);
        g_core->log_trace(std::format(L"Writing screen buffer to savestate, width: {}, height: {}", width, height));

        void* video = malloc(width * height * 3);
        g_core->copy_video(video);

        g_core->io_service->vecwrite(b, screen_section, sizeof(screen_section));
        g_core->io_service->vecwrite(b, &width, sizeof(width));
        g_core->io_service->vecwrite(b, &height, sizeof(height));
        g_core->io_service->vecwrite(b, video, width * height * 3);

        free(video);
    }

    return b;
}

void savestates_save_immediate_impl(const t_savestate_task& task)
{
    // TODO: Reimplement timing

    const auto st = generate_savestate();

    if (task.medium == core_st_medium_path)
    {
        // Always save summercart for some reason
        std::filesystem::path new_st_path = task.params.path;
        std::filesystem::path new_sd_path = "";
        get_paths_for_task(task, new_st_path, new_sd_path);
        if (g_core->cfg->use_summercart)
            save_summercart(new_sd_path);

        // Generate compressed buffer
        std::vector<uint8_t> compressed_buffer;
        compressed_buffer.resize(st.size());

        const auto compressor = libdeflate_alloc_compressor(6);
        const size_t final_size = libdeflate_gzip_compress(compressor, st.data(), st.size(), compressed_buffer.data(), compressed_buffer.size());
        libdeflate_free_compressor(compressor);
        compressed_buffer.resize(final_size);

        // write compressed st to disk
        if (!g_core->io_service->write_file_buffer(new_st_path, compressed_buffer))
        {
            task.callback(core_st_callback_info{
                          .result = ST_FileWriteError,
                          .job = task.job,
                          .medium = task.medium,
                          .params = task.params},
                          st);
            return;
        }
    }

    task.callback(core_st_callback_info{
                  .result = Res_Ok,
                  .job = task.job,
                  .medium = task.medium,
                  .params = task.params},
                  st);
    g_core->callbacks.save_state();
}

void savestates_load_immediate_impl(const t_savestate_task& task)
{
    // TODO: Reimplement timing

    memset(g_event_queue_buf, 0, sizeof(g_event_queue_buf));

    std::filesystem::path new_st_path = task.params.path;
    std::filesystem::path new_sd_path = "";
    get_paths_for_task(task, new_st_path, new_sd_path);

    if (g_core->cfg->use_summercart)
        load_summercart(new_sd_path);

    std::vector<uint8_t> st_buf;

    switch (task.medium)
    {
    case core_st_medium_path:
        st_buf = g_core->io_service->read_file_buffer(new_st_path);
        break;
    case core_st_medium_memory:
        st_buf = task.params.buffer;
        break;
    default:
        assert(false);
    }

    if (st_buf.empty())
    {
        task.callback(core_st_callback_info{
                      .result = ST_NotFound,
                      .job = task.job,
                      .medium = task.medium,
                      .params = task.params},
                      {});
        return;
    }

    std::vector<uint8_t> decompressed_buf = g_core->io_service->auto_decompress(st_buf, 0xB624F0);
    if (decompressed_buf.empty())
    {
        task.callback(core_st_callback_info{
                      .result = ST_DecompressionError,
                      .job = task.job,
                      .medium = task.medium,
                      .params = task.params},
                      {});
        return;
    }

    // BUG (PRONE): we arent allowed to hold on to a vector element pointer
    // find another way of doing this
    auto ptr = decompressed_buf.data();

    // compare current rom hash with one stored in state
    char md5[33] = {0};
    g_core->io_service->memread(&ptr, &md5, 32);

    if (!task.ignore_warnings && memcmp(md5, rom_md5, 32))
    {
        auto result = g_core->show_ask_dialog(CORE_DLG_ST_HASH_MISMATCH, std::format(L"The savestate was created on a rom with hash {}, but is being loaded on another rom.\r\nThe emulator may crash. Are you sure you want to continue?", g_core->io_service->string_to_wstring(md5)).c_str(), L"Savestate", true);

        if (!result)
        {
            task.callback(core_st_callback_info{
                          .result = Res_Cancelled,
                          .job = task.job,
                          .medium = task.medium,
                          .params = task.params},
                          {});
            return;
        }
    }

    // new version does one bigass gzread for first part of .st (static size)
    g_core->io_service->memread(&ptr, g_first_block, sizeof(g_first_block));

    const auto si_reg = (core_si_reg*)&g_first_block[0xDC - 0x20];
    if (!check_register_validity(si_reg) || !check_flashram_infos(&g_first_block[0x8021F0 - 0x20]))
    {
        task.callback(core_st_callback_info{
                      .result = ST_InvalidRegisters,
                      .job = task.job,
                      .medium = task.medium,
                      .params = task.params},
                      {});
        return;
    }

    // now read interrupt queue into buf
    int32_t len;
    for (len = 0; len < sizeof(g_event_queue_buf); len += 8)
    {
        g_core->io_service->memread(&ptr, g_event_queue_buf + len, 4);
        if (*reinterpret_cast<uint32_t*>(&g_event_queue_buf[len]) == 0xFFFFFFFF)
            break;
        g_core->io_service->memread(&ptr, g_event_queue_buf + len + 4, 4);
    }
    if (len == sizeof(g_event_queue_buf))
    {
        // Exhausted the buffer and still no terminator. Prevents the buffer overflow "Queuecrush".
        task.callback(core_st_callback_info{
                      .result = ST_EventQueueTooLong,
                      .job = task.job,
                      .medium = task.medium,
                      .params = task.params},
                      {});
        return;
    }

    uint32_t is_movie;
    g_core->io_service->memread(&ptr, &is_movie, sizeof(is_movie));

    if (is_movie)
    {
        // this .st is part of a movie, we need to overwrite our current movie buffer
        // hash matches, load and verify rest of the data
        vcr_freeze_info freeze{};

        g_core->io_service->memread(&ptr, &freeze.size, sizeof(freeze.size));
        g_core->io_service->memread(&ptr, &freeze.uid, sizeof(freeze.uid));
        g_core->io_service->memread(&ptr, &freeze.current_sample, sizeof(freeze.current_sample));
        g_core->io_service->memread(&ptr, &freeze.current_vi, sizeof(freeze.current_vi));
        g_core->io_service->memread(&ptr, &freeze.length_samples, sizeof(freeze.length_samples));

        freeze.input_buffer.resize(sizeof(core_buttons) * (freeze.length_samples + 1));
        g_core->io_service->memread(&ptr, freeze.input_buffer.data(), freeze.input_buffer.size());

        const auto code = vcr_unfreeze(freeze);

        if (!task.ignore_warnings && code != Res_Ok && vcr_get_task() != task_idle)
        {
            std::wstring err_str = L"Failed to restore movie, ";
            switch (code)
            {
            case VCR_NotFromThisMovie:
                err_str += L"the savestate is not from this movie.";
                break;
            case VCR_InvalidFrame:
                err_str += L"the savestate frame is outside the bounds of the movie.";
                break;
            case VCR_InvalidFormat:
                err_str += L"the savestate freeze buffer format is invalid.";
                break;
            default:
                err_str += L"an unknown error has occured.";
                break;
            }
            err_str += L" Loading the savestate might desynchronize the movie.\r\nAre you sure you want to continue?";

            const auto result = g_core->show_ask_dialog(CORE_DLG_ST_UNFREEZE_WARNING, err_str.c_str(), L"Savestate", true);
            if (!result)
            {
                task.callback(core_st_callback_info{
                              .result = Res_Cancelled,
                              .job = task.job,
                              .medium = task.medium,
                              .params = task.params},
                              {});
                goto failedLoad;
            }
        }
    }
    else
    {
        if (!task.ignore_warnings && (vcr_get_task() == task_recording || vcr_get_task() == task_playback))
        {
            const auto result = g_core->show_ask_dialog(CORE_DLG_ST_NOT_FROM_MOVIE,
                                                        L"The savestate is not from a movie. Loading it might desynchronize the movie.\r\nAre you sure you want to continue?",
                                                        L"Savestate",
                                                        true);
            if (!result)
            {
                task.callback(core_st_callback_info{
                              .result = Res_Cancelled,
                              .job = task.job,
                              .medium = task.medium,
                              .params = task.params},
                              {});
                return;
            }
        }

        // at this point we know the savestate is safe to be loaded (done after else block)
    }

    {
        g_core->log_trace(std::format(L"[Savestates] {} bytes remaining", decompressed_buf.size() - (ptr - decompressed_buf.data())));
        int32_t video_width = 0;
        int32_t video_height = 0;
        void* video_buffer = nullptr;
        if (decompressed_buf.size() - (ptr - decompressed_buf.data()) > 0)
        {
            char scr_section[sizeof(screen_section)] = {0};
            g_core->io_service->memread(&ptr, scr_section, sizeof(screen_section));

            if (!memcmp(scr_section, screen_section, sizeof(screen_section)))
            {
                g_core->log_trace(std::format(L"[Savestates] Restoring screen buffer..."));
                g_core->io_service->memread(&ptr, &video_width, sizeof(video_width));
                g_core->io_service->memread(&ptr, &video_height, sizeof(video_height));

                video_buffer = malloc(video_width * video_height * 3);
                g_core->io_service->memread(&ptr, video_buffer, video_width * video_height * 3);
            }
        }

        // so far loading success! overwrite memory
        load_eventqueue_infos(g_event_queue_buf);
        load_memory_from_buffer(g_first_block);

        // NOTE: We don't want to restore screen buffer while seeking, since it creates a int16_t ugly flicker when the movie restarts by loading state
        if (vr_get_mge_available() && video_buffer && !vcr_is_seeking())
        {
            int32_t current_width, current_height;
            g_core->plugin_funcs.video_get_video_size(&current_width, &current_height);
            if (current_width == video_width && current_height == video_height)
            {
                g_core->load_screen(video_buffer);
                free(video_buffer);
            }
        }
    }

    // NOTE: Some savestates don't have an SI interrupt in the queue, which means that a dma_si_read call which should have happened prior to the save didn't happen.
    // In that case, we "finish up" the dma by performing its final part manually.
    if (get_event(SI_INT) == 0)
    {
        g_core->log_warn(L"[ST] Finishing up DMA...");
        for (size_t i = 0; i < 64 / 4; i++)
            rdram[si_register.si_dram_addr / 4 + i] = sl(PIF_RAM[i]);
        update_count();
        add_interrupt_event(SI_INT, 0x900);
        g_st_skip_dma = true;
    }

    g_core->callbacks.load_state();
    task.callback(core_st_callback_info{
                  .result = Res_Ok,
                  .job = task.job,
                  .medium = task.medium,
                  .params = task.params},
                  decompressed_buf);

failedLoad:
    // legacy .st fix, makes BEQ instruction ignore jump, because .st writes new address explictly.
    // This should cause issues anyway but libultra seems to be flexible (this means there's a chance it fails).
    // For safety, load .sts in dynarec because it completely avoids this issue by being differently coded
    g_st_old = (interp_addr == 0x80000180 || PC->addr == 0x80000180);
    // doubled because can't just reuse this variable
    if (interp_addr == 0x80000180 || (PC->addr == 0x80000180 && !dynacore))
        g_vr_beq_ignore_jmp = true;
    if (!dynacore && interpcore)
    {
        // g_core->log_info(L".st jump: {:#06x}, stopped here:{:#06x}", interp_addr, last_addr);
        last_addr = interp_addr;
    }
    else
    {
        // g_core->log_info(L".st jump: {:#06x}, stopped here:{:#06x}", PC->addr, last_addr);
        last_addr = PC->addr;
    }
}

/**
 * Simplifies the task queue by removing duplicates. Only slot-based tasks are affected for now.
 */
void savestates_simplify_tasks()
{
    std::scoped_lock lock(g_task_mutex);
    g_core->log_info(L"[ST] Simplifying task queue...");

    std::vector<size_t> duplicate_indicies{};


    // De-dup slot-based save tasks
    // 1. Loop through all tasks
    for (size_t i = 0; i < g_tasks.size(); i++)
    {
        const auto& task = g_tasks[i];

        if (task.medium != core_st_medium_path)
            continue;

        // 2. If a path task is detected, loop through all other tasks up to the next load task to find duplicates
        for (size_t j = i + 1; j < g_tasks.size(); j++)
        {
            const auto& other_task = g_tasks[j];

            if (other_task.job == core_st_job_load)
            {
                break;
            }

            if (other_task.medium == core_st_medium_path && task.params.path == other_task.params.path)
            {
                g_core->log_trace(std::format(L"[ST] Found duplicate slot task at index {}", j));
                duplicate_indicies.push_back(j);
            }
        }
    }

    g_tasks = g_core->io_service->erase_indices(g_tasks, duplicate_indicies);
}

/**
 * Warns if a savestate load task is scheduled after a save task.
 */
void savestates_warn_if_load_after_save()
{
    std::scoped_lock lock(g_task_mutex);

    bool encountered_load = false;
    for (const auto& task : g_tasks)
    {
        if (task.job == core_st_job_save && encountered_load)
        {
            g_core->log_warn(std::format(L"[ST] A savestate save task is scheduled after a load task. This may cause unexpected behavior for the caller."));
            break;
        }

        if (task.job == core_st_job_load)
        {
            encountered_load = true;
        }
    }
}

/**
 * Logs the current task queue.
 */
void savestates_log_tasks()
{
    std::scoped_lock lock(g_task_mutex);
    g_core->log_info(L"[ST] Begin task dump");
    savestates_warn_if_load_after_save();
    for (const auto& task : g_tasks)
    {
        std::wstring job_str = (task.job == core_st_job_save) ? L"Save" : L"Load";
        std::wstring medium_str;
        switch (task.medium)
        {
        case core_st_medium_path:
            medium_str = L"Path";
            break;
        case core_st_medium_memory:
            medium_str = L"Memory";
            break;
        default:
            medium_str = L"Unknown";
            break;
        }
        g_core->log_info(std::format(L"[ST] \tTask: Job = {}, Medium = {}", job_str, medium_str));
    }
    g_core->log_info(L"[ST] End task dump");
}

/**
 * Inserts a save operation at the start of the queue (whose callback assigns the undo savestate buffer) if the task queue contains one or more load operations.
 */
void savestates_create_undo_point()
{
    if (!g_core->cfg->st_undo_load)
    {
        return;
    }

    bool queue_contains_load = std::ranges::any_of(g_tasks, [](const t_savestate_task& task) {
        return task.job == core_st_job_load;
    });

    if (!queue_contains_load)
    {
        g_core->log_trace(L"[ST] Skipping undo point creation: no load in queue.");
        return;
    }

    g_core->log_trace(L"[ST] Inserting undo point creation into task queue...");

    const t_savestate_task task = {
    .job = core_st_job_save,
    .medium = core_st_medium_memory,
    .callback = [](const core_st_callback_info& info, const std::vector<uint8_t>& buffer) {
        if (info.result != Res_Ok)
        {
            return;
        }

        std::scoped_lock lock(g_task_mutex);
        g_undo_savestate = buffer;
    },
    .params = {
    .buffer = {},
    },
    .ignore_warnings = true,
    };

    g_tasks.insert(g_tasks.begin(), task);
}

void st_do_work()
{
    std::scoped_lock lock(g_task_mutex);

    if (g_tasks.empty())
    {
        return;
    }

    savestates_simplify_tasks();
    savestates_create_undo_point();
    savestates_simplify_tasks();
    savestates_log_tasks();

    for (const auto& task : g_tasks)
    {
        if (task.job == core_st_job_save)
        {
            savestates_save_immediate_impl(task);
        }
        else
        {
            savestates_load_immediate_impl(task);
        }

        extern void print_queue();
        print_queue();
    }
    g_tasks.clear();
}

void st_on_core_stop()
{
    std::scoped_lock lock(g_task_mutex);
    g_tasks.clear();
    g_undo_savestate.clear();
}

/**
 * Gets whether work can currently be enqueued.
 */
bool can_push_work()
{
    return core_executing;
}

bool st_do_file(const std::filesystem::path& path, const core_st_job job, const core_st_callback& callback, bool ignore_warnings)
{
    std::scoped_lock lock(g_task_mutex);

    if (!can_push_work())
    {
        g_core->log_trace(L"[ST] do_file: Can't enqueue work.");
        if (callback)
        {
            callback(core_st_callback_info{
                     .result = ST_CoreNotLaunched,
                     .job = job,
                     .medium = core_st_medium_path,
                     .params = {.path = path}},
                     {});
        }
        return false;
    }

    auto internal_callback_wrapper = [=](const core_st_callback_info& info, const std::vector<uint8_t>& buffer) {
        g_core->st_pre_callback(info, buffer);
        if (callback)
        {
            callback(info, buffer);
        }
    };

    const t_savestate_task task = {
    .job = job,
    .medium = core_st_medium_path,
    .callback = internal_callback_wrapper,
    .params = {
    .path = path},
    .ignore_warnings = ignore_warnings,
    };

    g_tasks.insert(g_tasks.begin(), task);
    return true;
}

bool st_do_memory(const std::vector<uint8_t>& buffer, const core_st_job job, const core_st_callback& callback, bool ignore_warnings)
{
    std::scoped_lock lock(g_task_mutex);

    if (!can_push_work())
    {
        g_core->log_trace(L"[ST] do_memory: Can't enqueue work.");
        if (callback)
        {
            callback(core_st_callback_info{.result = ST_CoreNotLaunched,
                                           .job = job,
                                           .medium = core_st_medium_memory,
                                           .params = {.buffer = buffer}},
                     {});
        }
        return false;
    }

    auto internal_callback_wrapper = [=](const core_st_callback_info& info, const std::vector<uint8_t>& buffer) {
        g_core->st_pre_callback(info, buffer);
        if (callback)
        {
            callback(info, buffer);
        }
    };

    const t_savestate_task task = {
    .job = job,
    .medium = core_st_medium_memory,
    .callback = internal_callback_wrapper,
    .params = {
    .buffer = buffer},
    .ignore_warnings = ignore_warnings,
    };

    g_tasks.insert(g_tasks.begin(), task);
    return true;
}

void st_get_undo_savestate(std::vector<uint8_t>& buffer)
{
    std::scoped_lock lock(g_task_mutex);
    buffer.clear();
    buffer = g_undo_savestate;
}
