/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

typedef std::chrono::high_resolution_clock::time_point time_point;

void timer_new_frame();
void timer_new_vi();
void timer_on_speed_modifier_changed();
void timer_get_timings(float &fps, float &vis);
