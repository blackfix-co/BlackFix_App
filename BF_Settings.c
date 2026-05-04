#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include "BF_Settings.h"

typedef enum BFSettingsDrop {
    BF_SETTINGS_DROP_NONE,
    BF_SETTINGS_DROP_THEME,
    BF_SETTINGS_DROP_LANGUAGE
} BFSettingsDrop;

typedef struct BFSettingsState {
    BFFonts fonts;
    RECT themeBox;
    RECT languageBox;
    RECT soundBox;
    RECT volumeTrack;
    BFSettingsDrop openDrop;
} BFSettingsState;

static LRESULT CALLBACK BFSettingsWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static const BFTextKey BF_THEME_TEXT[BF_THEME_COUNT] = {
    BF_TX_THEME_TERMINAL,
    BF_TX_THEME_DARK,
    BF_TX_THEME_SPACE,
    BF_TX_THEME_BOOK,
    BF_TX_THEME_LIGHT,
    BF_TX_THEME_DEEP
};

static const BFTextKey BF_LANGUAGE_TEXT[BF_LANG_COUNT] = {
    BF_TX_LANG_KO,
    BF_TX_LANG_EN,
    BF_TX_LANG_JA
};

static BFSettingsState *BFGetSettingsState(void)
{
    if (BF_SettingsWindow == NULL || !IsWindow(BF_SettingsWindow)) {
        return NULL;
    }
    return (BFSettingsState *)GetWindowLongPtrW(BF_SettingsWindow, GWLP_USERDATA);
}

void BFSettingsSyncControls(void)
{
    BFSettingsState *state = BFGetSettingsState();
    if (state != NULL) {
        InvalidateRect(BF_SettingsWindow, NULL, FALSE);
    }
}

static void BFDrawControlBox(HDC dc, RECT rect, const wchar_t *text, HFONT font, int selected)
{
    const BFPalette *palette = BFP();
    RECT textRect = rect;
    RECT arrow = rect;
    BFDrawBox(dc, rect, selected ? palette->selected : palette->panel, selected ? palette->accent : palette->border, selected ? 2 : 1);

    textRect.left += 10;
    textRect.right -= 34;
    BFDrawTextBlock(dc, text, textRect, font, palette->text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    arrow.left = rect.right - 28;
    arrow.right = rect.right - 8;
    BFDrawTextBlock(dc, L"▼", arrow, font, palette->accent, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

static void BFDrawDropList(HDC dc, RECT box, HFONT font, int count, const BFTextKey *keys, int selected)
{
    const BFPalette *palette = BFP();
    RECT list = box;
    RECT row;
    int i;

    list.top = box.bottom + 2;
    list.bottom = list.top + count * 28;
    BFDrawBox(dc, list, palette->panel, palette->border, 1);

    for (i = 0; i < count; ++i) {
        row.left = list.left + 1;
        row.top = list.top + i * 28;
        row.right = list.right - 1;
        row.bottom = row.top + 28;
        if (i == selected) {
            BFFillRectColor(dc, &row, palette->selected);
        }
        row.left += 10;
        row.right -= 10;
        BFDrawTextBlock(dc, BFT(keys[i]), row, font, i == selected ? palette->accent : palette->text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
}

static int BFHitDropItem(RECT box, int count, int x, int y)
{
    RECT list = box;
    if (x < box.left || x >= box.right) {
        return -1;
    }
    list.top = box.bottom + 2;
    list.bottom = list.top + count * 28;
    if (y < list.top || y >= list.bottom) {
        return -1;
    }
    return (y - list.top) / 28;
}

static void BFDrawSoundToggle(HDC dc, RECT rect, HFONT font)
{
    const BFPalette *palette = BFP();
    RECT mark = rect;
    RECT text = rect;

    mark.right = mark.left + 28;
    BFDrawBox(dc, mark, BF_Settings.sound ? palette->selected : palette->panel, BF_Settings.sound ? palette->accent : palette->border, BF_Settings.sound ? 2 : 1);

    BFDrawTextBlock(dc, BF_Settings.sound ? L"✓" : L"", mark, font, palette->accent, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    text.left = mark.right + 10;
    BFDrawTextBlock(dc, BFT(BF_TX_SOUND), text, font, palette->text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

static void BFLayoutSettings(BFSettingsState *state)
{
    int x = 174;
    int y = 94;
    int w = 300;
    int h = 32;

    state->themeBox.left = x;
    state->themeBox.top = y;
    state->themeBox.right = x + w;
    state->themeBox.bottom = y + h;

    y += 64;
    state->soundBox.left = x;
    state->soundBox.top = y;
    state->soundBox.right = x + w;
    state->soundBox.bottom = y + h;

    y += 62;
    state->languageBox.left = x;
    state->languageBox.top = y;
    state->languageBox.right = x + w;
    state->languageBox.bottom = y + h;

    y += 64;
    state->volumeTrack.left = x;
    state->volumeTrack.top = y;
    state->volumeTrack.right = x + w;
    state->volumeTrack.bottom = y + h;
}

static void BFDrawSettings(HWND hwnd, HDC dc, const RECT *client, BFSettingsState *state)
{
    const BFPalette *palette = BFP();
    RECT rect;
    RECT volumeBounds;

    (void)hwnd;
    BFLayoutSettings(state);
    BFDrawGrid(dc, client);

    rect.left = BF_MARGIN;
    rect.top = 18;
    rect.right = client->right - BF_MARGIN;
    rect.bottom = 58;
    BFDrawTextBlock(dc, BFT(BF_TX_SETTINGS), rect, state->fonts.title, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    rect.left = 38;
    rect.top = 94;
    rect.right = 150;
    rect.bottom = 126;
    BFDrawTextBlock(dc, BFT(BF_TX_THEME), rect, state->fonts.ui, palette->text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    BFDrawControlBox(dc, state->themeBox, BFT(BF_THEME_TEXT[BFClampInt(BF_Settings.theme, 0, BF_THEME_COUNT - 1)]), state->fonts.small, state->openDrop == BF_SETTINGS_DROP_THEME);

    rect.top += 64;
    rect.bottom += 64;
    BFDrawTextBlock(dc, BFT(BF_TX_SOUND), rect, state->fonts.ui, palette->text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    BFDrawSoundToggle(dc, state->soundBox, state->fonts.small);

    rect.top += 62;
    rect.bottom += 62;
    BFDrawTextBlock(dc, BFT(BF_TX_LANGUAGE), rect, state->fonts.ui, palette->text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    BFDrawControlBox(dc, state->languageBox, BFT(BF_LANGUAGE_TEXT[BFClampInt(BF_Settings.language, 0, BF_LANG_COUNT - 1)]), state->fonts.small, state->openDrop == BF_SETTINGS_DROP_LANGUAGE);

    rect.top += 64;
    rect.bottom += 64;
    BFDrawTextBlock(dc, BFT(BF_TX_VOLUME), rect, state->fonts.ui, palette->text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    volumeBounds = state->volumeTrack;
    volumeBounds.left -= 96;
    BFDrawVolume(dc, volumeBounds, state->fonts.small, &state->volumeTrack);

    if (state->openDrop == BF_SETTINGS_DROP_THEME) {
        BFDrawDropList(dc, state->themeBox, state->fonts.small, BF_THEME_COUNT, BF_THEME_TEXT, BFClampInt(BF_Settings.theme, 0, BF_THEME_COUNT - 1));
    } else if (state->openDrop == BF_SETTINGS_DROP_LANGUAGE) {
        BFDrawDropList(dc, state->languageBox, state->fonts.small, BF_LANG_COUNT, BF_LANGUAGE_TEXT, BFClampInt(BF_Settings.language, 0, BF_LANG_COUNT - 1));
    }
}

static void BFHandleSettingsClick(HWND hwnd, BFSettingsState *state, int x, int y)
{
    int item;

    if (state->openDrop == BF_SETTINGS_DROP_THEME) {
        item = BFHitDropItem(state->themeBox, BF_THEME_COUNT, x, y);
        if (item >= 0) {
            BF_Settings.theme = item;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSaveState();
            BFInvalidateAllWindows();
            return;
        }
    } else if (state->openDrop == BF_SETTINGS_DROP_LANGUAGE) {
        item = BFHitDropItem(state->languageBox, BF_LANG_COUNT, x, y);
        if (item >= 0) {
            BF_Settings.language = item;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSaveState();
            BFInvalidateAllWindows();
            return;
        }
    }

    if (BFPointInRect(&state->themeBox, x, y)) {
        state->openDrop = state->openDrop == BF_SETTINGS_DROP_THEME ? BF_SETTINGS_DROP_NONE : BF_SETTINGS_DROP_THEME;
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    if (BFPointInRect(&state->languageBox, x, y)) {
        state->openDrop = state->openDrop == BF_SETTINGS_DROP_LANGUAGE ? BF_SETTINGS_DROP_NONE : BF_SETTINGS_DROP_LANGUAGE;
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    if (BFPointInRect(&state->soundBox, x, y)) {
        BF_Settings.sound = !BF_Settings.sound;
        state->openDrop = BF_SETTINGS_DROP_NONE;
        BFSaveState();
        BFInvalidateAllWindows();
        return;
    }
    if (!IsRectEmpty(&state->volumeTrack) && BFPointInRect(&state->volumeTrack, x, y)) {
        state->openDrop = BF_SETTINGS_DROP_NONE;
        BFSetVolumeFromTrack(state->volumeTrack, x);
        BF_VolumeCaptureWindow = hwnd;
        SetCapture(hwnd);
        return;
    }

    state->openDrop = BF_SETTINGS_DROP_NONE;
    InvalidateRect(hwnd, NULL, FALSE);
}

int BFRegisterSettingsWindow(HINSTANCE instance)
{
    WNDCLASSEXW cls;
    ZeroMemory(&cls, sizeof(cls));
    cls.cbSize = sizeof(cls);
    cls.lpfnWndProc = BFSettingsWindowProc;
    cls.hInstance = instance;
    cls.hCursor = LoadCursorW(NULL, IDC_ARROW);
    cls.lpszClassName = BF_SETTINGS_CLASS;
    return RegisterClassExW(&cls) != 0;
}

void BFOpenSettingsWindow(HWND parent)
{
    HWND owner = parent != NULL ? parent : BF_MainWindow;
    if (BF_SettingsWindow != NULL && IsWindow(BF_SettingsWindow)) {
        ShowWindow(BF_SettingsWindow, SW_SHOWNORMAL);
        SetForegroundWindow(BF_SettingsWindow);
        return;
    }
    BF_SettingsWindow = CreateWindowExW(WS_EX_APPWINDOW, BF_SETTINGS_CLASS, BFT(BF_TX_SETTINGS), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 560, 430, owner, NULL, BF_Instance, NULL);
    if (BF_SettingsWindow != NULL) {
        ShowWindow(BF_SettingsWindow, SW_SHOWNORMAL);
        UpdateWindow(BF_SettingsWindow);
    }
}

void BFToggleSettingsWindow(HWND parent)
{
    if (BF_SettingsWindow != NULL && IsWindow(BF_SettingsWindow)) {
        DestroyWindow(BF_SettingsWindow);
        return;
    }
    BFOpenSettingsWindow(parent);
}

static LRESULT CALLBACK BFSettingsWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BFSettingsState *state = (BFSettingsState *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (message) {
    case WM_CREATE:
        state = (BFSettingsState *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*state));
        if (state == NULL) {
            return -1;
        }
        BFCreateFonts(&state->fonts, 20);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        BFLayoutSettings(state);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_SIZE:
        if (state != NULL) {
            BFLayoutSettings(state);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;

    case WM_LBUTTONDOWN:
        if (state != NULL) {
            BFHandleSettingsClick(hwnd, state, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        return 0;

    case WM_MOUSEMOVE:
        if (state != NULL && BF_VolumeCaptureWindow == hwnd && (wParam & MK_LBUTTON) != 0 && !IsRectEmpty(&state->volumeTrack)) {
            BFSetVolumeFromTrack(state->volumeTrack, GET_X_LPARAM(lParam));
            return 0;
        }
        break;

    case WM_LBUTTONUP:
        if (BF_VolumeCaptureWindow == hwnd) {
            BF_VolumeCaptureWindow = NULL;
            ReleaseCapture();
            return 0;
        }
        break;

    case WM_PAINT:
        if (state != NULL) {
            BFPaintBuffer paint;
            if (BFBeginBufferedPaint(hwnd, &paint)) {
                BFDrawSettings(hwnd, paint.memoryDc, &paint.client, state);
                BFEndBufferedPaint(hwnd, &paint);
                return 0;
            }
        }
        break;

    case WM_CLOSE:
        BFSaveState();
        DestroyWindow(hwnd);
        return 0;

    case WM_NCDESTROY:
        if (state != NULL) {
            BFDestroyFonts(&state->fonts);
            HeapFree(GetProcessHeap(), 0, state);
        }
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        BF_SettingsWindow = NULL;
        BFInvalidateAllWindows();
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
