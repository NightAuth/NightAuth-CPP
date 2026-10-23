#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define NIGHTAUTH_IMPLEMENTATION
#include "nightauth.hpp"
#include "logo_data.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <thread>
#include <string>
#include <vector>
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cctype>

static NightAuth::Api NightAuthApp(
    "",
    "",
    "1.0",
    ""
);

/* Refined Obsidian & Electric Cobalt Palette */
#define COLOR_CANVAS_BLACK     RGB(9, 10, 15)         /* Deep Obsidian #090A0F */
#define COLOR_SIDEBAR_BG       RGB(13, 15, 22)        /* Elevated Sidebar #0D0F16 */
#define COLOR_HEADER_BG        RGB(11, 13, 19)        /* Titlebar Surface #0B0D13 */
#define COLOR_CARD_BG          RGB(17, 20, 29)        /* Card Surface #11141D */
#define COLOR_CARD_HEADER_BG   RGB(21, 25, 36)        /* Card Header Strip #151924 */
#define COLOR_CARD_BORDER      RGB(36, 42, 58)        /* Subtle Hairline #242A3A */
#define COLOR_CARD_HIGHLIGHT   RGB(52, 62, 86)        /* Top Specular Edge #343E56 */
#define COLOR_INPUT_BG         RGB(11, 13, 20)        /* Recessed Input #0B0D14 */
#define COLOR_INPUT_BORDER     RGB(42, 49, 68)        /* Input Border #2A3144 */
#define COLOR_INPUT_FOCUS      RGB(59, 130, 246)      /* Focused Input Ring #3B82F6 */

#define COLOR_BLUE_PRIMARY     RGB(37, 99, 235)       /* Primary Cobalt #2563EB */
#define COLOR_BLUE_HOVER       RGB(59, 130, 246)      /* Hover Cobalt #3B82F6 */
#define COLOR_BLUE_LIGHT       RGB(147, 197, 253)     /* Soft Ice Blue #93C5FD */
#define COLOR_BLUE_GLOW        RGB(96, 165, 250)      /* Electric Accent #60A5FA */
#define COLOR_BLUE_SUBTLE      RGB(23, 37, 68)        /* Pill / Badge Fill #172544 */

#define COLOR_TEXT_HEAD        RGB(248, 250, 252)     /* Crisp Alabaster #F8FAFC */
#define COLOR_TEXT_BODY        RGB(203, 213, 225)     /* Slate-300 #CBD5E1 */
#define COLOR_TEXT_LABEL       RGB(148, 163, 184)     /* Slate-400 #94A3B8 */
#define COLOR_TEXT_MUTED       RGB(100, 116, 139)     /* Slate-500 #64748B */

#define COLOR_SUCCESS          RGB(52, 211, 153)      /* Emerald-400 #34D399 */
#define COLOR_SUCCESS_BG       RGB(14, 46, 38)        /* Emerald Surface */
#define COLOR_WARNING          RGB(251, 191, 36)      /* Amber-400 #FBBF24 */
#define COLOR_WARNING_BG       RGB(54, 39, 14)        /* Amber Surface */
#define COLOR_ERROR            RGB(248, 113, 113)     /* Rose-400 #F87171 */
#define COLOR_ERROR_BG         RGB(56, 20, 26)        /* Rose Surface */
#define COLOR_DANGER           RGB(220, 38, 38)       /* Red-600 #DC2626 */

#define WIN_WIDTH   1020
#define WIN_HEIGHT  680
#define TITLEBAR_H  46
#define STATUSBAR_H 0
#define SIDEBAR_W   224

enum class ViewMode {
    Login = 0,
    Register,
    License,
    Reset,
    Upgrade,
    Forgot,
    TokenReset,
    Dashboard
};

enum class DashboardTab {
    Overview = 0,
    Subscriptions,
    Variables,
    Chat,
    Stats,
    Security,
    FilesHooks,
    Count
};

#define ID_BTN_MINIMIZE       1001
#define ID_BTN_CLOSE          1002
#define ID_BTN_ACTION         1003
#define ID_BTN_LOGOUT         1004

#define ID_TAB_PRE_BASE       1010
#define ID_DASH_TAB_BASE      1020

#define ID_BTN_HEARTBEAT      1040
#define ID_BTN_REFRESH_SUBS   1041
#define ID_BTN_GET_GLOBAL     1042
#define ID_BTN_GET_USER_VAR   1043
#define ID_BTN_SET_USER_VAR   1044
#define ID_BTN_GET_ALL_VARS   1045
#define ID_BTN_SEND_CHAT      1046
#define ID_BTN_REFRESH_CHAT   1047
#define ID_BTN_REFRESH_STATS  1048
#define ID_BTN_SETUP_2FA      1049
#define ID_BTN_CONFIRM_2FA    1050
#define ID_BTN_DISABLE_2FA    1051
#define ID_BTN_CHECK_BL       1052
#define ID_BTN_VERIFY_HASH    1053
#define ID_BTN_KILLSWITCH     1054
#define ID_BTN_DOWNLOAD_FILE  1055
#define ID_BTN_TRIGGER_HOOK   1056
#define ID_BTN_CHANGE_USERNAME 1057

struct CppAppState {
    std::unique_ptr<NightAuth::Api> api;
    ViewMode current_view{ViewMode::Login};
    DashboardTab active_dash_tab{DashboardTab::Overview};
    bool authenticated{false};
    bool sdk_ready{false};

    HBRUSH hbrCanvas{nullptr};
    HBRUSH hbrSidebar{nullptr};
    HBRUSH hbrHeader{nullptr};
    HBRUSH hbrCard{nullptr};
    HBRUSH hbrCardHeader{nullptr};
    HBRUSH hbrInput{nullptr};

    HFONT hFontBrand{nullptr};
    HFONT hFontHero{nullptr};
    HFONT hFontTitle{nullptr};
    HFONT hFontSection{nullptr};
    HFONT hFontBody{nullptr};
    HFONT hFontInput{nullptr};
    HFONT hFontLabel{nullptr};
    HFONT hFontBadge{nullptr};
    HFONT hFontStatus{nullptr};
    HFONT hFontMono{nullptr};
    HFONT hFontKpi{nullptr};
    HBITMAP hbmLogo{nullptr};

    HWND hWnd{nullptr};

    /* Pre-login controls */
    HWND hEditUser{nullptr};
    HWND hEditPass{nullptr};
    HWND hEditKey{nullptr};
    HWND hEdit2FA{nullptr};
    HWND hEditEmail{nullptr};
    HWND hEditResetToken{nullptr};
    HWND hEditNewPass{nullptr};

    /* Dashboard controls */
    HWND hEditGlobalKey{nullptr};
    HWND hEditVarKey{nullptr};
    HWND hEditVarVal{nullptr};
    HWND hEditChatRoom{nullptr};
    HWND hEditChatMsg{nullptr};
    HWND hEditChangeNewUser{nullptr};
    HWND hEditChangePass{nullptr};
    HWND hEdit2FACode{nullptr};
    HWND hEditDisablePass{nullptr};
    HWND hEditDisableCode{nullptr};
    HWND hEditCheckHwid{nullptr};
    HWND hEditVerifyHash{nullptr};
    HWND hEditBanReason{nullptr};
    HWND hEditFileId{nullptr};
    HWND hEditHookId{nullptr};
    HWND hEditHookPayload{nullptr};

    std::string status_text;
    COLORREF status_color{COLOR_TEXT_MUTED};
    bool is_busy{false};
    int hovered_btn{0};

    /* State caches */
    std::string last_global_val;
    std::string last_user_val;
    std::vector<std::pair<std::string, std::string>> user_vars;
    std::vector<NightAuth::ChatMessage> chat_msgs;
    NightAuth::AppStats app_stats{};
    bool has_stats{false};
    std::vector<std::string> online_users;
    NightAuth::TwoFactorSetupData tfa_setup{};
    bool has_tfa_setup{false};
    std::string download_status;
    bool download_status_ok{false};
    std::string webhook_response;
    bool webhook_response_ok{false};
};

static CppAppState g_app;

static void set_status(const std::string& text, COLORREF color) {
    g_app.status_text = text;
    g_app.status_color = color;
    if (g_app.hWnd) InvalidateRect(g_app.hWnd, NULL, FALSE);
}

static void draw_line(HDC hdc, int x1, int y1, int x2, int y2, COLORREF col) {
    HPEN hPen = CreatePen(PS_SOLID, 1, col);
    HGDIOBJ oldP = SelectObject(hdc, hPen);
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
    SelectObject(hdc, oldP);
    DeleteObject(hPen);
}

static void draw_filled_round_rect(HDC hdc, RECT rc, int radius, COLORREF fillCol, COLORREF borderCol) {
    HBRUSH hbr = CreateSolidBrush(fillCol);
    HPEN hPen = CreatePen(PS_SOLID, 1, borderCol);
    HGDIOBJ hOldBr = SelectObject(hdc, hbr);
    HGDIOBJ hOldPen = SelectObject(hdc, hPen);
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);
    SelectObject(hdc, hOldBr);
    SelectObject(hdc, hOldPen);
    DeleteObject(hbr);
    DeleteObject(hPen);
}

static void draw_card_rect(HDC hdc, RECT rc, int radius, HBRUSH hbrFill, COLORREF borderCol) {
    HPEN hPen = CreatePen(PS_SOLID, 1, borderCol);
    HGDIOBJ hOldPen = SelectObject(hdc, hPen);
    HGDIOBJ hOldBr = SelectObject(hdc, hbrFill);
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);
    SelectObject(hdc, hOldBr);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    if (rc.right - rc.left > 40 && borderCol == COLOR_CARD_BORDER) {
        draw_line(hdc, rc.left + radius / 2 + 2, rc.top + 1, rc.right - radius / 2 - 2, rc.top + 1, COLOR_CARD_HIGHLIGHT);
    }
}

static void draw_section_card(HDC hdc, RECT rc, const char* title, const char* badgeText) {
    draw_card_rect(hdc, rc, 10, g_app.hbrCard, COLOR_CARD_BORDER);

    RECT accentRc = { rc.left + 18, rc.top + 15, rc.left + 21, rc.top + 29 };
    HBRUSH accBr = CreateSolidBrush(COLOR_BLUE_GLOW);
    FillRect(hdc, &accentRc, accBr);
    DeleteObject(accBr);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_TEXT_HEAD);
    SelectObject(hdc, g_app.hFontSection);
    RECT titleRc = { rc.left + 29, rc.top + 12, rc.right - 140, rc.top + 32 };
    DrawTextA(hdc, title, -1, &titleRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    if (badgeText && badgeText[0]) {
        SIZE sz = {0};
        SelectObject(hdc, g_app.hFontBadge);
        GetTextExtentPoint32A(hdc, badgeText, static_cast<int>(std::strlen(badgeText)), &sz);
        int bw = sz.cx + 18;
        RECT bRc = { rc.right - 18 - bw, rc.top + 12, rc.right - 18, rc.top + 32 };
        draw_filled_round_rect(hdc, bRc, 6, COLOR_BLUE_SUBTLE, RGB(37, 62, 112));
        SetTextColor(hdc, COLOR_BLUE_LIGHT);
        DrawTextA(hdc, badgeText, -1, &bRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    draw_line(hdc, rc.left + 1, rc.top + 42, rc.right - 1, rc.top + 42, RGB(28, 33, 46));
}

static void draw_input_box(HDC hdc, RECT rc, HWND hEdit) {
    bool focused = (GetFocus() == hEdit);
    COLORREF border = focused ? COLOR_INPUT_FOCUS : COLOR_INPUT_BORDER;
    draw_card_rect(hdc, rc, 6, g_app.hbrInput, border);
    if (focused) {
        RECT glowRc = { rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1 };
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(30, 64, 138));
        HGDIOBJ oldPen = SelectObject(hdc, hPen);
        HGDIOBJ oldBr = SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, glowRc.left, glowRc.top, glowRc.right, glowRc.bottom, 8, 8);
        SelectObject(hdc, oldBr);
        SelectObject(hdc, oldPen);
        DeleteObject(hPen);
    }
}

static void draw_badge(HDC hdc, int x, int y, const char* text, COLORREF fg, COLORREF bg, COLORREF border) {
    SelectObject(hdc, g_app.hFontBadge);
    SIZE sz = {0};
    GetTextExtentPoint32A(hdc, text, static_cast<int>(std::strlen(text)), &sz);
    RECT rc = { x, y, x + sz.cx + 18, y + 22 };
    draw_filled_round_rect(hdc, rc, 6, bg, border);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, fg);
    DrawTextA(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

static void draw_status_dot(HDC hdc, int cx, int cy, int r, COLORREF col) {
    HBRUSH hbr = CreateSolidBrush(col);
    HPEN hpen = CreatePen(PS_SOLID, 1, col);
    HGDIOBJ oldBr = SelectObject(hdc, hbr);
    HGDIOBJ oldPen = SelectObject(hdc, hpen);
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    SelectObject(hdc, oldBr);
    SelectObject(hdc, oldPen);
    DeleteObject(hbr);
    DeleteObject(hpen);
}

static void draw_tab_pill(HDC hdc, RECT rc, const char* text, bool hovered, bool active) {
    COLORREF bgCol = active ? COLOR_BLUE_PRIMARY : (hovered ? RGB(25, 30, 44) : RGB(13, 15, 22));
    COLORREF borderCol = active ? COLOR_BLUE_GLOW : (hovered ? RGB(55, 68, 98) : RGB(28, 33, 46));
    COLORREF textCol = active ? RGB(255, 255, 255) : (hovered ? COLOR_TEXT_HEAD : COLOR_TEXT_LABEL);

    draw_filled_round_rect(hdc, rc, 6, bgCol, borderCol);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textCol);
    SelectObject(hdc, active ? g_app.hFontBadge : g_app.hFontBody);
    DrawTextA(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

static void draw_sidebar_pill(HDC hdc, RECT rc, const char* code, const char* text, bool hovered, bool active) {
    COLORREF bgCol = active ? RGB(22, 35, 64) : (hovered ? RGB(20, 24, 35) : COLOR_SIDEBAR_BG);
    COLORREF borderCol = active ? RGB(45, 82, 156) : (hovered ? COLOR_CARD_BORDER : COLOR_SIDEBAR_BG);
    COLORREF textCol = active ? COLOR_TEXT_HEAD : (hovered ? COLOR_TEXT_BODY : COLOR_TEXT_LABEL);

    draw_filled_round_rect(hdc, rc, 8, bgCol, borderCol);

    if (active) {
        RECT barRc = { rc.left + 4, rc.top + 8, rc.left + 7, rc.bottom - 8 };
        HBRUSH barBr = CreateSolidBrush(COLOR_BLUE_GLOW);
        FillRect(hdc, &barRc, barBr);
        DeleteObject(barBr);
    }

    RECT idxRc = { rc.left + 13, rc.top + 8, rc.left + 35, rc.bottom - 8 };
    COLORREF idxBg = active ? COLOR_BLUE_PRIMARY : (hovered ? RGB(28, 33, 48) : RGB(18, 21, 30));
    COLORREF idxBorder = active ? COLOR_BLUE_GLOW : RGB(34, 40, 56);
    draw_filled_round_rect(hdc, idxRc, 5, idxBg, idxBorder);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, active ? RGB(255, 255, 255) : COLOR_TEXT_MUTED);
    SelectObject(hdc, g_app.hFontBadge);
    DrawTextA(hdc, code, -1, &idxRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    SetTextColor(hdc, textCol);
    SelectObject(hdc, active ? g_app.hFontSection : g_app.hFontBody);
    RECT textRc = { rc.left + 44, rc.top, rc.right - 10, rc.bottom };
    DrawTextA(hdc, text, -1, &textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

static void draw_primary_button(HDC hdc, RECT rc, const char* text, bool hovered) {
    COLORREF bgCol = hovered ? COLOR_BLUE_HOVER : COLOR_BLUE_PRIMARY;
    COLORREF borderCol = hovered ? COLOR_BLUE_LIGHT : COLOR_BLUE_GLOW;

    draw_filled_round_rect(hdc, rc, 7, bgCol, borderCol);
    draw_line(hdc, rc.left + 4, rc.top + 1, rc.right - 4, rc.top + 1, hovered ? RGB(147, 197, 253) : RGB(96, 165, 250));

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    SelectObject(hdc, g_app.hFontSection);
    DrawTextA(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

static void draw_secondary_button(HDC hdc, RECT rc, const char* text, bool hovered) {
    COLORREF bgCol = hovered ? RGB(28, 34, 49) : RGB(21, 25, 36);
    COLORREF borderCol = hovered ? COLOR_BLUE_GLOW : RGB(45, 53, 74);

    draw_filled_round_rect(hdc, rc, 7, bgCol, borderCol);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, hovered ? COLOR_TEXT_HEAD : COLOR_TEXT_BODY);
    SelectObject(hdc, g_app.hFontSection);
    DrawTextA(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

static void draw_danger_button(HDC hdc, RECT rc, const char* text, bool hovered) {
    COLORREF bgCol = hovered ? RGB(239, 68, 68) : COLOR_DANGER;
    COLORREF borderCol = hovered ? RGB(254, 202, 202) : RGB(248, 113, 113);

    draw_filled_round_rect(hdc, rc, 7, bgCol, borderCol);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));
    SelectObject(hdc, g_app.hFontSection);
    DrawTextA(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

static void draw_info_tile(HDC hdc, RECT rc, const char* label, const char* value, bool monoValue, COLORREF valColor) {
    draw_filled_round_rect(hdc, rc, 7, COLOR_INPUT_BG, RGB(30, 36, 50));
    SetBkMode(hdc, TRANSPARENT);

    SetTextColor(hdc, COLOR_TEXT_MUTED);
    SelectObject(hdc, g_app.hFontLabel);
    RECT lRc = { rc.left + 14, rc.top + 8, rc.right - 12, rc.top + 22 };
    DrawTextA(hdc, label, -1, &lRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

    SetTextColor(hdc, valColor);
    SelectObject(hdc, monoValue ? g_app.hFontMono : g_app.hFontSection);
    RECT vRc = { rc.left + 14, rc.top + 24, rc.right - 12, rc.bottom - 6 };
    DrawTextA(hdc, (value && value[0]) ? value : "N/A", -1, &vRc, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
}

static void update_controls_visibility() {
    HWND all_edits[] = {
        g_app.hEditUser, g_app.hEditPass, g_app.hEditKey, g_app.hEdit2FA,
        g_app.hEditEmail, g_app.hEditResetToken, g_app.hEditNewPass,
        g_app.hEditGlobalKey, g_app.hEditVarKey, g_app.hEditVarVal,
        g_app.hEditChatRoom, g_app.hEditChatMsg,
        g_app.hEditChangeNewUser, g_app.hEditChangePass,
        g_app.hEdit2FACode,
        g_app.hEditDisablePass, g_app.hEditDisableCode, g_app.hEditCheckHwid,
        g_app.hEditVerifyHash, g_app.hEditBanReason, g_app.hEditFileId,
        g_app.hEditHookId, g_app.hEditHookPayload
    };
    for (HWND h : all_edits) {
        if (h) ShowWindow(h, SW_HIDE);
    }

    if (g_app.current_view != ViewMode::Dashboard) {
        int cardW = 860;
        int cardX = (WIN_WIDTH - cardW) / 2;
        int formX = cardX + 340;
        int formW = cardW - 340;
        int fieldX = formX + 36;
        int fieldW = formW - 72;

        if (g_app.current_view == ViewMode::Login) {
            SetWindowPos(g_app.hEditUser, NULL, fieldX + 10, 249, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditPass, NULL, fieldX + 10, 319, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEdit2FA,  NULL, fieldX + 10, 389, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
        } else if (g_app.current_view == ViewMode::Register) {
            SetWindowPos(g_app.hEditUser,  NULL, fieldX + 10, 232, fieldW - 20, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditPass,  NULL, fieldX + 10, 290, fieldW - 20, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditKey,   NULL, fieldX + 10, 348, fieldW - 20, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditEmail, NULL, fieldX + 10, 406, fieldW - 20, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
        } else if (g_app.current_view == ViewMode::License) {
            SetWindowPos(g_app.hEditKey, NULL, fieldX + 10, 269, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
        } else if (g_app.current_view == ViewMode::Reset) {
            SetWindowPos(g_app.hEditUser, NULL, fieldX + 10, 249, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditPass, NULL, fieldX + 10, 319, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEdit2FA,  NULL, fieldX + 10, 389, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
        } else if (g_app.current_view == ViewMode::Upgrade) {
            SetWindowPos(g_app.hEditUser, NULL, fieldX + 10, 259, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditKey,  NULL, fieldX + 10, 329, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
        } else if (g_app.current_view == ViewMode::Forgot) {
            SetWindowPos(g_app.hEditEmail, NULL, fieldX + 10, 269, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
        } else if (g_app.current_view == ViewMode::TokenReset) {
            SetWindowPos(g_app.hEditResetToken, NULL, fieldX + 10, 259, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditNewPass,    NULL, fieldX + 10, 329, fieldW - 20, 24, SWP_NOZORDER | SWP_SHOWWINDOW);
        }
    } else {
        int cx = SIDEBAR_W + 24;
        int cw = WIN_WIDTH - cx - 24;

        if (g_app.active_dash_tab == DashboardTab::Variables) {
            SetWindowPos(g_app.hEditGlobalKey, NULL, cx + 24, 179, 252, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditVarKey,    NULL, cx + 24, 314, 212, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditVarVal,    NULL, cx + 254, 314, 212, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
        } else if (g_app.active_dash_tab == DashboardTab::Chat) {
            SetWindowPos(g_app.hEditChatRoom, NULL, cx + 24, 169, 192, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditChatMsg,  NULL, cx + 24, 576, cw - 172, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
        } else if (g_app.active_dash_tab == DashboardTab::Security) {
            SetWindowPos(g_app.hEditChangeNewUser, NULL, cx + 24, 174, 242, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditChangePass,    NULL, cx + 289, 174, 242, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEdit2FACode,       NULL, cx + 24, 343, 172, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditDisablePass, NULL, cx + 379, 343, 132, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditDisableCode, NULL, cx + 529, 343, 96, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditCheckHwid,   NULL, cx + 24, 485, 342, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditVerifyHash,  NULL, cx + 24, 536, 342, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditBanReason,   NULL, cx + 24, 585, 342, 20, SWP_NOZORDER | SWP_SHOWWINDOW);
        } else if (g_app.active_dash_tab == DashboardTab::FilesHooks) {
            SetWindowPos(g_app.hEditFileId,      NULL, cx + 24, 179, 352, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditHookId,      NULL, cx + 24, 369, 352, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
            SetWindowPos(g_app.hEditHookPayload, NULL, cx + 24, 439, 352, 22, SWP_NOZORDER | SWP_SHOWWINDOW);
        }
    }

    InvalidateRect(g_app.hWnd, NULL, FALSE);
}

static void on_paint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdcMem = BeginPaint(hWnd, &ps);

    RECT clientRc;
    GetClientRect(hWnd, &clientRc);
    int width = clientRc.right;
    int height = clientRc.bottom;

    HDC hdc = CreateCompatibleDC(hdcMem);
    HBITMAP hbm = CreateCompatibleBitmap(hdcMem, width, height);
    HGDIOBJ oldBmp = SelectObject(hdc, hbm);

    FillRect(hdc, &clientRc, g_app.hbrCanvas);

    /* ================= TOP TITLE BAR ================= */
    RECT topBar = { 0, 0, width, TITLEBAR_H };
    FillRect(hdc, &topBar, g_app.hbrHeader);
    draw_line(hdc, 0, TITLEBAR_H - 1, width, TITLEBAR_H - 1, COLOR_CARD_BORDER);

    /* Brand Logo Image (from public/logo.webp -> BGRA 32-bit) */
    if (g_app.hbmLogo) {
        HDC hdcLogo = CreateCompatibleDC(hdc);
        HGDIOBJ oldL = SelectObject(hdcLogo, g_app.hbmLogo);
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        AlphaBlend(hdc, 15, 9, LOGO_WIDTH, LOGO_HEIGHT, hdcLogo, 0, 0, LOGO_WIDTH, LOGO_HEIGHT, bf);
        SelectObject(hdcLogo, oldL);
        DeleteDC(hdcLogo);
    }

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_TEXT_HEAD);
    SelectObject(hdc, g_app.hFontBrand);
    RECT brandRc = { 52, 9, 220, 28 };
    DrawTextA(hdc, "NIGHTAUTH", -1, &brandRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

    SetTextColor(hdc, COLOR_TEXT_MUTED);
    SelectObject(hdc, g_app.hFontLabel);
    RECT subBrandRc = { 52, 26, 260, 40 };
    DrawTextA(hdc, "NATIVE C++ SECURITY CLIENT", -1, &subBrandRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

    /* Window Controls (Minimize / Close) */
    RECT minRc = { width - 76, 8, width - 44, 38 };
    RECT closeRc = { width - 40, 8, width - 8, 38 };
    if (g_app.hovered_btn == ID_BTN_MINIMIZE) {
        draw_filled_round_rect(hdc, minRc, 6, RGB(26, 31, 44), COLOR_CARD_BORDER);
    }
    if (g_app.hovered_btn == ID_BTN_CLOSE) {
        draw_filled_round_rect(hdc, closeRc, 6, COLOR_DANGER, RGB(248, 113, 113));
    }
    SetTextColor(hdc, g_app.hovered_btn == ID_BTN_MINIMIZE ? COLOR_TEXT_HEAD : COLOR_TEXT_LABEL);
    SelectObject(hdc, g_app.hFontSection);
    DrawTextA(hdc, "-", -1, &minRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SetTextColor(hdc, g_app.hovered_btn == ID_BTN_CLOSE ? RGB(255, 255, 255) : COLOR_TEXT_LABEL);
    DrawTextA(hdc, "X", -1, &closeRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    /* ================= PRE-LOGIN VIEW ================= */
    if (g_app.current_view != ViewMode::Dashboard) {
        int cardW = 860;
        int cardH = 540;
        int cardX = (width - cardW) / 2;
        int cardY = 68;
        int heroW = 340;
        int formX = cardX + heroW;
        int formW = cardW - heroW;

        RECT mainCard = { cardX, cardY, cardX + cardW, cardY + cardH };
        draw_card_rect(hdc, mainCard, 12, g_app.hbrCard, COLOR_CARD_BORDER);

        /* Left Hero Branding Panel */
        RECT heroPanel = { cardX + 1, cardY + 1, formX, cardY + cardH - 1 };
        HBRUSH heroBr = CreateSolidBrush(RGB(13, 16, 25));
        FillRect(hdc, &heroPanel, heroBr);
        DeleteObject(heroBr);
        draw_line(hdc, formX, cardY + 1, formX, cardY + cardH - 1, COLOR_CARD_BORDER);

        /* Top Cobalt Accent Bar on Hero */
        RECT heroAccent = { cardX + 28, cardY + 32, cardX + 76, cardY + 36 };
        HBRUSH accBr = CreateSolidBrush(COLOR_BLUE_PRIMARY);
        FillRect(hdc, &heroAccent, accBr);
        DeleteObject(accBr);

        draw_badge(hdc, cardX + 28, cardY + 50, "ZERO-TRUST TELEMETRY", COLOR_BLUE_LIGHT, COLOR_BLUE_SUBTLE, RGB(37, 62, 112));

        SetTextColor(hdc, COLOR_TEXT_HEAD);
        SelectObject(hdc, g_app.hFontHero);
        RECT hTitle1 = { cardX + 28, cardY + 86, formX - 24, cardY + 116 };
        DrawTextA(hdc, "Enterprise Cloud", -1, &hTitle1, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
        RECT hTitle2 = { cardX + 28, cardY + 114, formX - 24, cardY + 144 };
        SetTextColor(hdc, COLOR_BLUE_GLOW);
        DrawTextA(hdc, "Licensing & Identity", -1, &hTitle2, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        SetTextColor(hdc, COLOR_TEXT_LABEL);
        SelectObject(hdc, g_app.hFontBody);
        RECT hDesc = { cardX + 28, cardY + 154, formX - 28, cardY + 218 };
        DrawTextA(hdc, "Cryptographically bound sessions, hardware fingerprinting, real-time cloud variables, and tamper-evident delivery.", -1, &hDesc, DT_WORDBREAK | DT_NOPREFIX);

        /* Feature Checkmarks */
        const char* feats[] = {
            "Instant License Key & Account Activation",
            "Automatic Device HWID Binding & Reset",
            "Cloud Variables, Live Chat & File Delivery",
            "Two-Factor (2FA) Account Protection"
        };
        for (int i = 0; i < 4; ++i) {
            int fy = cardY + 232 + i * 34;
            RECT dotBox = { cardX + 28, fy + 2, cardX + 46, fy + 20 };
            draw_filled_round_rect(hdc, dotBox, 5, COLOR_SUCCESS_BG, RGB(24, 92, 70));
            SetTextColor(hdc, COLOR_SUCCESS);
            SelectObject(hdc, g_app.hFontBadge);
            DrawTextA(hdc, "+", -1, &dotBox, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            SetTextColor(hdc, COLOR_TEXT_BODY);
            SelectObject(hdc, g_app.hFontBody);
            RECT fTxt = { cardX + 56, fy, formX - 20, fy + 22 };
            DrawTextA(hdc, feats[i], -1, &fTxt, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }

        /* Hardware Fingerprint Card at bottom of Hero */
        RECT hwidCard = { cardX + 24, cardY + cardH - 124, formX - 24, cardY + cardH - 24 };
        draw_filled_round_rect(hdc, hwidCard, 8, COLOR_INPUT_BG, RGB(30, 36, 52));

        SetTextColor(hdc, COLOR_BLUE_GLOW);
        SelectObject(hdc, g_app.hFontLabel);
        RECT hwLbl = { hwidCard.left + 14, hwidCard.top + 12, hwidCard.right - 14, hwidCard.top + 26 };
        DrawTextA(hdc, "BOUND APPLICATION & DEVICE", -1, &hwLbl, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        SetTextColor(hdc, COLOR_TEXT_HEAD);
        SelectObject(hdc, g_app.hFontSection);
        RECT appLbl = { hwidCard.left + 14, hwidCard.top + 32, hwidCard.right - 14, hwidCard.top + 52 };
        DrawTextA(hdc, !NightAuthApp.name.empty() ? NightAuthApp.name.c_str() : "Unconfigured App", -1, &appLbl, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        std::string hwidPreview = NightAuth::SecurityProtections::getHardwareId();
        std::string hwLine = "HWID: " + (hwidPreview.empty() ? "Detected on Auth" : hwidPreview);
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        SelectObject(hdc, g_app.hFontMono);
        RECT hwVal = { hwidCard.left + 14, hwidCard.top + 58, hwidCard.right - 14, hwidCard.top + 82 };
        DrawTextA(hdc, hwLine.c_str(), -1, &hwVal, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        /* Right Side: Navigation Pills + Active Auth Form */
        int fieldX = formX + 36;
        int fieldW = formW - 72;

        int tabGap = 5;
        int tabW = (fieldW - tabGap * 6) / 7;
        int tabY = cardY + 26;
        int tabH = 30;

        const char* pre_tabs[] = { "Login", "Register", "License", "HWID", "Upgrade", "Forgot", "Token" };
        for (int i = 0; i < 7; ++i) {
            RECT tabRc = { fieldX + i * (tabW + tabGap), tabY, fieldX + i * (tabW + tabGap) + tabW, tabY + tabH };
            draw_tab_pill(hdc, tabRc, pre_tabs[i], (g_app.hovered_btn == (ID_TAB_PRE_BASE + i)), (g_app.current_view == static_cast<ViewMode>(i)));
        }

        const char* view_titles[] = {
            "Sign in to your account",
            "Create a new account",
            "Instant license key access",
            "Reset hardware binding",
            "Upgrade account subscription",
            "Recover account password",
            "Complete password reset"
        };
        const char* view_subs[] = {
            "Enter your credentials to establish an encrypted cloud session.",
            "Activate your license key and bind your credentials.",
            "Authenticate directly using a valid license key without a password.",
            "Unbind your previous machine fingerprint to authorize this device.",
            "Apply a new license key tier to an existing registered user.",
            "Request a time-limited recovery token to your registered email.",
            "Provide the recovery token and choose a new account password."
        };

        int vIdx = static_cast<int>(g_app.current_view);
        SetTextColor(hdc, COLOR_TEXT_HEAD);
        SelectObject(hdc, g_app.hFontTitle);
        RECT vtRc = { fieldX, cardY + 74, fieldX + fieldW, cardY + 102 };
        DrawTextA(hdc, view_titles[vIdx], -1, &vtRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        SetTextColor(hdc, COLOR_TEXT_LABEL);
        SelectObject(hdc, g_app.hFontBody);
        RECT vsRc = { fieldX, cardY + 104, fieldX + fieldW, cardY + 126 };
        DrawTextA(hdc, view_subs[vIdx], -1, &vsRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        draw_line(hdc, fieldX, cardY + 136, fieldX + fieldW, cardY + 136, RGB(28, 33, 46));

        if (g_app.current_view == ViewMode::Login) {
            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l1 = { fieldX, 224, fieldX + fieldW, 240 }; DrawTextA(hdc, "USERNAME", -1, &l1, DT_LEFT | DT_NOPREFIX);
            RECT f1 = { fieldX, 243, fieldX + fieldW, 279 }; draw_input_box(hdc, f1, g_app.hEditUser);

            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l2 = { fieldX, 294, fieldX + fieldW, 310 }; DrawTextA(hdc, "PASSWORD", -1, &l2, DT_LEFT | DT_NOPREFIX);
            RECT f2 = { fieldX, 313, fieldX + fieldW, 349 }; draw_input_box(hdc, f2, g_app.hEditPass);

            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l3 = { fieldX, 364, fieldX + fieldW, 380 }; DrawTextA(hdc, "TWO-FACTOR CODE (OPTIONAL)", -1, &l3, DT_LEFT | DT_NOPREFIX);
            RECT f3 = { fieldX, 383, fieldX + fieldW, 419 }; draw_input_box(hdc, f3, g_app.hEdit2FA);

            RECT btnRc = { fieldX, 456, fieldX + fieldW, 500 };
            draw_primary_button(hdc, btnRc, g_app.is_busy ? "Authenticating Session..." : "Sign In to Dashboard", g_app.hovered_btn == ID_BTN_ACTION);

        } else if (g_app.current_view == ViewMode::Register) {
            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l1 = { fieldX, 209, fieldX + fieldW, 224 }; DrawTextA(hdc, "USERNAME", -1, &l1, DT_LEFT | DT_NOPREFIX);
            RECT f1 = { fieldX, 226, fieldX + fieldW, 260 }; draw_input_box(hdc, f1, g_app.hEditUser);

            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l2 = { fieldX, 267, fieldX + fieldW, 282 }; DrawTextA(hdc, "PASSWORD", -1, &l2, DT_LEFT | DT_NOPREFIX);
            RECT f2 = { fieldX, 284, fieldX + fieldW, 318 }; draw_input_box(hdc, f2, g_app.hEditPass);

            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l3 = { fieldX, 325, fieldX + fieldW, 340 }; DrawTextA(hdc, "ACTIVATION LICENSE KEY", -1, &l3, DT_LEFT | DT_NOPREFIX);
            RECT f3 = { fieldX, 342, fieldX + fieldW, 376 }; draw_input_box(hdc, f3, g_app.hEditKey);

            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l4 = { fieldX, 383, fieldX + fieldW, 398 }; DrawTextA(hdc, "EMAIL ADDRESS (OPTIONAL)", -1, &l4, DT_LEFT | DT_NOPREFIX);
            RECT f4 = { fieldX, 400, fieldX + fieldW, 434 }; draw_input_box(hdc, f4, g_app.hEditEmail);

            RECT btnRc = { fieldX, 456, fieldX + fieldW, 500 };
            draw_primary_button(hdc, btnRc, g_app.is_busy ? "Creating Account..." : "Register & Activate", g_app.hovered_btn == ID_BTN_ACTION);

        } else if (g_app.current_view == ViewMode::License) {
            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l1 = { fieldX, 244, fieldX + fieldW, 260 }; DrawTextA(hdc, "LICENSE KEY", -1, &l1, DT_LEFT | DT_NOPREFIX);
            RECT f1 = { fieldX, 263, fieldX + fieldW, 299 }; draw_input_box(hdc, f1, g_app.hEditKey);

            RECT infoCard = { fieldX, 322, fieldX + fieldW, 398 };
            draw_filled_round_rect(hdc, infoCard, 8, COLOR_BLUE_SUBTLE, RGB(37, 62, 112));
            SetTextColor(hdc, COLOR_BLUE_LIGHT);
            SelectObject(hdc, g_app.hFontBody);
            RECT noteRc = { fieldX + 16, 336, fieldX + fieldW - 16, 386 };
            DrawTextA(hdc, "License-only mode binds your key directly to this machine's hardware ID for instant passwordless authentication.", -1, &noteRc, DT_WORDBREAK | DT_NOPREFIX);

            RECT btnRc = { fieldX, 456, fieldX + fieldW, 500 };
            draw_primary_button(hdc, btnRc, g_app.is_busy ? "Validating License..." : "Authenticate with Key", g_app.hovered_btn == ID_BTN_ACTION);

        } else if (g_app.current_view == ViewMode::Reset) {
            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l1 = { fieldX, 224, fieldX + fieldW, 240 }; DrawTextA(hdc, "ACCOUNT USERNAME", -1, &l1, DT_LEFT | DT_NOPREFIX);
            RECT f1 = { fieldX, 243, fieldX + fieldW, 279 }; draw_input_box(hdc, f1, g_app.hEditUser);

            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l2 = { fieldX, 294, fieldX + fieldW, 310 }; DrawTextA(hdc, "ACCOUNT PASSWORD", -1, &l2, DT_LEFT | DT_NOPREFIX);
            RECT f2 = { fieldX, 313, fieldX + fieldW, 349 }; draw_input_box(hdc, f2, g_app.hEditPass);

            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l3 = { fieldX, 364, fieldX + fieldW, 380 }; DrawTextA(hdc, "2FA CODE (IF ENABLED)", -1, &l3, DT_LEFT | DT_NOPREFIX);
            RECT f3 = { fieldX, 383, fieldX + fieldW, 419 }; draw_input_box(hdc, f3, g_app.hEdit2FA);

            RECT btnRc = { fieldX, 456, fieldX + fieldW, 500 };
            draw_primary_button(hdc, btnRc, g_app.is_busy ? "Resetting Binding..." : "Reset Hardware Fingerprint", g_app.hovered_btn == ID_BTN_ACTION);

        } else if (g_app.current_view == ViewMode::Upgrade) {
            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l1 = { fieldX, 234, fieldX + fieldW, 250 }; DrawTextA(hdc, "EXISTING USERNAME", -1, &l1, DT_LEFT | DT_NOPREFIX);
            RECT f1 = { fieldX, 253, fieldX + fieldW, 289 }; draw_input_box(hdc, f1, g_app.hEditUser);

            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l2 = { fieldX, 304, fieldX + fieldW, 320 }; DrawTextA(hdc, "NEW UPGRADE LICENSE KEY", -1, &l2, DT_LEFT | DT_NOPREFIX);
            RECT f2 = { fieldX, 323, fieldX + fieldW, 359 }; draw_input_box(hdc, f2, g_app.hEditKey);

            RECT btnRc = { fieldX, 456, fieldX + fieldW, 500 };
            draw_primary_button(hdc, btnRc, g_app.is_busy ? "Applying Upgrade..." : "Apply Subscription Upgrade", g_app.hovered_btn == ID_BTN_ACTION);

        } else if (g_app.current_view == ViewMode::Forgot) {
            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l1 = { fieldX, 244, fieldX + fieldW, 260 }; DrawTextA(hdc, "REGISTERED EMAIL ADDRESS", -1, &l1, DT_LEFT | DT_NOPREFIX);
            RECT f1 = { fieldX, 263, fieldX + fieldW, 299 }; draw_input_box(hdc, f1, g_app.hEditEmail);

            RECT infoCard = { fieldX, 322, fieldX + fieldW, 398 };
            draw_filled_round_rect(hdc, infoCard, 8, COLOR_BLUE_SUBTLE, RGB(37, 62, 112));
            SetTextColor(hdc, COLOR_BLUE_LIGHT);
            SelectObject(hdc, g_app.hFontBody);
            RECT noteRc = { fieldX + 16, 336, fieldX + fieldW - 16, 386 };
            DrawTextA(hdc, "We will dispatch a one-time cryptographic reset token to your verified email address. Then switch to the Token tab.", -1, &noteRc, DT_WORDBREAK | DT_NOPREFIX);

            RECT btnRc = { fieldX, 456, fieldX + fieldW, 500 };
            draw_primary_button(hdc, btnRc, g_app.is_busy ? "Dispatching Email..." : "Send Recovery Token", g_app.hovered_btn == ID_BTN_ACTION);

        } else if (g_app.current_view == ViewMode::TokenReset) {
            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l1 = { fieldX, 234, fieldX + fieldW, 250 }; DrawTextA(hdc, "RECOVERY RESET TOKEN", -1, &l1, DT_LEFT | DT_NOPREFIX);
            RECT f1 = { fieldX, 253, fieldX + fieldW, 289 }; draw_input_box(hdc, f1, g_app.hEditResetToken);

            SetTextColor(hdc, COLOR_TEXT_LABEL); SelectObject(hdc, g_app.hFontLabel);
            RECT l2 = { fieldX, 304, fieldX + fieldW, 320 }; DrawTextA(hdc, "NEW ACCOUNT PASSWORD", -1, &l2, DT_LEFT | DT_NOPREFIX);
            RECT f2 = { fieldX, 323, fieldX + fieldW, 359 }; draw_input_box(hdc, f2, g_app.hEditNewPass);

            RECT btnRc = { fieldX, 456, fieldX + fieldW, 500 };
            draw_primary_button(hdc, btnRc, g_app.is_busy ? "Updating Password..." : "Confirm New Password", g_app.hovered_btn == ID_BTN_ACTION);
        }

    } else {
        /* ================= DASHBOARD VIEW ================= */
        int contentBottom = height - STATUSBAR_H;
        RECT sideRc = { 0, TITLEBAR_H, SIDEBAR_W, contentBottom };
        FillRect(hdc, &sideRc, g_app.hbrSidebar);
        draw_line(hdc, SIDEBAR_W, TITLEBAR_H, SIDEBAR_W, contentBottom, COLOR_CARD_BORDER);

        const NightAuth::UserData& ud = g_app.api->user;

        /* Sidebar Section Header */
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        SelectObject(hdc, g_app.hFontLabel);
        RECT navHdr = { 20, TITLEBAR_H + 16, SIDEBAR_W - 16, TITLEBAR_H + 32 };
        DrawTextA(hdc, "WORKSPACE MODULES", -1, &navHdr, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        const char* dash_tabs[] = {
            "Overview", "Subscriptions", "Cloud Variables",
            "Live Chat", "App Statistics", "Security & 2FA", "Files & Webhooks"
        };
        const char* dash_codes[] = { "01", "02", "03", "04", "05", "06", "07" };

        int sPadY = TITLEBAR_H + 38;
        int sTabH = 38;
        int sGap = 5;
        for (int i = 0; i < static_cast<int>(DashboardTab::Count); ++i) {
            RECT pillRc = { 12, sPadY + i * (sTabH + sGap), SIDEBAR_W - 12, sPadY + i * (sTabH + sGap) + sTabH };
            draw_sidebar_pill(hdc, pillRc, dash_codes[i], dash_tabs[i],
                              (g_app.hovered_btn == (ID_DASH_TAB_BASE + i)),
                              (g_app.active_dash_tab == static_cast<DashboardTab>(i)));
        }

        /* User Profile Mini-Card at Bottom of Sidebar */
        RECT profCard = { 12, contentBottom - 118, SIDEBAR_W - 12, contentBottom - 56 };
        draw_filled_round_rect(hdc, profCard, 8, COLOR_CARD_BG, COLOR_CARD_BORDER);

        RECT avatarRc = { profCard.left + 10, profCard.top + 12, profCard.left + 44, profCard.top + 46 };
        draw_filled_round_rect(hdc, avatarRc, 8, COLOR_BLUE_SUBTLE, COLOR_BLUE_PRIMARY);
        char initial[2] = { !ud.username.empty() ? static_cast<char>(std::toupper(static_cast<unsigned char>(ud.username[0]))) : 'U', '\0' };
        SetTextColor(hdc, COLOR_BLUE_LIGHT);
        SelectObject(hdc, g_app.hFontSection);
        DrawTextA(hdc, initial, -1, &avatarRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

        SetTextColor(hdc, COLOR_TEXT_HEAD);
        SelectObject(hdc, g_app.hFontSection);
        RECT pNameRc = { profCard.left + 52, profCard.top + 11, profCard.right - 10, profCard.top + 30 };
        DrawTextA(hdc, !ud.username.empty() ? ud.username.c_str() : "Authenticated", -1, &pNameRc, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        draw_status_dot(hdc, profCard.left + 57, profCard.top + 39, 3, COLOR_SUCCESS);
        SetTextColor(hdc, COLOR_SUCCESS);
        SelectObject(hdc, g_app.hFontBadge);
        RECT pStatRc = { profCard.left + 66, profCard.top + 31, profCard.right - 10, profCard.top + 47 };
        DrawTextA(hdc, "Verified Session", -1, &pStatRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        /* Sign Out Button */
        RECT logoutRc = { 12, contentBottom - 46, SIDEBAR_W - 12, contentBottom - 12 };
        draw_secondary_button(hdc, logoutRc, "Sign Out Session", g_app.hovered_btn == ID_BTN_LOGOUT);

        /* Right Content Area */
        int cx = SIDEBAR_W + 24;
        int cw = width - cx - 24;

        /* Page Header & Subtitle */
        const char* dash_subtitles[] = {
            "Session identity, hardware binding telemetry, and live cloud heartbeat.",
            "Active license entitlements, expiration windows, and tier assignments.",
            "Encrypted server-side global constants and per-user key-value storage.",
            "Real-time channel messaging with authenticated workspace members.",
            "Application-wide user metrics and active concurrent sessions.",
            "TOTP two-factor authentication, HWID blacklist check, and emergency lockout.",
            "Protected asset delivery via scoped access tickets and webhook dispatch."
        };

        int tabIdx = static_cast<int>(g_app.active_dash_tab);
        SetTextColor(hdc, COLOR_TEXT_HEAD);
        SelectObject(hdc, g_app.hFontTitle);
        RECT headRc = { cx, 58, cx + cw - 180, 84 };
        DrawTextA(hdc, dash_tabs[tabIdx], -1, &headRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        SetTextColor(hdc, COLOR_TEXT_LABEL);
        SelectObject(hdc, g_app.hFontBody);
        RECT subHeadRc = { cx, 84, cx + cw - 180, 104 };
        DrawTextA(hdc, dash_subtitles[tabIdx], -1, &subHeadRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

        if (g_app.active_dash_tab == DashboardTab::Overview) {
            RECT userBox = { cx, 114, cx + cw, 376 };
            draw_section_card(hdc, userBox, "USER IDENTITY & HARDWARE BINDING", "ENCRYPTED SESSION");

            int col3W = (cw - 64) / 3;
            RECT t1 = { cx + 20, 170, cx + 20 + col3W, 220 };
            draw_info_tile(hdc, t1, "ACCOUNT USERNAME", !ud.username.empty() ? ud.username.c_str() : "Authenticated", false, COLOR_TEXT_HEAD);

            RECT t2 = { cx + 32 + col3W, 170, cx + 32 + col3W * 2, 220 };
            draw_info_tile(hdc, t2, "EMAIL ADDRESS", !ud.email.empty() ? ud.email.c_str() : "Not Bound", false, COLOR_TEXT_BODY);

            RECT t3 = { cx + 44 + col3W * 2, 170, cx + cw - 20, 220 };
            draw_info_tile(hdc, t3, "ACCOUNT STATUS", !ud.status.empty() ? ud.status.c_str() : "Active", false, COLOR_SUCCESS);

            RECT t4 = { cx + 20, 230, cx + 20 + col3W, 280 };
            draw_info_tile(hdc, t4, "CLIENT IP ADDRESS", !ud.ip.empty() ? ud.ip.c_str() : "Protected / TLS", true, COLOR_BLUE_LIGHT);

            RECT t5 = { cx + 32 + col3W, 230, cx + 32 + col3W * 2, 280 };
            draw_info_tile(hdc, t5, "CREATED DATE", !ud.created_at.empty() ? ud.created_at.c_str() : "-", true, COLOR_TEXT_BODY);

            RECT t6 = { cx + 44 + col3W * 2, 230, cx + cw - 20, 280 };
            draw_info_tile(hdc, t6, "LAST LOGIN", !ud.last_login_at.empty() ? ud.last_login_at.c_str() : "Just Now", true, COLOR_TEXT_BODY);

            std::string hwid = NightAuth::SecurityProtections::getHardwareId();
            RECT t7 = { cx + 20, 290, cx + cw - 210, 340 };
            draw_info_tile(hdc, t7, "LOCAL HARDWARE FINGERPRINT (HWID)", hwid.c_str(), true, COLOR_TEXT_BODY);

            RECT hbBtn = { cx + cw - 196, 294, cx + cw - 20, 336 };
            draw_primary_button(hdc, hbBtn, "Send Heartbeat", g_app.hovered_btn == ID_BTN_HEARTBEAT);

            /* App Session Card */
            RECT appBox = { cx, 392, cx + cw, 618 };
            draw_section_card(hdc, appBox, "APPLICATION TELEMETRY & CLOUD SESSION", "REALTIME");

            int thirdW = (cw - 64) / 3;

            RECT a1 = { cx + 20, 450, cx + 20 + thirdW, 502 };
            draw_info_tile(hdc, a1, "APPLICATION NAME", !NightAuthApp.name.empty() ? NightAuthApp.name.c_str() : "NightAuth App", false, COLOR_TEXT_HEAD);

            char onlineBuf[64];
            std::snprintf(onlineBuf, sizeof(onlineBuf), "%lld Active Sessions", static_cast<long long>(g_app.api->app.online_users));
            RECT a2 = { cx + 32 + thirdW, 450, cx + 32 + thirdW * 2, 502 };
            draw_info_tile(hdc, a2, "CLOUD PRESENCE", onlineBuf, false, COLOR_BLUE_GLOW);

            RECT a3 = { cx + 44 + thirdW * 2, 450, cx + cw - 20, 502 };
            draw_info_tile(hdc, a3, "LAST AUTHENTICATED", !ud.last_login_at.empty() ? ud.last_login_at.c_str() : "Just Now", true, COLOR_TEXT_BODY);

            RECT bannerRc = { cx + 20, 518, cx + cw - 20, 598 };
            draw_filled_round_rect(hdc, bannerRc, 8, COLOR_BLUE_SUBTLE, RGB(37, 62, 112));
            SetTextColor(hdc, COLOR_BLUE_LIGHT);
            SelectObject(hdc, g_app.hFontSection);
            RECT bTitle = { bannerRc.left + 16, bannerRc.top + 12, bannerRc.right - 16, bannerRc.top + 32 };
            DrawTextA(hdc, "Native Win32 GDI Security Runtime", -1, &bTitle, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

            SetTextColor(hdc, COLOR_TEXT_BODY);
            SelectObject(hdc, g_app.hFontBody);
            RECT bDesc = { bannerRc.left + 16, bannerRc.top + 36, bannerRc.right - 16, bannerRc.bottom - 10 };
            DrawTextA(hdc, "All API payloads are verified against server signatures with automatic replay protection and hardware binding.", -1, &bDesc, DT_WORDBREAK | DT_NOPREFIX);

        } else if (g_app.active_dash_tab == DashboardTab::Subscriptions) {
            RECT subBox = { cx, 114, cx + cw, 618 };
            int subCount = static_cast<int>(ud.subscriptions.size());
            char badgeStr[32];
            std::snprintf(badgeStr, sizeof(badgeStr), "%d ACTIVE", subCount);
            draw_section_card(hdc, subBox, "ACTIVE SUBSCRIPTIONS & ENTITLEMENTS", badgeStr);

            if (subCount == 0) {
                RECT emptyCard = { cx + 20, 176, cx + cw - 20, 266 };
                draw_filled_round_rect(hdc, emptyCard, 8, COLOR_INPUT_BG, COLOR_CARD_BORDER);
                SetTextColor(hdc, COLOR_TEXT_HEAD);
                SelectObject(hdc, g_app.hFontSection);
                RECT e1 = { emptyCard.left + 20, emptyCard.top + 22, emptyCard.right - 20, emptyCard.top + 44 };
                DrawTextA(hdc, "No Active Subscription Tiers", -1, &e1, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

                SetTextColor(hdc, COLOR_TEXT_MUTED);
                SelectObject(hdc, g_app.hFontBody);
                RECT e2 = { emptyCard.left + 20, emptyCard.top + 46, emptyCard.right - 20, emptyCard.top + 74 };
                DrawTextA(hdc, "This account does not currently have any recurring or lifetime subscription entitlements attached.", -1, &e2, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            } else {
                for (int i = 0; i < subCount && i < 7; ++i) {
                    int sy = 172 + i * 58;
                    RECT itemRc = { cx + 20, sy, cx + cw - 20, sy + 48 };
                    draw_filled_round_rect(hdc, itemRc, 8, COLOR_INPUT_BG, COLOR_INPUT_BORDER);

                    RECT stripe = { itemRc.left + 4, itemRc.top + 8, itemRc.left + 7, itemRc.bottom - 8 };
                    HBRUSH sBr = CreateSolidBrush(COLOR_SUCCESS);
                    FillRect(hdc, &stripe, sBr);
                    DeleteObject(sBr);

                    const auto& sub = ud.subscriptions[i];
                    SetTextColor(hdc, COLOR_TEXT_HEAD);
                    SelectObject(hdc, g_app.hFontSection);
                    RECT tierRc = { itemRc.left + 20, sy + 8, itemRc.left + 260, sy + 28 };
                    DrawTextA(hdc, sub.subscription.c_str(), -1, &tierRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

                    std::string keyStr = "Key: " + (!sub.key.empty() ? sub.key : "Direct Entitlement");
                    SetTextColor(hdc, COLOR_TEXT_MUTED);
                    SelectObject(hdc, g_app.hFontMono);
                    RECT keyRc = { itemRc.left + 20, sy + 27, itemRc.right - 220, sy + 44 };
                    DrawTextA(hdc, keyStr.c_str(), -1, &keyRc, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

                    std::string expStr = "EXPIRES: " + (!sub.expiry.empty() ? sub.expiry : "LIFETIME");
                    draw_badge(hdc, itemRc.right - 210, sy + 13, expStr.c_str(), COLOR_SUCCESS, COLOR_SUCCESS_BG, RGB(24, 92, 70));
                }
            }

        } else if (g_app.active_dash_tab == DashboardTab::Variables) {
            RECT gBox = { cx, 114, cx + cw, 224 };
            draw_section_card(hdc, gBox, "GLOBAL CLOUD VARIABLE", "READ-ONLY");

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lgk = { cx + 20, 158, cx + 280, 173 };
            DrawTextA(hdc, "VARIABLE KEY", -1, &lgk, DT_LEFT | DT_NOPREFIX);

            RECT f1 = { cx + 20, 174, cx + 280, 208 }; draw_input_box(hdc, f1, g_app.hEditGlobalKey);
            RECT b1 = { cx + 295, 174, cx + 425, 208 }; draw_primary_button(hdc, b1, "Fetch Global", g_app.hovered_btn == ID_BTN_GET_GLOBAL);

            RECT gResPill = { cx + 440, 174, cx + cw - 20, 208 };
            draw_filled_round_rect(hdc, gResPill, 6, COLOR_INPUT_BG, RGB(32, 38, 54));
            SetTextColor(hdc, !g_app.last_global_val.empty() ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
            SelectObject(hdc, g_app.hFontMono);
            RECT rVal = { gResPill.left + 12, gResPill.top, gResPill.right - 12, gResPill.bottom };
            DrawTextA(hdc, !g_app.last_global_val.empty() ? g_app.last_global_val.c_str() : "Result will appear here...", -1, &rVal, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

            RECT uBox = { cx, 238, cx + cw, 384 };
            draw_section_card(hdc, uBox, "USER CLOUD VARIABLE (GET / SET)", "READ & WRITE");

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT luk = { cx + 20, 293, cx + 240, 308 };
            DrawTextA(hdc, "VARIABLE KEY", -1, &luk, DT_LEFT | DT_NOPREFIX);

            RECT luv = { cx + 250, 293, cx + 470, 308 };
            DrawTextA(hdc, "VALUE (FOR SET)", -1, &luv, DT_LEFT | DT_NOPREFIX);

            RECT f2 = { cx + 20, 309, cx + 240, 343 }; draw_input_box(hdc, f2, g_app.hEditVarKey);
            RECT f3 = { cx + 250, 309, cx + 470, 343 }; draw_input_box(hdc, f3, g_app.hEditVarVal);

            RECT b2 = { cx + 485, 309, cx + 590, 343 }; draw_secondary_button(hdc, b2, "Get Value", g_app.hovered_btn == ID_BTN_GET_USER_VAR);
            RECT b3 = { cx + 600, 309, cx + 705, 343 }; draw_primary_button(hdc, b3, "Save Value", g_app.hovered_btn == ID_BTN_SET_USER_VAR);

            if (!g_app.last_user_val.empty()) {
                RECT ruVal = { cx + 20, 350, cx + cw - 20, 374 };
                SetTextColor(hdc, COLOR_SUCCESS);
                SelectObject(hdc, g_app.hFontMono);
                std::string resStr = "Retrieved Value: " + g_app.last_user_val;
                DrawTextA(hdc, resStr.c_str(), -1, &ruVal, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }

            RECT allBox = { cx, 398, cx + cw, 618 };
            draw_section_card(hdc, allBox, "ALL USER VARIABLES", NULL);

            RECT bAll = { cx + cw - 156, 405, cx + cw - 18, 434 };
            draw_secondary_button(hdc, bAll, "Sync All Vars", g_app.hovered_btn == ID_BTN_GET_ALL_VARS);

            if (g_app.user_vars.empty()) {
                SetTextColor(hdc, COLOR_TEXT_MUTED);
                SelectObject(hdc, g_app.hFontBody);
                RECT emptyV = { cx + 20, 462, cx + cw - 20, 490 };
                DrawTextA(hdc, "Click 'Sync All Vars' to inspect all key-value pairs stored for this account.", -1, &emptyV, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            } else {
                for (size_t i = 0; i < g_app.user_vars.size() && i < 4; ++i) {
                    int vy = 452 + static_cast<int>(i) * 38;
                    RECT rowRc = { cx + 20, vy, cx + cw - 20, vy + 32 };
                    draw_filled_round_rect(hdc, rowRc, 6, COLOR_INPUT_BG, RGB(30, 36, 50));

                    std::string vstr = g_app.user_vars[i].first + "  =  " + g_app.user_vars[i].second;
                    RECT vr = { rowRc.left + 14, rowRc.top, rowRc.right - 14, rowRc.bottom };
                    SetTextColor(hdc, COLOR_BLUE_LIGHT);
                    SelectObject(hdc, g_app.hFontMono);
                    DrawTextA(hdc, vstr.c_str(), -1, &vr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
                }
            }

        } else if (g_app.active_dash_tab == DashboardTab::Chat) {
            RECT cBox = { cx, 114, cx + cw, 618 };
            draw_section_card(hdc, cBox, "LIVE CHANNEL STREAM", "REALTIME");

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lr = { cx + 20, 148, cx + 220, 163 };
            DrawTextA(hdc, "CHANNEL / ROOM", -1, &lr, DT_LEFT | DT_NOPREFIX);

            RECT fRoom = { cx + 20, 164, cx + 220, 198 }; draw_input_box(hdc, fRoom, g_app.hEditChatRoom);
            RECT bRefChat = { cx + 235, 164, cx + 365, 198 }; draw_secondary_button(hdc, bRefChat, "Refresh Channel", g_app.hovered_btn == ID_BTN_REFRESH_CHAT);

            RECT msgBox = { cx + 20, 210, cx + cw - 20, 542 };
            draw_filled_round_rect(hdc, msgBox, 8, COLOR_CANVAS_BLACK, COLOR_CARD_BORDER);

            if (g_app.chat_msgs.empty()) {
                RECT emptyChat = { msgBox.left + 20, msgBox.top + 145, msgBox.right - 20, msgBox.top + 185 };
                SetTextColor(hdc, COLOR_TEXT_MUTED);
                SelectObject(hdc, g_app.hFontBody);
                DrawTextA(hdc, "No messages loaded in this channel yet. Click 'Refresh Channel' or send a message below.", -1, &emptyChat, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            } else {
                for (size_t i = 0; i < g_app.chat_msgs.size() && i < 8; ++i) {
                    int my = 222 + static_cast<int>(i) * 38;
                    RECT bubble = { msgBox.left + 12, my, msgBox.right - 12, my + 32 };
                    draw_filled_round_rect(hdc, bubble, 6, COLOR_CARD_BG, RGB(30, 35, 48));

                    SetTextColor(hdc, COLOR_BLUE_GLOW);
                    SelectObject(hdc, g_app.hFontSection);
                    RECT authRc = { bubble.left + 12, bubble.top, bubble.left + 150, bubble.bottom };
                    DrawTextA(hdc, g_app.chat_msgs[i].author.c_str(), -1, &authRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

                    SetTextColor(hdc, COLOR_TEXT_BODY);
                    SelectObject(hdc, g_app.hFontBody);
                    RECT mTxtRc = { bubble.left + 156, bubble.top, bubble.right - 12, bubble.bottom };
                    DrawTextA(hdc, g_app.chat_msgs[i].message.c_str(), -1, &mTxtRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
                }
            }

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lm = { cx + 20, 554, cx + cw - 140, 569 };
            DrawTextA(hdc, "COMPOSE MESSAGE", -1, &lm, DT_LEFT | DT_NOPREFIX);

            RECT fMsg = { cx + 20, 571, cx + cw - 144, 605 }; draw_input_box(hdc, fMsg, g_app.hEditChatMsg);
            RECT bSend = { cx + cw - 132, 571, cx + cw - 20, 605 }; draw_primary_button(hdc, bSend, "Send", g_app.hovered_btn == ID_BTN_SEND_CHAT);

        } else if (g_app.active_dash_tab == DashboardTab::Stats) {
            int cardW = (cw - 36) / 4;
            const char* stat_labels[] = { "TOTAL USERS", "ACTIVE USERS", "BANNED USERS", "TOTAL LICENSES" };
            COLORREF stat_accents[] = { COLOR_BLUE_GLOW, COLOR_SUCCESS, COLOR_ERROR, COLOR_WARNING };
            long long stat_vals[4] = {
                g_app.has_stats ? static_cast<long long>(g_app.app_stats.total_users) : 0,
                g_app.has_stats ? static_cast<long long>(g_app.app_stats.active_users) : 0,
                g_app.has_stats ? static_cast<long long>(g_app.app_stats.banned_users) : 0,
                g_app.has_stats ? static_cast<long long>(g_app.app_stats.total_licenses) : 0
            };

            for (int i = 0; i < 4; ++i) {
                RECT kpi = { cx + i * (cardW + 12), 114, cx + i * (cardW + 12) + cardW, 212 };
                draw_card_rect(hdc, kpi, 10, g_app.hbrCard, COLOR_CARD_BORDER);

                RECT topStrip = { kpi.left + 16, kpi.top + 14, kpi.left + 44, kpi.top + 17 };
                HBRUSH stBr = CreateSolidBrush(stat_accents[i]);
                FillRect(hdc, &topStrip, stBr);
                DeleteObject(stBr);

                SetTextColor(hdc, COLOR_TEXT_LABEL);
                SelectObject(hdc, g_app.hFontLabel);
                RECT kl = { kpi.left + 16, kpi.top + 26, kpi.right - 14, kpi.top + 42 };
                DrawTextA(hdc, stat_labels[i], -1, &kl, DT_LEFT | DT_NOPREFIX);

                std::string vbuf = std::to_string(stat_vals[i]);
                SetTextColor(hdc, COLOR_TEXT_HEAD);
                SelectObject(hdc, g_app.hFontKpi);
                RECT kv = { kpi.left + 16, kpi.top + 46, kpi.right - 14, kpi.bottom - 10 };
                DrawTextA(hdc, vbuf.c_str(), -1, &kv, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }

            RECT onlineBox = { cx, 228, cx + cw, 618 };
            draw_section_card(hdc, onlineBox, "ONLINE ACTIVE SESSIONS", NULL);

            RECT bRefStats = { cx + cw - 156, 235, cx + cw - 18, 264 };
            draw_secondary_button(hdc, bRefStats, "Refresh Metrics", g_app.hovered_btn == ID_BTN_REFRESH_STATS);

            if (g_app.online_users.empty()) {
                SetTextColor(hdc, COLOR_TEXT_MUTED);
                SelectObject(hdc, g_app.hFontBody);
                RECT emptyO = { cx + 20, 294, cx + cw - 20, 322 };
                DrawTextA(hdc, "Click 'Refresh Metrics' to query live application telemetry and active users.", -1, &emptyO, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
            } else {
                for (size_t i = 0; i < g_app.online_users.size() && i < 8; ++i) {
                    int oy = 284 + static_cast<int>(i) * 38;
                    RECT orc = { cx + 20, oy, cx + cw - 20, oy + 32 };
                    draw_filled_round_rect(hdc, orc, 6, COLOR_INPUT_BG, RGB(30, 36, 50));
                    draw_status_dot(hdc, orc.left + 16, oy + 16, 4, COLOR_SUCCESS);

                    SetTextColor(hdc, COLOR_TEXT_HEAD);
                    SelectObject(hdc, g_app.hFontSection);
                    RECT uTxt = { orc.left + 30, oy, orc.right - 150, oy + 32 };
                    DrawTextA(hdc, g_app.online_users[i].c_str(), -1, &uTxt, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

                    draw_badge(hdc, orc.right - 120, oy + 5, "ACTIVE SESSION", COLOR_SUCCESS, COLOR_SUCCESS_BG, RGB(24, 92, 70));
                }
            }

        } else if (g_app.active_dash_tab == DashboardTab::Security) {
            /* Card 1: Account Identity & Username Change */
            RECT userBox = { cx, 114, cx + cw, 222 };
            draw_section_card(hdc, userBox, "ACCOUNT IDENTITY & USERNAME CHANGE", "RE-BIND");

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lNewU = { cx + 20, 154, cx + 270, 169 };
            DrawTextA(hdc, "NEW DESIRED USERNAME", -1, &lNewU, DT_LEFT | DT_NOPREFIX);
            RECT fNewU = { cx + 20, 169, cx + 270, 203 }; draw_input_box(hdc, fNewU, g_app.hEditChangeNewUser);

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lPass = { cx + 285, 154, cx + 535, 169 };
            DrawTextA(hdc, "CONFIRM CURRENT PASSWORD", -1, &lPass, DT_LEFT | DT_NOPREFIX);
            RECT fPass = { cx + 285, 169, cx + 535, 203 }; draw_input_box(hdc, fPass, g_app.hEditChangePass);

            RECT bChg = { cx + 550, 169, cx + 720, 203 }; draw_primary_button(hdc, bChg, "Update Username", g_app.hovered_btn == ID_BTN_CHANGE_USERNAME);

            /* Card 2: Two-Factor Authentication (TOTP) */
            RECT tfaBox = { cx, 234, cx + cw, 420 };
            draw_section_card(hdc, tfaBox, "TWO-FACTOR AUTHENTICATION (TOTP)", "RFC 6238");

            RECT bSetup = { cx + 20, 278, cx + 185, 310 };
            draw_primary_button(hdc, bSetup, "Generate Secret", g_app.hovered_btn == ID_BTN_SETUP_2FA);

            RECT secPill = { cx + 200, 278, cx + cw - 20, 310 };
            draw_filled_round_rect(hdc, secPill, 6, COLOR_INPUT_BG, RGB(32, 38, 54));
            SetTextColor(hdc, g_app.has_tfa_setup ? COLOR_SUCCESS : COLOR_TEXT_MUTED);
            SelectObject(hdc, g_app.hFontMono);
            std::string sbuf = g_app.has_tfa_setup ? ("TOTP Secret: " + g_app.tfa_setup.secret) : "Click 'Generate Secret' to enroll an authenticator app.";
            RECT sr = { secPill.left + 12, secPill.top, secPill.right - 12, secPill.bottom };
            DrawTextA(hdc, sbuf.c_str(), -1, &sr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lCode = { cx + 20, 322, cx + 200, 337 };
            DrawTextA(hdc, "CONFIRM TOTP CODE", -1, &lCode, DT_LEFT | DT_NOPREFIX);

            RECT fCode = { cx + 20, 338, cx + 200, 372 }; draw_input_box(hdc, fCode, g_app.hEdit2FACode);
            RECT bConf = { cx + 212, 338, cx + 336, 372 }; draw_primary_button(hdc, bConf, "Confirm 2FA", g_app.hovered_btn == ID_BTN_CONFIRM_2FA);

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lDisPass = { cx + 375, 322, cx + 515, 337 };
            DrawTextA(hdc, "PASSWORD", -1, &lDisPass, DT_LEFT | DT_NOPREFIX);
            RECT fDisPass = { cx + 375, 338, cx + 515, 372 }; draw_input_box(hdc, fDisPass, g_app.hEditDisablePass);

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lDisCode = { cx + 525, 322, cx + 629, 337 };
            DrawTextA(hdc, "2FA CODE", -1, &lDisCode, DT_LEFT | DT_NOPREFIX);
            RECT fDisCode = { cx + 525, 338, cx + 629, 372 }; draw_input_box(hdc, fDisCode, g_app.hEditDisableCode);

            RECT bDis = { cx + 640, 338, cx + cw - 20, 372 }; draw_secondary_button(hdc, bDis, "Disable 2FA", g_app.hovered_btn == ID_BTN_DISABLE_2FA);

            /* HWID Blacklist & Integrity Box */
            RECT diagBox = { cx, 432, cx + cw, 618 };
            draw_section_card(hdc, diagBox, "SECURITY INTEGRITY & EMERGENCY KILLSWITCH", "PROTECTION");

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lh = { cx + 20, 466, cx + 370, 480 };
            DrawTextA(hdc, "HARDWARE ID (BLACKLIST CHECK)", -1, &lh, DT_LEFT | DT_NOPREFIX);
            RECT fHwid = { cx + 20, 480, cx + 370, 514 }; draw_input_box(hdc, fHwid, g_app.hEditCheckHwid);
            RECT bCheckBl = { cx + 385, 480, cx + 535, 514 }; draw_secondary_button(hdc, bCheckBl, "Check Blacklist", g_app.hovered_btn == ID_BTN_CHECK_BL);

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lha = { cx + 20, 517, cx + 370, 531 };
            DrawTextA(hdc, "FILE INTEGRITY HASH (SHA-256)", -1, &lha, DT_LEFT | DT_NOPREFIX);
            RECT fHash = { cx + 20, 531, cx + 370, 565 }; draw_input_box(hdc, fHash, g_app.hEditVerifyHash);
            RECT bHash = { cx + 385, 531, cx + 535, 565 }; draw_secondary_button(hdc, bHash, "Verify Hash", g_app.hovered_btn == ID_BTN_VERIFY_HASH);

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lkr = { cx + 20, 568, cx + 370, 582 };
            DrawTextA(hdc, "KILLSWITCH REASON (EMERGENCY LOCKOUT)", -1, &lkr, DT_LEFT | DT_NOPREFIX);
            RECT fReason = { cx + 20, 582, cx + 370, 612 }; draw_input_box(hdc, fReason, g_app.hEditBanReason);
            RECT bKill = { cx + 385, 582, cx + 535, 612 }; draw_danger_button(hdc, bKill, "Killswitch / Ban", g_app.hovered_btn == ID_BTN_KILLSWITCH);

        } else if (g_app.active_dash_tab == DashboardTab::FilesHooks) {
            RECT fileBox = { cx, 114, cx + cw, 286 };
            draw_section_card(hdc, fileBox, "PROTECTED REMOTE FILE DOWNLOADER", "SCOPED TICKET");

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lfid = { cx + 20, 158, cx + 380, 173 };
            DrawTextA(hdc, "FILE ID / ASSET IDENTIFIER", -1, &lfid, DT_LEFT | DT_NOPREFIX);

            RECT fFile = { cx + 20, 174, cx + 380, 208 }; draw_input_box(hdc, fFile, g_app.hEditFileId);
            RECT bDown = { cx + 395, 174, cx + 555, 208 }; draw_primary_button(hdc, bDown, "Download Asset", g_app.hovered_btn == ID_BTN_DOWNLOAD_FILE);

            RECT dStatusBox = { cx + 20, 224, cx + cw - 20, 266 };
            draw_filled_round_rect(hdc, dStatusBox, 6, COLOR_INPUT_BG, RGB(30, 36, 50));
            SetTextColor(hdc, !g_app.download_status.empty() ? (g_app.download_status_ok ? COLOR_SUCCESS : COLOR_ERROR) : COLOR_TEXT_MUTED);
            SelectObject(hdc, g_app.hFontMono);
            RECT rDown = { dStatusBox.left + 12, dStatusBox.top, dStatusBox.right - 12, dStatusBox.bottom };
            DrawTextA(hdc, !g_app.download_status.empty() ? g_app.download_status.c_str() : "Ready to request scoped download ticket.", -1, &rDown, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

            RECT hookBox = { cx, 304, cx + cw, 618 };
            draw_section_card(hdc, hookBox, "AUTHENTICATED WEBHOOK TRIGGER", "SERVER-SIDE");

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lhid = { cx + 20, 348, cx + 380, 363 };
            DrawTextA(hdc, "WEBHOOK IDENTIFIER / ID", -1, &lhid, DT_LEFT | DT_NOPREFIX);

            RECT fHookId = { cx + 20, 364, cx + 380, 398 }; draw_input_box(hdc, fHookId, g_app.hEditHookId);

            SetTextColor(hdc, COLOR_TEXT_LABEL);
            SelectObject(hdc, g_app.hFontLabel);
            RECT lhpl = { cx + 20, 418, cx + 380, 433 };
            DrawTextA(hdc, "JSON PAYLOAD (DATA)", -1, &lhpl, DT_LEFT | DT_NOPREFIX);

            RECT fHookPayload = { cx + 20, 434, cx + 380, 468 }; draw_input_box(hdc, fHookPayload, g_app.hEditHookPayload);
            RECT bHook = { cx + 395, 434, cx + 555, 468 }; draw_primary_button(hdc, bHook, "Trigger Webhook", g_app.hovered_btn == ID_BTN_TRIGGER_HOOK);

            RECT hRespBox = { cx + 20, 486, cx + cw - 20, 598 };
            draw_filled_round_rect(hdc, hRespBox, 6, COLOR_INPUT_BG, RGB(30, 36, 50));
            SetTextColor(hdc, !g_app.webhook_response.empty() ? (g_app.webhook_response_ok ? COLOR_SUCCESS : COLOR_ERROR) : COLOR_TEXT_MUTED);
            SelectObject(hdc, g_app.hFontMono);
            RECT rHook = { hRespBox.left + 12, hRespBox.top + 10, hRespBox.right - 12, hRespBox.bottom - 10 };
            DrawTextA(hdc, !g_app.webhook_response.empty() ? g_app.webhook_response.c_str() : "Webhook response payload will be displayed here...", -1, &rHook, DT_WORDBREAK | DT_NOPREFIX);
        }
    }


    /* Outer Window Frame Border */
    HPEN framePen = CreatePen(PS_SOLID, 1, RGB(42, 49, 68));
    HGDIOBJ oldFP = SelectObject(hdc, framePen);
    HGDIOBJ oldFB = SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, width, height);
    SelectObject(hdc, oldFB);
    SelectObject(hdc, oldFP);
    DeleteObject(framePen);

    BitBlt(hdcMem, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

    SelectObject(hdc, oldBmp);
    DeleteObject(hbm);
    DeleteDC(hdc);
    EndPaint(hWnd, &ps);
}

static std::string get_text(HWND h) {
    if (!h) return "";
    int len = GetWindowTextLengthA(h);
    if (len <= 0) return "";
    std::string s(len, '\0');
    GetWindowTextA(h, &s[0], len + 1);
    return s;
}

static void handle_action_click() {
    if (g_app.is_busy) return;
    if (!g_app.sdk_ready) {
        set_status("Client is unconfigured. Please configure application credentials.", COLOR_ERROR);
        return;
    }
    g_app.is_busy = true;
    set_status("Connecting to secure cloud...", COLOR_BLUE_GLOW);

    std::string user  = get_text(g_app.hEditUser);
    std::string pass  = get_text(g_app.hEditPass);
    std::string key   = get_text(g_app.hEditKey);
    std::string code  = get_text(g_app.hEdit2FA);
    std::string mail  = get_text(g_app.hEditEmail);
    std::string tok   = get_text(g_app.hEditResetToken);
    std::string npass = get_text(g_app.hEditNewPass);

    bool ok = false;

    if (g_app.current_view == ViewMode::Login) {
        if (user.empty() || pass.empty()) {
            set_status("Please enter username and password.", COLOR_ERROR);
            g_app.is_busy = false;
            return;
        }
        ok = g_app.api->login(user, pass, code);
    } else if (g_app.current_view == ViewMode::Register) {
        if (user.empty() || pass.empty() || key.empty()) {
            set_status("Please provide username, password and license key.", COLOR_ERROR);
            g_app.is_busy = false;
            return;
        }
        ok = g_app.api->registerAccount(user, pass, key, mail);
    } else if (g_app.current_view == ViewMode::License) {
        if (key.empty()) {
            set_status("Please enter a valid license key.", COLOR_ERROR);
            g_app.is_busy = false;
            return;
        }
        ok = g_app.api->license(key);
    } else if (g_app.current_view == ViewMode::Reset) {
        if (user.empty() || pass.empty()) {
            set_status("Please enter username and password.", COLOR_ERROR);
            g_app.is_busy = false;
            return;
        }
        ok = g_app.api->resetHwid(user, pass, code);
    } else if (g_app.current_view == ViewMode::Upgrade) {
        if (user.empty() || key.empty()) {
            set_status("Please enter username and upgrade key.", COLOR_ERROR);
            g_app.is_busy = false;
            return;
        }
        ok = g_app.api->upgrade(user, key);
    } else if (g_app.current_view == ViewMode::Forgot) {
        if (mail.empty()) {
            set_status("Please enter registered email address.", COLOR_ERROR);
            g_app.is_busy = false;
            return;
        }
        ok = g_app.api->forgotPassword(mail);
    } else if (g_app.current_view == ViewMode::TokenReset) {
        if (tok.empty() || npass.empty()) {
            set_status("Please enter reset token and new password.", COLOR_ERROR);
            g_app.is_busy = false;
            return;
        }
        ok = g_app.api->resetPassword(tok, npass);
    }

    g_app.is_busy = false;

    if (ok) {
        if (g_app.current_view == ViewMode::Login || g_app.current_view == ViewMode::Register || g_app.current_view == ViewMode::License) {
            g_app.authenticated = true;
            g_app.current_view = ViewMode::Dashboard;
            g_app.active_dash_tab = DashboardTab::Overview;
            set_status("Session authenticated securely.", COLOR_SUCCESS);
            update_controls_visibility();
        } else {
            set_status(g_app.api->response.message, COLOR_SUCCESS);
        }
    } else {
        set_status(g_app.api->response.message.empty() ? "Operation failed." : g_app.api->response.message, COLOR_ERROR);
    }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_app.hWnd = hWnd;

            g_app.hbrCanvas     = CreateSolidBrush(COLOR_CANVAS_BLACK);
            g_app.hbrSidebar    = CreateSolidBrush(COLOR_SIDEBAR_BG);
            g_app.hbrHeader     = CreateSolidBrush(COLOR_HEADER_BG);
            g_app.hbrCard       = CreateSolidBrush(COLOR_CARD_BG);
            g_app.hbrCardHeader = CreateSolidBrush(COLOR_CARD_HEADER_BG);
            g_app.hbrInput      = CreateSolidBrush(COLOR_INPUT_BG);

            g_app.hFontBrand   = CreateFontA(-15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_app.hFontHero    = CreateFontA(-22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_app.hFontTitle   = CreateFontA(-19, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_app.hFontSection = CreateFontA(-13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_app.hFontBody    = CreateFontA(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_app.hFontInput   = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_app.hFontLabel   = CreateFontA(-10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_app.hFontBadge   = CreateFontA(-10, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_app.hFontStatus  = CreateFontA(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            g_app.hFontMono    = CreateFontA(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
            g_app.hFontKpi     = CreateFontA(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

            /* Create 32-bit premultiplied BGRA DIB section for the NightAuth logo */
            BITMAPINFO bmi;
            ZeroMemory(&bmi, sizeof(bmi));
            bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth       = LOGO_WIDTH;
            bmi.bmiHeader.biHeight      = -LOGO_HEIGHT; /* top-down */
            bmi.bmiHeader.biPlanes      = 1;
            bmi.bmiHeader.biBitCount    = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            void* pvBits = nullptr;
            HDC hdcScreen = GetDC(NULL);
            g_app.hbmLogo = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
            ReleaseDC(NULL, hdcScreen);
            if (g_app.hbmLogo && pvBits) {
                std::memcpy(pvBits, g_logo_bgra, sizeof(g_logo_bgra));
            }

            HINSTANCE hInst = GetModuleHandle(NULL);

            g_app.hEditUser       = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditPass       = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_PASSWORD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditKey        = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEdit2FA        = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditEmail      = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditResetToken = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditNewPass    = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_PASSWORD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);

            g_app.hEditChangeNewUser = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditChangePass    = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_PASSWORD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditGlobalKey   = CreateWindowExA(0, "EDIT", "version", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditVarKey      = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditVarVal      = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditChatRoom    = CreateWindowExA(0, "EDIT", "general", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditChatMsg     = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEdit2FACode     = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditDisablePass = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_PASSWORD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditDisableCode = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditCheckHwid   = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditVerifyHash  = CreateWindowExA(0, "EDIT", "", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditBanReason   = CreateWindowExA(0, "EDIT", "Security violation", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditFileId      = CreateWindowExA(0, "EDIT", "1", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditHookId      = CreateWindowExA(0, "EDIT", "1", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
            g_app.hEditHookPayload = CreateWindowExA(0, "EDIT", "{\"alert\":\"critical\"}", WS_CHILD | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);

            SendMessageW(g_app.hEditUser, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter username");
            SendMessageW(g_app.hEditPass, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter password");
            SendMessageW(g_app.hEditKey, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter license key");
            SendMessageW(g_app.hEdit2FA, EM_SETCUEBANNER, TRUE, (LPARAM)L"2FA code (if enabled)");
            SendMessageW(g_app.hEditEmail, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter email address");
            SendMessageW(g_app.hEditResetToken, EM_SETCUEBANNER, TRUE, (LPARAM)L"Reset token from email");
            SendMessageW(g_app.hEditNewPass, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter new password");

            HWND edits[] = {
                g_app.hEditUser, g_app.hEditPass, g_app.hEditKey, g_app.hEdit2FA,
                g_app.hEditEmail, g_app.hEditResetToken, g_app.hEditNewPass,
                g_app.hEditGlobalKey, g_app.hEditVarKey, g_app.hEditVarVal,
                g_app.hEditChatRoom, g_app.hEditChatMsg,
        g_app.hEditChangeNewUser, g_app.hEditChangePass,
        g_app.hEdit2FACode,
                g_app.hEditDisablePass, g_app.hEditDisableCode, g_app.hEditCheckHwid,
                g_app.hEditVerifyHash, g_app.hEditBanReason, g_app.hEditFileId,
                g_app.hEditHookId, g_app.hEditHookPayload
            };
            for (HWND h : edits) {
                if (h) {
                    SendMessage(h, WM_SETFONT, (WPARAM)g_app.hFontInput, TRUE);
                    SendMessage(h, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(8, 8));
                }
            }

            update_controls_visibility();
            return 0;
        }

        case WM_COMMAND: {
            if (HIWORD(wParam) == EN_SETFOCUS || HIWORD(wParam) == EN_KILLFOCUS) {
                InvalidateRect(hWnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, COLOR_TEXT_HEAD);
            SetBkColor(hdc, COLOR_INPUT_BG);
            return (LRESULT)g_app.hbrInput;
        }

        case WM_NCHITTEST: {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(hWnd, &pt);
            if (pt.y <= TITLEBAR_H && pt.x < (WIN_WIDTH - 84)) {
                return HTCAPTION;
            }
            return DefWindowProcA(hWnd, msg, wParam, lParam);
        }

        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            int new_hovered = 0;

            if (y <= TITLEBAR_H) {
                if (x >= WIN_WIDTH - 76 && x < WIN_WIDTH - 44) new_hovered = ID_BTN_MINIMIZE;
                else if (x >= WIN_WIDTH - 40 && x <= WIN_WIDTH - 8) new_hovered = ID_BTN_CLOSE;
            } else if (g_app.current_view != ViewMode::Dashboard) {
                int cardW = 860;
                int cardX = (WIN_WIDTH - cardW) / 2;
                int cardY = 68;
                int formX = cardX + 340;
                int formW = cardW - 340;
                int fieldX = formX + 36;
                int fieldW = formW - 72;
                int tabGap = 5;
                int tabW = (fieldW - tabGap * 6) / 7;
                int tabY = cardY + 26;

                if (y >= tabY && y <= tabY + 30) {
                    for (int i = 0; i < 7; ++i) {
                        int tx = fieldX + i * (tabW + tabGap);
                        if (x >= tx && x <= tx + tabW) {
                            new_hovered = ID_TAB_PRE_BASE + i;
                            break;
                        }
                    }
                } else if (y >= 456 && y <= 500 && x >= fieldX && x <= fieldX + fieldW) {
                    new_hovered = ID_BTN_ACTION;
                }
            } else {
                int contentBottom = WIN_HEIGHT - STATUSBAR_H;
                if (x <= SIDEBAR_W) {
                    int sPadY = TITLEBAR_H + 38;
                    int sTabH = 38;
                    int sGap = 5;
                    for (int i = 0; i < static_cast<int>(DashboardTab::Count); ++i) {
                        int ty = sPadY + i * (sTabH + sGap);
                        if (y >= ty && y <= ty + sTabH && x >= 12 && x <= SIDEBAR_W - 12) {
                            new_hovered = ID_DASH_TAB_BASE + i;
                            break;
                        }
                    }
                    if (y >= contentBottom - 46 && y <= contentBottom - 12 && x >= 12 && x <= SIDEBAR_W - 12) {
                        new_hovered = ID_BTN_LOGOUT;
                    }
                } else {
                    int cx = SIDEBAR_W + 24;
                    int cw = WIN_WIDTH - cx - 24;
                    if (g_app.active_dash_tab == DashboardTab::Overview) {
                        if (x >= cx + cw - 196 && x <= cx + cw - 20 && y >= 294 && y <= 336) new_hovered = ID_BTN_HEARTBEAT;
                    } else if (g_app.active_dash_tab == DashboardTab::Variables) {
                        if (x >= cx + 295 && x <= cx + 425 && y >= 174 && y <= 208) new_hovered = ID_BTN_GET_GLOBAL;
                        else if (x >= cx + 485 && x <= cx + 590 && y >= 309 && y <= 343) new_hovered = ID_BTN_GET_USER_VAR;
                        else if (x >= cx + 600 && x <= cx + 705 && y >= 309 && y <= 343) new_hovered = ID_BTN_SET_USER_VAR;
                        else if (x >= cx + cw - 156 && x <= cx + cw - 18 && y >= 405 && y <= 434) new_hovered = ID_BTN_GET_ALL_VARS;
                    } else if (g_app.active_dash_tab == DashboardTab::Chat) {
                        if (x >= cx + 235 && x <= cx + 365 && y >= 164 && y <= 198) new_hovered = ID_BTN_REFRESH_CHAT;
                        else if (x >= cx + cw - 132 && x <= cx + cw - 20 && y >= 571 && y <= 605) new_hovered = ID_BTN_SEND_CHAT;
                    } else if (g_app.active_dash_tab == DashboardTab::Stats) {
                        if (x >= cx + cw - 156 && x <= cx + cw - 18 && y >= 235 && y <= 264) new_hovered = ID_BTN_REFRESH_STATS;
                    } else if (g_app.active_dash_tab == DashboardTab::Security) {
                        if (x >= cx + 550 && x <= cx + 720 && y >= 169 && y <= 203) new_hovered = ID_BTN_CHANGE_USERNAME;
                        else if (x >= cx + 20 && x <= cx + 185 && y >= 278 && y <= 310) new_hovered = ID_BTN_SETUP_2FA;
                        else if (x >= cx + 212 && x <= cx + 336 && y >= 338 && y <= 372) new_hovered = ID_BTN_CONFIRM_2FA;
                        else if (x >= cx + 640 && x <= cx + cw - 20 && y >= 338 && y <= 372) new_hovered = ID_BTN_DISABLE_2FA;
                        else if (x >= cx + 385 && x <= cx + 535 && y >= 480 && y <= 514) new_hovered = ID_BTN_CHECK_BL;
                        else if (x >= cx + 385 && x <= cx + 535 && y >= 531 && y <= 565) new_hovered = ID_BTN_VERIFY_HASH;
                        else if (x >= cx + 385 && x <= cx + 535 && y >= 582 && y <= 612) new_hovered = ID_BTN_KILLSWITCH;
                    } else if (g_app.active_dash_tab == DashboardTab::FilesHooks) {
                        if (x >= cx + 395 && x <= cx + 555 && y >= 174 && y <= 208) new_hovered = ID_BTN_DOWNLOAD_FILE;
                        else if (x >= cx + 395 && x <= cx + 555 && y >= 434 && y <= 468) new_hovered = ID_BTN_TRIGGER_HOOK;
                    }
                }
            }

            if (new_hovered != g_app.hovered_btn) {
                g_app.hovered_btn = new_hovered;
                InvalidateRect(hWnd, NULL, FALSE);
            }

            TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hWnd, 0 };
            TrackMouseEvent(&tme);
            return 0;
        }

        case WM_MOUSELEAVE: {
            if (g_app.hovered_btn != 0) {
                g_app.hovered_btn = 0;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            if (y <= TITLEBAR_H) {
                if (x >= WIN_WIDTH - 76 && x < WIN_WIDTH - 44) {
                    ShowWindow(hWnd, SW_MINIMIZE);
                    return 0;
                } else if (x >= WIN_WIDTH - 40 && x <= WIN_WIDTH - 8) {
                    PostQuitMessage(0);
                    return 0;
                }
                ReleaseCapture();
                SendMessageA(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                return 0;
            }

            if (g_app.current_view != ViewMode::Dashboard) {
                int cardW = 860;
                int cardX = (WIN_WIDTH - cardW) / 2;
                int cardY = 68;
                int formX = cardX + 340;
                int formW = cardW - 340;
                int fieldX = formX + 36;
                int fieldW = formW - 72;
                int tabGap = 5;
                int tabW = (fieldW - tabGap * 6) / 7;
                int tabY = cardY + 26;

                if (y >= tabY && y <= tabY + 30) {
                    for (int i = 0; i < 7; ++i) {
                        int tx = fieldX + i * (tabW + tabGap);
                        if (x >= tx && x <= tx + tabW) {
                            g_app.current_view = static_cast<ViewMode>(i);
                            update_controls_visibility();
                            set_status("Ready.", COLOR_TEXT_MUTED);
                            return 0;
                        }
                    }
                }

                if (y >= 456 && y <= 500 && x >= fieldX && x <= fieldX + fieldW) {
                    handle_action_click();
                    return 0;
                }
                if (x < formX || y < cardY || y > (cardY + 540)) {
                    ReleaseCapture();
                    SendMessageA(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
                    return 0;
                }
            } else {
                int contentBottom = WIN_HEIGHT - STATUSBAR_H;
                if (x <= SIDEBAR_W) {
                    int sPadY = TITLEBAR_H + 38;
                    int sTabH = 38;
                    int sGap = 5;
                    for (int i = 0; i < static_cast<int>(DashboardTab::Count); ++i) {
                        int ty = sPadY + i * (sTabH + sGap);
                        if (y >= ty && y <= ty + sTabH && x >= 12 && x <= SIDEBAR_W - 12) {
                            g_app.active_dash_tab = static_cast<DashboardTab>(i);
                            update_controls_visibility();
                            set_status("Module loaded.", COLOR_TEXT_MUTED);
                            return 0;
                        }
                    }
                    if (y >= contentBottom - 46 && y <= contentBottom - 12 && x >= 12 && x <= SIDEBAR_W - 12) {
                        g_app.api->logout();
                        g_app.authenticated = false;
                        g_app.current_view = ViewMode::Login;
                        update_controls_visibility();
                        set_status("Signed out cleanly.", COLOR_TEXT_MUTED);
                        return 0;
                    }
                } else {
                    int cx = SIDEBAR_W + 24;
                    int cw = WIN_WIDTH - cx - 24;

                    if (g_app.active_dash_tab == DashboardTab::Overview) {
                        if (x >= cx + cw - 196 && x <= cx + cw - 20 && y >= 294 && y <= 336) {
                            bool ok = g_app.api->heartbeat();
                            set_status(ok ? "Heartbeat acknowledged by cloud." : "Heartbeat failed.", ok ? COLOR_SUCCESS : COLOR_ERROR);
                            return 0;
                        }
                    } else if (g_app.active_dash_tab == DashboardTab::Variables) {
                        if (x >= cx + 295 && x <= cx + 425 && y >= 174 && y <= 208) {
                            std::string key = get_text(g_app.hEditGlobalKey);
                            g_app.last_global_val = g_app.api->getGlobalVar(key);
                            set_status(!g_app.last_global_val.empty() ? "Global variable fetched." : "Failed to fetch global variable.", !g_app.last_global_val.empty() ? COLOR_SUCCESS : COLOR_ERROR);
                            InvalidateRect(hWnd, NULL, FALSE);
                            return 0;
                        } else if (x >= cx + 485 && x <= cx + 590 && y >= 309 && y <= 343) {
                            std::string key = get_text(g_app.hEditVarKey);
                            g_app.last_user_val = g_app.api->getUserVar(key);
                            set_status(!g_app.last_user_val.empty() ? "User variable retrieved." : "User variable not found.", !g_app.last_user_val.empty() ? COLOR_SUCCESS : COLOR_ERROR);
                            InvalidateRect(hWnd, NULL, FALSE);
                            return 0;
                        } else if (x >= cx + 600 && x <= cx + 705 && y >= 309 && y <= 343) {
                            std::string key = get_text(g_app.hEditVarKey);
                            std::string val = get_text(g_app.hEditVarVal);
                            bool ok = g_app.api->setUserVar(key, val);
                            set_status(ok ? "User variable saved to cloud." : "Failed to save user variable.", ok ? COLOR_SUCCESS : COLOR_ERROR);
                            return 0;
                        } else if (x >= cx + cw - 156 && x <= cx + cw - 18 && y >= 405 && y <= 434) {
                            bool ok = g_app.api->getUserVariables(g_app.user_vars);
                            set_status(ok ? "User variables refreshed." : "Failed to load variables.", ok ? COLOR_SUCCESS : COLOR_ERROR);
                            InvalidateRect(hWnd, NULL, FALSE);
                            return 0;
                        }
                    } else if (g_app.active_dash_tab == DashboardTab::Chat) {
                        if (x >= cx + 235 && x <= cx + 365 && y >= 164 && y <= 198) {
                            std::string room = get_text(g_app.hEditChatRoom);
                            g_app.chat_msgs = g_app.api->getChatMessages(room);
                            set_status("Chat channel synchronized.", COLOR_SUCCESS);
                            InvalidateRect(hWnd, NULL, FALSE);
                            return 0;
                        } else if (x >= cx + cw - 132 && x <= cx + cw - 20 && y >= 571 && y <= 605) {
                            std::string msgText = get_text(g_app.hEditChatMsg);
                            std::string room = get_text(g_app.hEditChatRoom);
                            if (!msgText.empty()) {
                                if (g_app.api->sendChatMessage(msgText, room)) {
                                    SetWindowTextA(g_app.hEditChatMsg, "");
                                    g_app.chat_msgs = g_app.api->getChatMessages(room);
                                    set_status("Message sent.", COLOR_SUCCESS);
                                } else {
                                    set_status("Failed to send chat message.", COLOR_ERROR);
                                }
                                InvalidateRect(hWnd, NULL, FALSE);
                            }
                            return 0;
                        }
                    } else if (g_app.active_dash_tab == DashboardTab::Stats) {
                        if (x >= cx + cw - 156 && x <= cx + cw - 18 && y >= 235 && y <= 264) {
                            g_app.has_stats = g_app.api->getStats(g_app.app_stats);
                            g_app.online_users = g_app.api->getOnlineUsers();
                            set_status("Metrics and active sessions refreshed.", COLOR_SUCCESS);
                            InvalidateRect(hWnd, NULL, FALSE);
                            return 0;
                        }
                    } else if (g_app.active_dash_tab == DashboardTab::Security) {
                        if (x >= cx + 550 && x <= cx + 720 && y >= 169 && y <= 203) {
                            std::string newU = get_text(g_app.hEditChangeNewUser);
                            std::string pass = get_text(g_app.hEditChangePass);
                            if (newU.empty() || pass.empty()) {
                                set_status("Please enter new username and current password.", COLOR_ERROR);
                                return 0;
                            }
                            set_status("Updating username with cloud...", COLOR_BLUE_GLOW);
                            UpdateWindow(hWnd);
                            if (g_app.api->changeUsername(newU, pass)) {
                                MessageBoxA(hWnd,
                                    "Username updated successfully! For security, your active session has been terminated. Please sign in again with your new username.",
                                    "Username Changed", MB_ICONINFORMATION | MB_OK);
                                g_app.authenticated = false;
                                g_app.active_dash_tab = DashboardTab::Overview;
                                g_app.current_view = ViewMode::Login;
                                SetWindowTextA(g_app.hEditUser, newU.c_str());
                                SetWindowTextA(g_app.hEditPass, "");
                                SetWindowTextA(g_app.hEditChangeNewUser, "");
                                SetWindowTextA(g_app.hEditChangePass, "");
                                set_status("Session closed. Please sign in with your new username.", COLOR_SUCCESS);
                                update_controls_visibility();
                            } else {
                                set_status(g_app.api->response.message.empty() ? "Failed to change username." : g_app.api->response.message, COLOR_ERROR);
                            }
                            return 0;
                        } else if (x >= cx + 20 && x <= cx + 185 && y >= 278 && y <= 310) {
                            g_app.has_tfa_setup = g_app.api->setup2FA(g_app.tfa_setup);
                            set_status(g_app.has_tfa_setup ? "2FA Setup secret generated." : "Failed to setup 2FA.", g_app.has_tfa_setup ? COLOR_SUCCESS : COLOR_ERROR);
                            InvalidateRect(hWnd, NULL, FALSE);
                            return 0;
                        } else if (x >= cx + 212 && x <= cx + 336 && y >= 338 && y <= 372) {
                            std::string code = get_text(g_app.hEdit2FACode);
                            NightAuth::TwoFactorConfirmData conf;
                            bool ok = g_app.api->confirm2FA(code, conf);
                            set_status(ok ? "2FA confirmed and activated." : "Invalid 2FA code.", ok ? COLOR_SUCCESS : COLOR_ERROR);
                            return 0;
                        } else if (x >= cx + 640 && x <= cx + cw - 20 && y >= 338 && y <= 372) {
                            std::string pass = get_text(g_app.hEditDisablePass);
                            std::string code = get_text(g_app.hEditDisableCode);
                            bool ok = g_app.api->disable2FA(pass, code);
                            set_status(ok ? "2FA disabled successfully." : "Failed to disable 2FA.", ok ? COLOR_SUCCESS : COLOR_ERROR);
                            return 0;
                        } else if (x >= cx + 385 && x <= cx + 535 && y >= 480 && y <= 514) {
                            std::string hwid = get_text(g_app.hEditCheckHwid);
                            bool is_bl = g_app.api->checkBlacklist(hwid);
                            set_status(is_bl ? "HWID is BLACKLISTED." : "HWID is clean (not blacklisted).", is_bl ? COLOR_ERROR : COLOR_SUCCESS);
                            return 0;
                        } else if (x >= cx + 385 && x <= cx + 535 && y >= 531 && y <= 565) {
                            std::string hash = get_text(g_app.hEditVerifyHash);
                            bool ok = g_app.api->verifyFileHash(hash);
                            set_status(ok ? "Hash verified with server signature." : "Hash verification failed / mismatched.", ok ? COLOR_SUCCESS : COLOR_ERROR);
                            return 0;
                        } else if (x >= cx + 385 && x <= cx + 535 && y >= 582 && y <= 612) {
                            std::string reason = get_text(g_app.hEditBanReason);
                            bool ok = g_app.api->ban(reason);
                            set_status(ok ? "Account banned / killswitch executed." : "Failed to execute ban.", ok ? COLOR_SUCCESS : COLOR_ERROR);
                            return 0;
                        }
                    } else if (g_app.active_dash_tab == DashboardTab::FilesHooks) {
                        if (x >= cx + 395 && x <= cx + 555 && y >= 174 && y <= 208) {
                            std::string fileId = get_text(g_app.hEditFileId);
                            std::string fileContent;
                            if (g_app.api->downloadFile(fileId, fileContent)) {
                                g_app.download_status_ok = true;
                                g_app.download_status = "Downloaded " + std::to_string(fileContent.size()) + " bytes successfully.";
                                set_status("File downloaded successfully.", COLOR_SUCCESS);
                            } else {
                                g_app.download_status_ok = false;
                                g_app.download_status = "Download failed for file ID: " + fileId;
                                set_status("File download failed.", COLOR_ERROR);
                            }
                            InvalidateRect(hWnd, NULL, FALSE);
                            return 0;
                        } else if (x >= cx + 395 && x <= cx + 555 && y >= 434 && y <= 468) {
                            std::string hookId = get_text(g_app.hEditHookId);
                            std::string payload = get_text(g_app.hEditHookPayload);
                            NightAuth::WebhookResult res;
                            try {
                                if (g_app.api->triggerWebhook(hookId, payload, res)) {
                                    g_app.webhook_response_ok = true;
                                    g_app.webhook_response = "HTTP " + std::to_string(res.status) + "  |  " + g_app.api->response.message;
                                    set_status("Webhook executed successfully.", COLOR_SUCCESS);
                                } else {
                                    g_app.webhook_response_ok = false;
                                    g_app.webhook_response = "Webhook dispatch failed: " + g_app.api->response.message;
                                    set_status("Webhook failed.", COLOR_ERROR);
                                }
                            } catch (const std::exception& ex) {
                                g_app.webhook_response_ok = false;
                                g_app.webhook_response = std::string("Invalid payload: ") + ex.what();
                                set_status("Webhook payload error.", COLOR_ERROR);
                            }
                            InvalidateRect(hWnd, NULL, FALSE);
                            return 0;
                        }
                    }
                }
            }
            return 0;
        }

        case WM_PAINT:
            on_paint(hWnd);
            return 0;

        case WM_DESTROY:
            DeleteObject(g_app.hbrCanvas);
            DeleteObject(g_app.hbrSidebar);
            DeleteObject(g_app.hbrHeader);
            DeleteObject(g_app.hbrCard);
            DeleteObject(g_app.hbrCardHeader);
            DeleteObject(g_app.hbrInput);

            DeleteObject(g_app.hFontBrand);
            DeleteObject(g_app.hFontHero);
            DeleteObject(g_app.hFontTitle);
            DeleteObject(g_app.hFontSection);
            DeleteObject(g_app.hFontBody);
            DeleteObject(g_app.hFontInput);
            DeleteObject(g_app.hFontLabel);
            DeleteObject(g_app.hFontBadge);
            DeleteObject(g_app.hFontStatus);
            DeleteObject(g_app.hFontMono);
            DeleteObject(g_app.hFontKpi);
            if (g_app.hbmLogo) DeleteObject(g_app.hbmLogo);

            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProcA(hWnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icex);

    WNDCLASSEXA wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(COLOR_CANVAS_BLACK);
    wc.lpszClassName = "NightAuthPureUICPP";

    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Window Registration Failed.", "NightAuth Error", MB_ICONERROR);
        return 1;
    }

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - WIN_WIDTH) / 2;
    int posY = (screenH - WIN_HEIGHT) / 2;

    HWND hWnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        "NightAuthPureUICPP",
        "NightAuth Enterprise C++ Desktop",
        WS_POPUP | WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU,
        posX, posY, WIN_WIDTH, WIN_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        return 1;
    }

    SetWindowPos(hWnd, HWND_TOPMOST, posX, posY, WIN_WIDTH, WIN_HEIGHT, SWP_SHOWWINDOW);
    SetWindowPos(hWnd, HWND_NOTOPMOST, posX, posY, WIN_WIDTH, WIN_HEIGHT, SWP_SHOWWINDOW);
    ShowWindow(hWnd, SW_SHOWNORMAL);
    UpdateWindow(hWnd);
    SetForegroundWindow(hWnd);

    const char* env_name  = std::getenv("NIGHTAUTH_APP_NAME");
    const char* env_key   = std::getenv("NIGHTAUTH_API_KEY");
    const char* env_ver   = std::getenv("NIGHTAUTH_APP_VERSION");
    const char* env_token = std::getenv("NIGHTAUTH_APP_TOKEN");
    const char* env_url   = std::getenv("NIGHTAUTH_API_URL");
    const char* env_loop  = std::getenv("NIGHTAUTH_ALLOW_INSECURE_LOOPBACK");

    std::string app_name  = (env_name && env_name[0]) ? env_name : NightAuthApp.name.c_str();
    std::string app_key   = (env_key && env_key[0]) ? env_key : NightAuthApp.api_key.c_str();
    std::string app_ver   = (env_ver && env_ver[0]) ? env_ver : NightAuthApp.version.c_str();
    std::string app_token = (env_token && env_token[0]) ? env_token : "";
    std::string api_url   = (env_url && env_url[0]) ? env_url : "https://nightauth.org/api/v1";
    bool allow_loopback   = (env_loop && (std::string(env_loop) == "1" || std::string(env_loop) == "true"));

    g_app.api = std::make_unique<NightAuth::Api>(app_name, app_key, app_ver, app_token, api_url);
    g_app.api->setTransport(NightAuth::Api::makeCurlTransport(api_url, allow_loopback));
    g_app.api->setStreamTransport(NightAuth::Api::makeCurlStreamTransport(api_url, allow_loopback));
    set_status("Establishing secure session with cloud...", COLOR_BLUE_GLOW);
    UpdateWindow(hWnd);

    std::thread initThread([hWnd]() {
        try {
            g_app.api->init();
            if (g_app.api->response.success) {
                g_app.sdk_ready = true;
                set_status("Connected securely. Ready to authenticate.", COLOR_SUCCESS);
            } else {
                g_app.sdk_ready = false;
                set_status(g_app.api->response.message.empty() ? "Handshake failed." : g_app.api->response.message, COLOR_ERROR);
            }
        } catch (const std::exception& ex) {
            g_app.sdk_ready = false;
            set_status(std::string("Handshake exception: ") + ex.what(), COLOR_ERROR);
        }
        InvalidateRect(hWnd, NULL, TRUE);
    });
    initThread.detach();

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    curl_global_cleanup();
    return static_cast<int>(msg.wParam);
}
