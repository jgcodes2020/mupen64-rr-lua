/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stdafx.h"
#include "Dispatcher.h"

// #define DISPATCHER_OVERHEAD_LOGGING

void Dispatcher::invoke(const std::function<void()> &func)
{
#ifdef DISPATCHER_OVERHEAD_LOGGING
    m_call_start = std::chrono::high_resolution_clock::now();
#endif

    // If the thread id matches the dispatcher's thread id, we don't need to do anything special and can just execute on
    // this thread.
    if (GetCurrentThreadId() == m_thread_id)
    {
        func();
        return;
    }

    std::lock_guard lock(m_mutex);
    m_queue.push(func);
    m_execute_callback();
}

void Dispatcher::execute()
{
    if (m_queue.empty())
    {
        return;
    }

#ifdef DISPATCHER_OVERHEAD_LOGGING
    const auto execute_start = std::chrono::high_resolution_clock::now();
#endif

    while (!m_queue.empty())
    {
        m_queue.front()();
        m_queue.pop();
    }

#ifdef DISPATCHER_OVERHEAD_LOGGING
    const auto end = std::chrono::high_resolution_clock::now();

    const auto call_ns = (end - m_call_start).count();
    const auto execute_ns = (end - execute_start).count();
    const auto overhead_ns = call_ns - execute_ns;
    const auto overhead_percent = (double)overhead_ns / (double)call_ns * 100.0;
    m_overhead_index = (m_overhead_index + 1) % std::size(m_overhead_percentages);
    m_overhead_times[m_overhead_index] = overhead_ns;
    m_overhead_percentages[m_overhead_index] = overhead_percent;

    double percentage_sum = 0.0;
    for (const auto p : m_overhead_percentages)
    {
        percentage_sum += p;
    }
    double avg_overhead_percentage = percentage_sum / (double)std::size(m_overhead_percentages);

    double overhead_sum = 0.0;
    for (const auto t : m_overhead_times)
    {
        overhead_sum += t;
    }
    double avg_overhead_time = overhead_sum / (double)std::size(m_overhead_times);

    g_view_logger->trace("[Dispatcher] id {} overhead avg {:.0f}ns ({:.2f}%)", m_thread_id, avg_overhead_time,
                         avg_overhead_percentage);
#endif
}
