/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "Presenter.h"

class DCompPresenter : public Presenter
{
  public:
    ~DCompPresenter() override;
    bool init(HWND hwnd) override;
    ID2D1RenderTarget *dc() const override;
    void begin_present() override;
    void end_present() override;
    void blit(HDC hdc, RECT rect) override;
    D2D1_SIZE_U size() override;

  private:
    D2D1_SIZE_U m_size{};
    CompositionContext m_cmp{};
};
