﻿/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#define PLUGIN_VERSION L"1.0.0"

#ifdef _M_X64
#define PLUGIN_ARCH L" x64"
#else
#define PLUGIN_ARCH L" x86"
#endif

#ifdef _DEBUG
#define PLUGIN_TARGET L" Debug"
#else
#define PLUGIN_TARGET L" Release"
#endif

#define PLUGIN_NAME L"No RSP " PLUGIN_VERSION PLUGIN_ARCH PLUGIN_TARGET