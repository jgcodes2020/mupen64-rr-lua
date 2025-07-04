/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stdafx.h"
#include <ThreadPool.h>
#include <BS_thread_pool.hpp>

static BS::thread_pool pool(4);
static std::unordered_set<size_t> pending_keys{};
static std::mutex mtx{};

void ThreadPool::submit_task(const std::function<void()>& func, const size_t key)
{
    if (!key)
    {
        (void)pool.submit_task(func);
        return;
    }

    {
        std::lock_guard lock(mtx);
        if (!pending_keys.insert(key).second)
        {
            g_view_logger->trace("[AsyncExecutor] Function with key {} already queued, skipping.", key);
            return;
        }
    }

    (void)pool.submit_task([=] {
        func();
        {
            std::lock_guard lock(mtx);
            pending_keys.erase(key);
        }
    });
}
