/*
 * Copyright (c) 2025, Mupen64 maintainers, contributors, and original authors (Hacktarux, ShadowPrince, linker).
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "stdafx.h"
#include <components/TextEditDialog.h>

struct t_text_edit_dialog_context {
    std::wstring text{};
    std::wstring caption{};
};

static t_text_edit_dialog_context g_ctx{};

static INT_PTR CALLBACK about_dlg_proc(const HWND hwnd, const UINT message, const WPARAM w_param, LPARAM)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetWindowText(hwnd, g_ctx.caption.c_str());
        SetWindowText(GetDlgItem(hwnd, IDC_EDIT), g_ctx.text.c_str());
        SetFocus(GetDlgItem(hwnd, IDC_EDIT));
        break;
    case WM_DESTROY:
        g_ctx.text = get_window_text(GetDlgItem(hwnd, IDC_EDIT)).value_or(L"");
        break;
    case WM_CLOSE:
        EndDialog(hwnd, IDCLOSE);
        break;
    case WM_COMMAND:
        switch (LOWORD(w_param))
        {
        case IDOK:
            EndDialog(hwnd, IDOK);
            break;
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;
        default:
            break;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

std::optional<std::wstring> TextEditDialog::show(const std::wstring& text, const std::wstring& caption)
{
    g_ctx = {};
    g_ctx.text = std::move(text);
    g_ctx.caption = std::move(caption);

    const auto result = DialogBox(g_app_instance, MAKEINTRESOURCE(IDD_TEXT_EDIT), g_main_hwnd, about_dlg_proc);

    if (result == IDCANCEL)
    {
        return std::nullopt;
    }

    return g_ctx.text;
}
