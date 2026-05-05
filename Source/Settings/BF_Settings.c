#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "BF_Settings.h"
#include "BF_Resource.h"

typedef enum BFSettingsDrop {
    BF_SETTINGS_DROP_NONE,
    BF_SETTINGS_DROP_THEME,
    BF_SETTINGS_DROP_LANGUAGE,
    BF_SETTINGS_DROP_EFFECT
} BFSettingsDrop;

typedef enum BFSettingsPanel {
    BF_SETTINGS_PANEL_SOUND,
    BF_SETTINGS_PANEL_SCREEN,
    BF_SETTINGS_PANEL_LANGUAGE,
    BF_SETTINGS_PANEL_COUNT
} BFSettingsPanel;

typedef enum BFSettingsCapture {
    BF_SETTINGS_CAPTURE_NONE,
    BF_SETTINGS_CAPTURE_VOLUME,
    BF_SETTINGS_CAPTURE_SPEED,
    BF_SETTINGS_CAPTURE_DURATION,
    BF_SETTINGS_CAPTURE_OPACITY
} BFSettingsCapture;

typedef struct BFSettingsState {
    BFFonts fonts;
    BFSettingsPanel panel;
    BFSettingsDrop openDrop;
    BFSettingsCapture capture;
    RECT panelButtons[BF_SETTINGS_PANEL_COUNT];
    RECT contentRect;
    RECT themeBox;
    RECT effectBox;
    RECT languageBox;
    RECT soundBox;
    RECT volumeTrack;
    RECT clickEffectBox;
    RECT speedTrack;
    RECT durationTrack;
    RECT opacityTrack;
    BFClickEffects effects;
} BFSettingsState;

static LRESULT CALLBACK BFSettingsWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static const BFTextKey BF_PANEL_TEXT[BF_SETTINGS_PANEL_COUNT] = {
    BF_TX_SOUND,
    BF_TX_SCREEN,
    BF_TX_LANGUAGE
};

static const BFTextKey BF_THEME_TEXT[BF_THEME_COUNT] = {
    BF_TX_THEME_TERMINAL,
    BF_TX_THEME_DARK,
    BF_TX_THEME_SPACE,
    BF_TX_THEME_BOOK,
    BF_TX_THEME_LIGHT,
    BF_TX_THEME_DEEP,
    BF_TX_THEME_BADUK,
    BF_TX_THEME_CHESS,
    BF_TX_THEME_WHITE
};

static const BFTextKey BF_EFFECT_TEXT[BF_EFFECT_COUNT] = {
    BF_TX_EFFECT_FIRE,
    BF_TX_EFFECT_WATER,
    BF_TX_EFFECT_SPACE,
    BF_TX_EFFECT_PIXEL,
    BF_TX_EFFECT_BADUK,
    BF_TX_EFFECT_CHESS,
    BF_TX_EFFECT_NOTE
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
    list.bottom = list.top + count * 30;
    BFDrawBox(dc, list, palette->panel, palette->border, 1);

    for (i = 0; i < count; ++i) {
        row.left = list.left + 1;
        row.top = list.top + i * 30;
        row.right = list.right - 1;
        row.bottom = row.top + 30;
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
    list.bottom = list.top + count * 30;
    if (y < list.top || y >= list.bottom) {
        return -1;
    }
    return (y - list.top) / 30;
}

static void BFDrawToggle(HDC dc, RECT rect, const wchar_t *text, HFONT font, int enabled)
{
    const BFPalette *palette = BFP();
    RECT mark = rect;
    RECT label = rect;

    mark.right = mark.left + 30;
    BFDrawBox(dc, mark, enabled ? palette->selected : palette->panel, enabled ? palette->accent : palette->border, enabled ? 2 : 1);
    BFDrawTextBlock(dc, enabled ? L"✓" : L"", mark, font, palette->accent, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    label.left = mark.right + 12;
    BFDrawTextBlock(dc, text, label, font, palette->text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

static void BFLayoutSettings(BFSettingsState *state, const RECT *client)
{
    int sidebarLeft = BF_MARGIN;
    int sidebarRight = sidebarLeft + 136;
    int contentLeft = sidebarRight + 24;
    int contentRight = BFMaxInt(contentLeft + 300, client->right - BF_MARGIN);
    int y = 92;
    int i;

    for (i = 0; i < BF_SETTINGS_PANEL_COUNT; ++i) {
        state->panelButtons[i].left = sidebarLeft;
        state->panelButtons[i].top = y + i * 48;
        state->panelButtons[i].right = sidebarRight;
        state->panelButtons[i].bottom = state->panelButtons[i].top + 38;
    }

    state->contentRect.left = contentLeft;
    state->contentRect.top = 92;
    state->contentRect.right = contentRight;
    state->contentRect.bottom = client->bottom - BF_MARGIN;

    state->themeBox.left = contentLeft + 18;
    state->themeBox.top = 144;
    state->themeBox.right = contentRight - 18;
    state->themeBox.bottom = state->themeBox.top + 34;

    state->effectBox.left = contentLeft + 18;
    state->effectBox.top = 224;
    state->effectBox.right = contentRight - 18;
    state->effectBox.bottom = state->effectBox.top + 34;

    state->clickEffectBox.left = contentLeft + 18;
    state->clickEffectBox.top = 306;
    state->clickEffectBox.right = contentRight - 18;
    state->clickEffectBox.bottom = state->clickEffectBox.top + 34;

    state->speedTrack.left = contentLeft + 128;
    state->speedTrack.top = 384;
    state->speedTrack.right = contentRight - 18;
    state->speedTrack.bottom = state->speedTrack.top + 10;

    state->durationTrack.left = contentLeft + 128;
    state->durationTrack.top = 438;
    state->durationTrack.right = contentRight - 18;
    state->durationTrack.bottom = state->durationTrack.top + 10;

    state->opacityTrack.left = contentLeft + 128;
    state->opacityTrack.top = 492;
    state->opacityTrack.right = contentRight - 18;
    state->opacityTrack.bottom = state->opacityTrack.top + 10;

    state->soundBox.left = contentLeft + 18;
    state->soundBox.top = 144;
    state->soundBox.right = contentRight - 18;
    state->soundBox.bottom = state->soundBox.top + 34;

    state->languageBox.left = contentLeft + 18;
    state->languageBox.top = 144;
    state->languageBox.right = contentRight - 18;
    state->languageBox.bottom = state->languageBox.top + 34;
}

static void BFDrawPanelLabel(HDC dc, RECT content, int top, const wchar_t *text, HFONT font)
{
    const BFPalette *palette = BFP();
    RECT label;

    label.left = content.left + 18;
    label.top = top;
    label.right = content.right - 18;
    label.bottom = top + 28;
    BFDrawTextBlock(dc, text, label, font, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

static void BFDrawValueSlider(HDC dc, RECT content, RECT track, const wchar_t *labelText, int value, int minimum, int maximum, HFONT font)
{
    const BFPalette *palette = BFP();
    RECT label;
    RECT valueBox;
    RECT fill;
    RECT knob;
    wchar_t buffer[48];
    int width = BFMaxInt(1, track.right - track.left);
    int knobX = track.left + (BFClampInt(value, minimum, maximum) - minimum) * width / BFMaxInt(1, maximum - minimum);

    label.left = content.left + 18;
    label.top = track.top - 12;
    label.right = track.left - 12;
    label.bottom = track.bottom + 12;
    BFDrawTextBlock(dc, labelText, label, font, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    BFDrawBox(dc, track, palette->panelAlt, palette->border, 1);
    fill = track;
    fill.right = knobX;
    BFFillRectColor(dc, &fill, palette->accent);

    knob.left = knobX - 5;
    knob.right = knobX + 5;
    knob.top = track.top - 6;
    knob.bottom = track.bottom + 6;
    BFDrawBox(dc, knob, palette->accent, palette->text, 1);

    valueBox.left = track.right - 58;
    valueBox.top = track.top - 24;
    valueBox.right = track.right;
    valueBox.bottom = track.top - 4;
    swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"%d", value);
    BFDrawTextBlock(dc, buffer, valueBox, font, palette->text, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

static int BFValueFromTrack(RECT track, int x, int minimum, int maximum)
{
    int width = BFMaxInt(1, track.right - track.left);
    int value = minimum + ((x - track.left) * (maximum - minimum) + width / 2) / width;
    return BFClampInt(value, minimum, maximum);
}

static int BFPointInSlider(RECT track, int x, int y)
{
    RECT hit = track;
    InflateRect(&hit, 12, 14);
    return BFPointInRect(&hit, x, y);
}

static void BFSetEffectSliderValue(HWND hwnd, BFSettingsState *state, int x)
{
    int changed = 0;

    if (state->capture == BF_SETTINGS_CAPTURE_SPEED) {
        int value = BFValueFromTrack(state->speedTrack, x, 1, 10);
        changed = BF_Settings.clickEffectSpeed != value;
        BF_Settings.clickEffectSpeed = value;
    } else if (state->capture == BF_SETTINGS_CAPTURE_DURATION) {
        int value = BFValueFromTrack(state->durationTrack, x, 10, 90);
        changed = BF_Settings.clickEffectDuration != value;
        BF_Settings.clickEffectDuration = value;
    } else if (state->capture == BF_SETTINGS_CAPTURE_OPACITY) {
        int value = BFValueFromTrack(state->opacityTrack, x, 10, 100);
        changed = BF_Settings.clickEffectOpacity != value;
        BF_Settings.clickEffectOpacity = value;
    }

    if (changed) {
        BFSaveState();
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

static void BFDrawSettings(HWND hwnd, HDC dc, const RECT *client, BFSettingsState *state)
{
    const BFPalette *palette = BFP();
    RECT rect;
    RECT volumeBounds;
    int i;

    (void)hwnd;
    BFLayoutSettings(state, client);
    SetRectEmpty(&state->volumeTrack);
    BFDrawGrid(dc, client);

    rect.left = BF_MARGIN;
    rect.top = 18;
    rect.right = client->right - BF_MARGIN;
    rect.bottom = 58;
    BFDrawTextBlock(dc, BFT(BF_TX_SETTINGS), rect, state->fonts.title, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    for (i = 0; i < BF_SETTINGS_PANEL_COUNT; ++i) {
        BFDrawButton(dc, state->panelButtons[i], BFT(BF_PANEL_TEXT[i]), state->fonts.small, state->panel == (BFSettingsPanel)i);
    }

    BFDrawBox(dc, state->contentRect, palette->panel, palette->border, 1);

    if (state->panel == BF_SETTINGS_PANEL_SOUND) {
        BFDrawPanelLabel(dc, state->contentRect, 112, BFT(BF_TX_SOUND), state->fonts.ui);
        BFDrawToggle(dc, state->soundBox, BFT(BF_TX_SOUND), state->fonts.small, BF_Settings.sound);

        BFDrawPanelLabel(dc, state->contentRect, 206, BFT(BF_TX_VOLUME), state->fonts.ui);
        volumeBounds.left = state->contentRect.left + 18;
        volumeBounds.top = 242;
        volumeBounds.right = state->contentRect.right - 18;
        volumeBounds.bottom = 278;
        BFDrawVolume(dc, volumeBounds, state->fonts.small, &state->volumeTrack);
    } else if (state->panel == BF_SETTINGS_PANEL_SCREEN) {
        BFDrawPanelLabel(dc, state->contentRect, 112, BFT(BF_TX_THEME), state->fonts.ui);
        BFDrawControlBox(dc, state->themeBox, BFT(BF_THEME_TEXT[BFClampInt(BF_Settings.theme, 0, BF_THEME_COUNT - 1)]), state->fonts.small, state->openDrop == BF_SETTINGS_DROP_THEME);

        BFDrawPanelLabel(dc, state->contentRect, 192, BFT(BF_TX_EFFECT_TYPE), state->fonts.ui);
        BFDrawControlBox(dc, state->effectBox, BFT(BF_EFFECT_TEXT[BFClampInt(BF_Settings.clickEffectStyle, 0, BF_EFFECT_COUNT - 1)]), state->fonts.small, state->openDrop == BF_SETTINGS_DROP_EFFECT);

        BFDrawPanelLabel(dc, state->contentRect, 274, BFT(BF_TX_CLICK_EFFECT), state->fonts.ui);
        BFDrawToggle(dc, state->clickEffectBox, BFT(BF_TX_CLICK_EFFECT), state->fonts.small, BF_Settings.clickEffect);

        BFDrawValueSlider(dc, state->contentRect, state->speedTrack, BFT(BF_TX_EFFECT_SPEED), BF_Settings.clickEffectSpeed, 1, 10, state->fonts.small);
        BFDrawValueSlider(dc, state->contentRect, state->durationTrack, BFT(BF_TX_EFFECT_DURATION), BF_Settings.clickEffectDuration, 10, 90, state->fonts.small);
        BFDrawValueSlider(dc, state->contentRect, state->opacityTrack, BFT(BF_TX_EFFECT_OPACITY), BF_Settings.clickEffectOpacity, 10, 100, state->fonts.small);

        if (state->openDrop == BF_SETTINGS_DROP_THEME) {
            BFDrawDropList(dc, state->themeBox, state->fonts.small, BF_THEME_COUNT, BF_THEME_TEXT, BFClampInt(BF_Settings.theme, 0, BF_THEME_COUNT - 1));
        } else if (state->openDrop == BF_SETTINGS_DROP_EFFECT) {
            BFDrawDropList(dc, state->effectBox, state->fonts.small, BF_EFFECT_COUNT, BF_EFFECT_TEXT, BFClampInt(BF_Settings.clickEffectStyle, 0, BF_EFFECT_COUNT - 1));
        }
    } else {
        BFDrawPanelLabel(dc, state->contentRect, 112, BFT(BF_TX_LANGUAGE), state->fonts.ui);
        BFDrawControlBox(dc, state->languageBox, BFT(BF_LANGUAGE_TEXT[BFClampInt(BF_Settings.language, 0, BF_LANG_COUNT - 1)]), state->fonts.small, state->openDrop == BF_SETTINGS_DROP_LANGUAGE);

        if (state->openDrop == BF_SETTINGS_DROP_LANGUAGE) {
            BFDrawDropList(dc, state->languageBox, state->fonts.small, BF_LANG_COUNT, BF_LANGUAGE_TEXT, BFClampInt(BF_Settings.language, 0, BF_LANG_COUNT - 1));
        }
    }

    BFDrawClickEffects(dc, &state->effects);
}

static int BFApplyDropClick(BFSettingsState *state, int x, int y)
{
    int item;

    if (state->openDrop == BF_SETTINGS_DROP_THEME) {
        item = BFHitDropItem(state->themeBox, BF_THEME_COUNT, x, y);
        if (item >= 0) {
            BF_Settings.theme = item;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSaveState();
            BFInvalidateAllWindows();
            return 1;
        }
    } else if (state->openDrop == BF_SETTINGS_DROP_LANGUAGE) {
        item = BFHitDropItem(state->languageBox, BF_LANG_COUNT, x, y);
        if (item >= 0) {
            BF_Settings.language = item;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSaveState();
            BFInvalidateAllWindows();
            return 1;
        }
    } else if (state->openDrop == BF_SETTINGS_DROP_EFFECT) {
        item = BFHitDropItem(state->effectBox, BF_EFFECT_COUNT, x, y);
        if (item >= 0) {
            BF_Settings.clickEffectStyle = item;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSaveState();
            BFInvalidateAllWindows();
            return 1;
        }
    }

    return 0;
}

static void BFHandleSettingsClick(HWND hwnd, BFSettingsState *state, int x, int y)
{
    int i;

    if (BFApplyDropClick(state, x, y)) {
        return;
    }

    for (i = 0; i < BF_SETTINGS_PANEL_COUNT; ++i) {
        if (BFPointInRect(&state->panelButtons[i], x, y)) {
            state->panel = (BFSettingsPanel)i;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
    }

    if (state->panel == BF_SETTINGS_PANEL_SOUND) {
        if (BFPointInRect(&state->soundBox, x, y)) {
            BF_Settings.sound = !BF_Settings.sound;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSaveState();
            BFInvalidateAllWindows();
            return;
        }
        if (!IsRectEmpty(&state->volumeTrack) && BFPointInRect(&state->volumeTrack, x, y)) {
            state->openDrop = BF_SETTINGS_DROP_NONE;
            state->capture = BF_SETTINGS_CAPTURE_VOLUME;
            BFSetVolumeFromTrack(state->volumeTrack, x);
            BF_VolumeCaptureWindow = hwnd;
            SetCapture(hwnd);
            return;
        }
    } else if (state->panel == BF_SETTINGS_PANEL_SCREEN) {
        if (BFPointInRect(&state->themeBox, x, y)) {
            state->openDrop = state->openDrop == BF_SETTINGS_DROP_THEME ? BF_SETTINGS_DROP_NONE : BF_SETTINGS_DROP_THEME;
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
        if (BFPointInRect(&state->effectBox, x, y)) {
            state->openDrop = state->openDrop == BF_SETTINGS_DROP_EFFECT ? BF_SETTINGS_DROP_NONE : BF_SETTINGS_DROP_EFFECT;
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
        if (BFPointInRect(&state->clickEffectBox, x, y)) {
            BF_Settings.clickEffect = !BF_Settings.clickEffect;
            if (!BF_Settings.clickEffect) {
                BFClearClickEffects(&state->effects);
            }
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSaveState();
            BFInvalidateAllWindows();
            return;
        }
        if (BFPointInSlider(state->speedTrack, x, y)) {
            state->capture = BF_SETTINGS_CAPTURE_SPEED;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSetEffectSliderValue(hwnd, state, x);
            SetCapture(hwnd);
            return;
        }
        if (BFPointInSlider(state->durationTrack, x, y)) {
            state->capture = BF_SETTINGS_CAPTURE_DURATION;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSetEffectSliderValue(hwnd, state, x);
            SetCapture(hwnd);
            return;
        }
        if (BFPointInSlider(state->opacityTrack, x, y)) {
            state->capture = BF_SETTINGS_CAPTURE_OPACITY;
            state->openDrop = BF_SETTINGS_DROP_NONE;
            BFSetEffectSliderValue(hwnd, state, x);
            SetCapture(hwnd);
            return;
        }
    } else {
        if (BFPointInRect(&state->languageBox, x, y)) {
            state->openDrop = state->openDrop == BF_SETTINGS_DROP_LANGUAGE ? BF_SETTINGS_DROP_NONE : BF_SETTINGS_DROP_LANGUAGE;
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
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
    cls.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(BF_ICON_APP));
    cls.hIconSm = LoadIconW(instance, MAKEINTRESOURCEW(BF_ICON_APP));
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
    BF_SettingsWindow = CreateWindowExW(WS_EX_APPWINDOW, BF_SETTINGS_CLASS, BFT(BF_TX_SETTINGS), WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 760, 620, owner, NULL, BF_Instance, NULL);
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
        state->panel = BF_SETTINGS_PANEL_SOUND;
        BFCreateFonts(&state->fonts, 20);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        SetTimer(hwnd, BF_TIMER_ANIMATION, BF_EFFECT_TIMER_MS, NULL);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lParam)->ptMinTrackSize.x = 700;
        ((MINMAXINFO *)lParam)->ptMinTrackSize.y = 560;
        return 0;

    case WM_SIZE:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_LBUTTONDOWN:
        if (state != NULL) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            BFHandleSettingsClick(hwnd, state, x, y);
            if (state->capture == BF_SETTINGS_CAPTURE_NONE) {
                BFAddClickEffect(&state->effects, x, y);
                BFRedrawNow(hwnd);
            }
        }
        return 0;

    case WM_MOUSEMOVE:
        if (state != NULL && BF_VolumeCaptureWindow == hwnd && (wParam & MK_LBUTTON) != 0 && !IsRectEmpty(&state->volumeTrack)) {
            BFSetVolumeFromTrack(state->volumeTrack, GET_X_LPARAM(lParam));
            return 0;
        }
        if (state != NULL && state->capture != BF_SETTINGS_CAPTURE_NONE && (wParam & MK_LBUTTON) != 0) {
            BFSetEffectSliderValue(hwnd, state, GET_X_LPARAM(lParam));
            return 0;
        }
        break;

    case WM_LBUTTONUP:
        if (state != NULL && state->capture != BF_SETTINGS_CAPTURE_NONE) {
            state->capture = BF_SETTINGS_CAPTURE_NONE;
            BF_VolumeCaptureWindow = NULL;
            ReleaseCapture();
            return 0;
        }
        break;

    case WM_TIMER:
        if (state != NULL && wParam == BF_TIMER_ANIMATION) {
            BFStepClickEffects(&state->effects);
            BFHandlePreviewTimer(hwnd, wParam);
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
