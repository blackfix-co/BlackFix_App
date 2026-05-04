#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include "BF_App.h"
#include "BF_BlackFix.h"
#include "BF_Album.h"
#include "BF_Member.h"
#include "BF_Settings.h"
#include "BF_Update.h"

typedef struct BFStarEntry {
    wchar_t id[96];
    unsigned int order;
} BFStarEntry;

HINSTANCE BF_Instance;
HWND BF_MainWindow;
HWND BF_CheonWindow;
HWND BF_CioWindow;
HWND BF_SettingsWindow;
HWND BF_AlbumWindows[BF_MAX_ALBUMS];
BFAppSettings BF_Settings = {BF_THEME_TERMINAL, 1, BF_LANG_KO, 80};
int BF_AnimTick;
HWND BF_VolumeCaptureWindow;

static HANDLE BF_SingletonMutex;
static BFStarEntry BF_Stars[BF_MAX_STARS];
static size_t BF_StarCount;
static unsigned int BF_NextStarOrder = 1;
static int BF_PreviewOpen;
static HWND BF_PreviewOwner;
static wchar_t BF_PreviewId[96];

static void BFApplyPreviewVolume(void);

static const BFPalette BF_Palettes[BF_THEME_COUNT] = {
    {RGB(2, 6, 4), RGB(6, 14, 9), RGB(9, 25, 14), RGB(10, 35, 16), RGB(198, 255, 207), RGB(118, 184, 126), RGB(42, 255, 121), RGB(18, 68, 30), RGB(13, 69, 31), RGB(21, 93, 42)},
    {RGB(9, 10, 12), RGB(17, 19, 22), RGB(25, 28, 32), RGB(31, 34, 39), RGB(230, 235, 231), RGB(145, 154, 148), RGB(118, 255, 153), RGB(58, 67, 61), RGB(33, 58, 43), RGB(45, 70, 54)},
    {RGB(1, 3, 14), RGB(7, 12, 31), RGB(12, 21, 48), RGB(19, 31, 70), RGB(221, 241, 255), RGB(140, 177, 214), RGB(88, 221, 255), RGB(42, 76, 127), RGB(22, 67, 104), RGB(33, 88, 128)},
    {RGB(236, 228, 210), RGB(248, 241, 223), RGB(232, 218, 194), RGB(215, 199, 173), RGB(44, 35, 27), RGB(103, 82, 62), RGB(36, 128, 76), RGB(154, 129, 93), RGB(203, 228, 199), RGB(190, 214, 184)},
    {RGB(233, 242, 231), RGB(247, 255, 246), RGB(224, 241, 222), RGB(205, 225, 204), RGB(18, 61, 33), RGB(69, 113, 78), RGB(0, 132, 55), RGB(112, 170, 123), RGB(193, 234, 202), RGB(180, 223, 190)},
    {RGB(1, 5, 2), RGB(3, 17, 7), RGB(5, 28, 11), RGB(7, 43, 14), RGB(183, 255, 195), RGB(92, 176, 105), RGB(76, 255, 94), RGB(24, 92, 35), RGB(16, 86, 28), RGB(22, 112, 41)}
};

static const wchar_t *BF_Text[BF_LANG_COUNT][BF_TX_COUNT] = {
    {
        L"앨범", L"노래", L"굿즈", L"천", L"씨오", L"PIXEL CATALOG", L"앨범 목록", L"전체 노래", L"굿즈",
        L"최신순", L"오래된순", L"별표 순", L"별표", L"영상 목록", L"쇼츠 목록",
        L"1분 미리보기", L"이동", L"소리", L"설정", L"테마", L"소리", L"언어",
        L"표시할 항목이 없습니다.", L"터미널", L"다크", L"우주", L"책", L"밝은 픽셀", L"딥 그린",
        L"한국어", L"영어", L"일본어",
        L"미리보기 파일이 없습니다. media 폴더에 파일을 넣거나 BF_Content.c의 previewSource를 수정하세요.",
        L"미리보기를 재생할 수 없습니다.",
        L"1분 미리보기 재생 중"
    },
    {
        L"Albums", L"Songs", L"Goods", L"Cheon", L"Cio", L"PIXEL CATALOG", L"Albums", L"All Songs", L"Goods",
        L"Newest", L"Oldest", L"Star Order", L"Stars", L"Videos", L"Shorts",
        L"1 min preview", L"Open", L"Sound", L"Settings", L"Theme", L"Sound", L"Language",
        L"No items to show.", L"Terminal", L"Dark", L"Space", L"Book", L"Light Pixel", L"Deep Green",
        L"Korean", L"English", L"Japanese",
        L"Preview file is missing. Put the file in the media folder or edit previewSource in BF_Content.c.",
        L"Could not play the preview.",
        L"Playing 1 minute preview"
    },
    {
        L"アルバム", L"曲", L"グッズ", L"チョン", L"シオ", L"PIXEL CATALOG", L"アルバム", L"全曲", L"グッズ",
        L"新しい順", L"古い順", L"星順", L"星", L"動画リスト", L"ショート",
        L"1分プレビュー", L"移動", L"音量", L"設定", L"テーマ", L"音", L"言語",
        L"表示する項目がありません。", L"ターミナル", L"ダーク", L"宇宙", L"本", L"ライトピクセル", L"ディープグリーン",
        L"韓国語", L"英語", L"日本語",
        L"プレビューファイルがありません。mediaフォルダーに置くか、BF_Content.cのpreviewSourceを修正してください。",
        L"プレビューを再生できません。",
        L"1分プレビュー再生中"
    }
};

const wchar_t *BFT(BFTextKey key)
{
    int language = BFClampInt(BF_Settings.language, 0, BF_LANG_COUNT - 1);
    return BF_Text[language][key];
}

const BFPalette *BFP(void)
{
    int theme = BFClampInt(BF_Settings.theme, 0, BF_THEME_COUNT - 1);
    return &BF_Palettes[theme];
}

int BFClampInt(int value, int minimum, int maximum)
{
    if (value < minimum) {
        return minimum;
    }
    if (value > maximum) {
        return maximum;
    }
    return value;
}

int BFMaxInt(int a, int b)
{
    return a > b ? a : b;
}

int BFMinInt(int a, int b)
{
    return a < b ? a : b;
}

int BFPointInRect(const RECT *rect, int x, int y)
{
    return x >= rect->left && x < rect->right && y >= rect->top && y < rect->bottom;
}

int BFHasText(const wchar_t *text)
{
    return text != NULL && text[0] != L'\0';
}

void BFCopyString(wchar_t *target, size_t capacity, const wchar_t *source)
{
    if (capacity == 0) {
        return;
    }
    if (source == NULL) {
        target[0] = L'\0';
        return;
    }
    wcsncpy(target, source, capacity - 1);
    target[capacity - 1] = L'\0';
}

void BFAppendString(wchar_t *target, size_t capacity, const wchar_t *source)
{
    size_t length;
    size_t remaining;

    if (capacity == 0 || source == NULL) {
        return;
    }
    length = wcslen(target);
    if (length >= capacity - 1) {
        return;
    }
    remaining = capacity - length - 1;
    wcsncat(target, source, remaining);
    target[capacity - 1] = L'\0';
}

static HFONT BFCreateFont(int pointSize, int weight)
{
    HDC dc = GetDC(NULL);
    int dpi = dc != NULL ? GetDeviceCaps(dc, LOGPIXELSY) : 96;
    int height = -MulDiv(pointSize, dpi, 72);

    if (dc != NULL) {
        ReleaseDC(NULL, dc);
    }

    return CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"D2Coding");
}

void BFCreateFonts(BFFonts *fonts, int titlePoint)
{
    fonts->title = BFCreateFont(titlePoint, FW_BOLD);
    fonts->ui = BFCreateFont(13, FW_BOLD);
    fonts->small = BFCreateFont(10, FW_NORMAL);
}

void BFDestroyFonts(BFFonts *fonts)
{
    if (fonts == NULL) {
        return;
    }
    DeleteObject(fonts->title);
    DeleteObject(fonts->ui);
    DeleteObject(fonts->small);
}

int BFBeginBufferedPaint(HWND hwnd, BFPaintBuffer *buffer)
{
    ZeroMemory(buffer, sizeof(*buffer));
    buffer->windowDc = BeginPaint(hwnd, &buffer->ps);
    GetClientRect(hwnd, &buffer->client);
    buffer->memoryDc = CreateCompatibleDC(buffer->windowDc);
    buffer->bitmap = CreateCompatibleBitmap(buffer->windowDc, BFMaxInt(1, buffer->client.right), BFMaxInt(1, buffer->client.bottom));
    if (buffer->memoryDc == NULL || buffer->bitmap == NULL) {
        return 0;
    }
    buffer->oldBitmap = SelectObject(buffer->memoryDc, buffer->bitmap);
    return 1;
}

void BFEndBufferedPaint(HWND hwnd, BFPaintBuffer *buffer)
{
    BitBlt(buffer->windowDc, 0, 0, buffer->client.right, buffer->client.bottom, buffer->memoryDc, 0, 0, SRCCOPY);
    SelectObject(buffer->memoryDc, buffer->oldBitmap);
    DeleteObject(buffer->bitmap);
    DeleteDC(buffer->memoryDc);
    EndPaint(hwnd, &buffer->ps);
}

void BFFillRectColor(HDC dc, const RECT *rect, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, rect, brush);
    DeleteObject(brush);
}

void BFDrawLine(HDC dc, int x1, int y1, int x2, int y2, COLORREF color, int width)
{
    HPEN pen = CreatePen(PS_SOLID, width, color);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    MoveToEx(dc, x1, y1, NULL);
    LineTo(dc, x2, y2);
    SelectObject(dc, oldPen);
    DeleteObject(pen);
}

void BFDrawBox(HDC dc, RECT rect, COLORREF fill, COLORREF border, int width)
{
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, width, border);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

void BFDrawTextBlock(HDC dc, const wchar_t *text, RECT rect, HFONT font, COLORREF color, UINT format)
{
    HGDIOBJ oldFont = SelectObject(dc, font != NULL ? font : GetStockObject(DEFAULT_GUI_FONT));
    int oldMode = SetBkMode(dc, TRANSPARENT);
    COLORREF oldColor = SetTextColor(dc, color);
    DrawTextW(dc, text != NULL ? text : L"", -1, &rect, format | DT_NOPREFIX);
    SetTextColor(dc, oldColor);
    SetBkMode(dc, oldMode);
    SelectObject(dc, oldFont);
}

void BFDrawGrid(HDC dc, const RECT *client)
{
    const BFPalette *palette = BFP();
    int x;
    int y;
    int offset = BF_AnimTick % 16;
    int scanY = client->top + (BF_AnimTick * 2) % BFMaxInt(1, client->bottom - client->top);
    HPEN pen = CreatePen(PS_SOLID, 1, palette->grid);
    HGDIOBJ oldPen;

    BFFillRectColor(dc, client, palette->bg);
    oldPen = SelectObject(dc, pen);
    for (x = client->left - offset; x < client->right; x += 16) {
        MoveToEx(dc, x, client->top, NULL);
        LineTo(dc, x, client->bottom);
    }
    for (y = client->top + offset; y < client->bottom; y += 16) {
        MoveToEx(dc, client->left, y, NULL);
        LineTo(dc, client->right, y);
    }
    SelectObject(dc, oldPen);
    DeleteObject(pen);
    BFDrawLine(dc, client->left, scanY, client->right, scanY, palette->selected, 1);
}

void BFDrawHeader(HDC dc, const RECT *client, const BFFonts *fonts, const wchar_t *title, const wchar_t *subtitle)
{
    const BFPalette *palette = BFP();
    RECT band = *client;
    RECT titleRect;
    RECT subRect;

    band.bottom = BF_HEADER_HEIGHT;
    BFFillRectColor(dc, &band, palette->panel);
    BFDrawLine(dc, client->left, BF_HEADER_HEIGHT - 1, client->right, BF_HEADER_HEIGHT - 1, palette->border, 1);

    titleRect.left = BF_MARGIN;
    titleRect.top = 14;
    titleRect.right = client->right - BF_MARGIN;
    titleRect.bottom = 54;
    BFDrawTextBlock(dc, title, titleRect, fonts->title, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    subRect.left = BF_MARGIN + 2;
    subRect.top = 56;
    subRect.right = client->right - BF_MARGIN;
    subRect.bottom = 80;
    BFDrawTextBlock(dc, subtitle, subRect, fonts->small, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void BFDrawButton(HDC dc, RECT rect, const wchar_t *text, HFONT font, int selected)
{
    const BFPalette *palette = BFP();
    RECT textRect = rect;
    BFDrawBox(dc, rect, selected ? palette->selected : palette->panel, selected ? palette->accent : palette->border, selected ? 2 : 1);
    InflateRect(&textRect, -8, -2);
    BFDrawTextBlock(dc, text, textRect, font, selected ? palette->accent : palette->text, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void BFDrawVolume(HDC dc, RECT bounds, HFONT font, RECT *trackOut)
{
    const BFPalette *palette = BFP();
    RECT label = bounds;
    RECT track = bounds;
    RECT fill;
    RECT knob;
    wchar_t buffer[64];
    int width;
    int knobX;

    swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"%ls %d", BFT(BF_TX_VOLUME), BF_Settings.sound ? BF_Settings.volume : 0);
    label.right = bounds.left + 86;
    BFDrawTextBlock(dc, buffer, label, font, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    track.left = label.right + 10;
    track.right = bounds.right;
    track.top = bounds.top + 12;
    track.bottom = bounds.bottom - 12;
    if (track.right <= track.left) {
        SetRectEmpty(trackOut);
        return;
    }

    BFDrawBox(dc, track, palette->panelAlt, palette->border, 1);
    width = BFMaxInt(1, track.right - track.left);
    knobX = track.left + ((BF_Settings.sound ? BF_Settings.volume : 0) * width + 50) / 100;
    fill = track;
    fill.right = knobX;
    BFFillRectColor(dc, &fill, palette->accent);

    knob.left = knobX - 5;
    knob.right = knobX + 5;
    knob.top = track.top - 6;
    knob.bottom = track.bottom + 6;
    BFDrawBox(dc, knob, palette->accent, palette->text, 1);
    *trackOut = track;
}

void BFSetVolumeFromTrack(RECT track, int x)
{
    int width = BFMaxInt(1, track.right - track.left);
    int value = ((x - track.left) * 100 + width / 2) / width;
    BF_Settings.sound = 1;
    BF_Settings.volume = BFClampInt(value, 0, 100);
    BFApplyPreviewVolume();
    BFSaveState();
    BFInvalidateAllWindows();
}

int BFColumnCount(int width)
{
    if (width >= 980) {
        return 3;
    }
    if (width >= 660) {
        return 2;
    }
    return 1;
}

void BFDrawSortBar(HDC dc, RECT bounds, HFONT font, RECT sortButtons[3], RECT *volumeTrack, BFSortMode sort)
{
    int width = BFMinInt(116, BFMaxInt(92, (bounds.right - bounds.left - 380) / 3));
    RECT volumeBounds;

    sortButtons[0].left = bounds.left;
    sortButtons[0].top = bounds.top;
    sortButtons[0].right = sortButtons[0].left + width;
    sortButtons[0].bottom = bounds.bottom;
    sortButtons[1] = sortButtons[0];
    OffsetRect(&sortButtons[1], width + 8, 0);
    sortButtons[2] = sortButtons[1];
    OffsetRect(&sortButtons[2], width + 8, 0);

    BFDrawButton(dc, sortButtons[0], BFT(BF_TX_LATEST), font, sort == BF_SORT_NEWEST);
    BFDrawButton(dc, sortButtons[1], BFT(BF_TX_OLDEST), font, sort == BF_SORT_OLDEST);
    BFDrawButton(dc, sortButtons[2], BFT(BF_TX_STAR_SORT), font, sort == BF_SORT_STARRED);

    volumeBounds.left = sortButtons[2].right + 20;
    volumeBounds.top = bounds.top;
    volumeBounds.right = bounds.right;
    volumeBounds.bottom = bounds.bottom;
    if (volumeBounds.right - volumeBounds.left > 180) {
        BFDrawVolume(dc, volumeBounds, font, volumeTrack);
    } else {
        SetRectEmpty(volumeTrack);
    }
}

int BFIsStarred(const wchar_t *id)
{
    size_t i;
    if (!BFHasText(id)) {
        return 0;
    }
    for (i = 0; i < BF_StarCount; ++i) {
        if (wcscmp(BF_Stars[i].id, id) == 0) {
            return 1;
        }
    }
    return 0;
}

unsigned int BFStarOrder(const wchar_t *id)
{
    size_t i;
    if (!BFHasText(id)) {
        return 0;
    }
    for (i = 0; i < BF_StarCount; ++i) {
        if (wcscmp(BF_Stars[i].id, id) == 0) {
            return BF_Stars[i].order;
        }
    }
    return 0;
}

int BFCompareMedia(const BFMediaItem *a, const BFMediaItem *b, BFSortMode sort)
{
    int dateCompare;
    unsigned int orderA;
    unsigned int orderB;

    if (sort == BF_SORT_STARRED) {
        orderA = BFStarOrder(a->id);
        orderB = BFStarOrder(b->id);
        if (orderA != 0 && orderB != 0) {
            return orderA < orderB ? -1 : (orderA > orderB ? 1 : 0);
        }
        if (orderA != 0) {
            return -1;
        }
        if (orderB != 0) {
            return 1;
        }
        dateCompare = wcscmp(a->date, b->date);
        return -dateCompare;
    }

    dateCompare = wcscmp(a->date, b->date);
    return sort == BF_SORT_OLDEST ? dateCompare : -dateCompare;
}

void BFSortMediaRefs(BFMediaRef *items, int count, BFSortMode sort)
{
    int i;
    int j;
    for (i = 1; i < count; ++i) {
        BFMediaRef value = items[i];
        j = i - 1;
        while (j >= 0 && BFCompareMedia(value.item, items[j].item, sort) < 0) {
            items[j + 1] = items[j];
            --j;
        }
        items[j + 1] = value;
    }
}

static int BFGetStatePath(wchar_t *path, size_t capacity)
{
    DWORD length;
    if (capacity == 0) {
        return 0;
    }
    length = GetEnvironmentVariableW(L"APPDATA", path, (DWORD)capacity);
    if (length == 0 || length >= capacity) {
        if (GetModuleFileNameW(NULL, path, (DWORD)capacity) == 0) {
            path[0] = L'\0';
            return 0;
        }
        path[capacity - 1] = L'\0';
        {
            wchar_t *slash = wcsrchr(path, L'\\');
            if (slash != NULL) {
                *(slash + 1) = L'\0';
            }
        }
        BFAppendString(path, capacity, L"BFState.txt");
        return 1;
    }
    BFAppendString(path, capacity, L"\\BlackFix");
    CreateDirectoryW(path, NULL);
    BFAppendString(path, capacity, L"\\BFState.txt");
    return 1;
}

static wchar_t *BFTrim(wchar_t *text)
{
    wchar_t *end;
    while (*text != L'\0' && iswspace(*text)) {
        ++text;
    }
    end = text + wcslen(text);
    while (end > text && iswspace(*(end - 1))) {
        --end;
    }
    *end = L'\0';
    return text;
}

void BFSaveState(void)
{
    wchar_t path[BF_PATH_CAPACITY];
    FILE *file;
    size_t i;

    if (!BFGetStatePath(path, sizeof(path) / sizeof(path[0]))) {
        return;
    }

    file = _wfopen(path, L"w");
    if (file == NULL) {
        return;
    }

    fwprintf(file, L"theme=%d\n", BF_Settings.theme);
    fwprintf(file, L"sound=%d\n", BF_Settings.sound);
    fwprintf(file, L"language=%d\n", BF_Settings.language);
    fwprintf(file, L"volume=%d\n", BF_Settings.volume);
    fwprintf(file, L"nextStar=%u\n", BF_NextStarOrder);

    for (i = 0; i < BF_StarCount; ++i) {
        fwprintf(file, L"star=%ls|%u\n", BF_Stars[i].id, BF_Stars[i].order);
    }
    fclose(file);
}

void BFLoadState(void)
{
    wchar_t path[BF_PATH_CAPACITY];
    FILE *file;
    wchar_t line[512];

    BF_Settings.theme = BF_THEME_TERMINAL;
    BF_Settings.sound = 1;
    BF_Settings.language = BF_LANG_KO;
    BF_Settings.volume = 80;
    BF_StarCount = 0;
    BF_NextStarOrder = 1;

    if (!BFGetStatePath(path, sizeof(path) / sizeof(path[0]))) {
        return;
    }
    file = _wfopen(path, L"r");
    if (file == NULL) {
        return;
    }

    while (fgetws(line, sizeof(line) / sizeof(line[0]), file) != NULL) {
        wchar_t *text = BFTrim(line);
        wchar_t *value;
        if (wcsncmp(text, L"theme=", 6) == 0) {
            BF_Settings.theme = BFClampInt(_wtoi(text + 6), 0, BF_THEME_COUNT - 1);
        } else if (wcsncmp(text, L"sound=", 6) == 0) {
            BF_Settings.sound = _wtoi(text + 6) != 0;
        } else if (wcsncmp(text, L"language=", 9) == 0) {
            BF_Settings.language = BFClampInt(_wtoi(text + 9), 0, BF_LANG_COUNT - 1);
        } else if (wcsncmp(text, L"volume=", 7) == 0) {
            BF_Settings.volume = BFClampInt(_wtoi(text + 7), 0, 100);
        } else if (wcsncmp(text, L"nextStar=", 9) == 0) {
            BF_NextStarOrder = (unsigned int)BFMaxInt(1, _wtoi(text + 9));
        } else if (wcsncmp(text, L"star=", 5) == 0 && BF_StarCount < BF_MAX_STARS) {
            value = wcschr(text + 5, L'|');
            if (value != NULL) {
                *value = L'\0';
                BFCopyString(BF_Stars[BF_StarCount].id, sizeof(BF_Stars[BF_StarCount].id) / sizeof(BF_Stars[BF_StarCount].id[0]), text + 5);
                BF_Stars[BF_StarCount].order = (unsigned int)BFMaxInt(1, _wtoi(value + 1));
                ++BF_StarCount;
            }
        }
    }
    fclose(file);
}

void BFToggleStar(const wchar_t *id)
{
    size_t i;
    if (!BFHasText(id)) {
        return;
    }
    for (i = 0; i < BF_StarCount; ++i) {
        if (wcscmp(BF_Stars[i].id, id) == 0) {
            while (i + 1 < BF_StarCount) {
                BF_Stars[i] = BF_Stars[i + 1];
                ++i;
            }
            --BF_StarCount;
            BFSaveState();
            BFInvalidateAllWindows();
            return;
        }
    }
    if (BF_StarCount < BF_MAX_STARS) {
        BFCopyString(BF_Stars[BF_StarCount].id, sizeof(BF_Stars[BF_StarCount].id) / sizeof(BF_Stars[BF_StarCount].id[0]), id);
        BF_Stars[BF_StarCount].order = BF_NextStarOrder++;
        ++BF_StarCount;
        BFSaveState();
        BFInvalidateAllWindows();
    }
}

void BFInvalidateAllWindows(void)
{
    size_t i;
    if (BF_MainWindow != NULL && IsWindow(BF_MainWindow)) {
        InvalidateRect(BF_MainWindow, NULL, FALSE);
    }
    if (BF_CheonWindow != NULL && IsWindow(BF_CheonWindow)) {
        InvalidateRect(BF_CheonWindow, NULL, FALSE);
    }
    if (BF_CioWindow != NULL && IsWindow(BF_CioWindow)) {
        InvalidateRect(BF_CioWindow, NULL, FALSE);
    }
    for (i = 0; i < BF_MAX_ALBUMS; ++i) {
        if (BF_AlbumWindows[i] != NULL && IsWindow(BF_AlbumWindows[i])) {
            InvalidateRect(BF_AlbumWindows[i], NULL, FALSE);
        }
    }
    if (BF_SettingsWindow != NULL && IsWindow(BF_SettingsWindow)) {
        BFSettingsSyncControls();
        InvalidateRect(BF_SettingsWindow, NULL, FALSE);
    }
}

static int BFIsAbsoluteOrUrl(const wchar_t *source)
{
    if (!BFHasText(source)) {
        return 0;
    }
    if (wcsstr(source, L"://") != NULL) {
        return 1;
    }
    if (iswalpha(source[0]) && source[1] == L':') {
        return 1;
    }
    if (source[0] == L'\\' && source[1] == L'\\') {
        return 1;
    }
    return 0;
}

static int BFIsUrl(const wchar_t *source)
{
    return BFHasText(source) && wcsstr(source, L"://") != NULL;
}

static void BFBuildMediaTarget(const wchar_t *source, wchar_t *target, size_t capacity)
{
    wchar_t *lastSlash;
    if (BFIsAbsoluteOrUrl(source)) {
        BFCopyString(target, capacity, source);
        return;
    }
    if (GetModuleFileNameW(NULL, target, (DWORD)capacity) == 0) {
        BFCopyString(target, capacity, source);
        return;
    }
    target[capacity - 1] = L'\0';
    lastSlash = wcsrchr(target, L'\\');
    if (lastSlash != NULL) {
        *(lastSlash + 1) = L'\0';
    } else {
        target[0] = L'\0';
    }
    BFAppendString(target, capacity, source);
}

void BFOpenLink(HWND hwnd, const wchar_t *link)
{
    HINSTANCE result;
    if (!BFHasText(link)) {
        return;
    }
    result = ShellExecuteW(hwnd, L"open", link, NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)result <= 32) {
        MessageBoxW(hwnd, L"링크를 열 수 없습니다.", L"BlackFix", MB_OK | MB_ICONWARNING);
    }
}

static void BFApplyPreviewVolume(void)
{
    wchar_t command[96];
    int volume = BF_Settings.sound ? BF_Settings.volume * 10 : 0;
    if (!BF_PreviewOpen) {
        return;
    }
    swprintf(command, sizeof(command) / sizeof(command[0]), L"setaudio BFPreview volume to %d", volume);
    mciSendStringW(command, NULL, 0, NULL);
}

void BFStopPreview(void)
{
    if (BF_PreviewOwner != NULL && IsWindow(BF_PreviewOwner)) {
        KillTimer(BF_PreviewOwner, BF_TIMER_PREVIEW);
    }
    if (BF_PreviewOpen) {
        mciSendStringW(L"stop BFPreview", NULL, 0, NULL);
        mciSendStringW(L"close BFPreview", NULL, 0, NULL);
    }
    BF_PreviewOpen = 0;
    BF_PreviewOwner = NULL;
    BF_PreviewId[0] = L'\0';
    BFInvalidateAllWindows();
}

int BFIsPreviewing(const BFMediaItem *item)
{
    return item != NULL && BF_PreviewOpen && wcscmp(BF_PreviewId, item->id) == 0;
}

void BFStartPreview(HWND hwnd, const BFMediaItem *item)
{
    wchar_t target[BF_PATH_CAPACITY];
    wchar_t command[BF_PATH_CAPACITY + 64];
    MCIERROR error;

    if (item == NULL || !BFHasText(item->previewSource)) {
        MessageBoxW(hwnd, BFT(BF_TX_PREVIEW_MISSING), L"BlackFix", MB_OK | MB_ICONINFORMATION);
        return;
    }
    BFBuildMediaTarget(item->previewSource, target, sizeof(target) / sizeof(target[0]));
    if (BFIsUrl(target) || GetFileAttributesW(target) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(hwnd, BFT(BF_TX_PREVIEW_MISSING), L"BlackFix", MB_OK | MB_ICONINFORMATION);
        return;
    }
    BFStopPreview();
    swprintf(command, sizeof(command) / sizeof(command[0]), L"open \"%ls\" alias BFPreview", target);
    error = mciSendStringW(command, NULL, 0, NULL);
    if (error != 0) {
        MessageBoxW(hwnd, BFT(BF_TX_PREVIEW_FAIL), L"BlackFix", MB_OK | MB_ICONWARNING);
        return;
    }
    BF_PreviewOpen = 1;
    BF_PreviewOwner = hwnd;
    BFCopyString(BF_PreviewId, sizeof(BF_PreviewId) / sizeof(BF_PreviewId[0]), item->id);
    BFApplyPreviewVolume();
    error = mciSendStringW(L"play BFPreview from 0", NULL, 0, NULL);
    if (error != 0) {
        BFStopPreview();
        MessageBoxW(hwnd, BFT(BF_TX_PREVIEW_FAIL), L"BlackFix", MB_OK | MB_ICONWARNING);
        return;
    }
    SetTimer(hwnd, BF_TIMER_PREVIEW, 60000, NULL);
    BFInvalidateAllWindows();
}

void BFHandlePreviewTimer(HWND hwnd, WPARAM timerId)
{
    if (timerId == BF_TIMER_PREVIEW) {
        BFStopPreview();
    } else if (timerId == BF_TIMER_ANIMATION) {
        ++BF_AnimTick;
        InvalidateRect(hwnd, NULL, FALSE);
    }
}

static void BFDrawStarGlyph(HDC dc, RECT rect, HFONT font, int starred)
{
    const BFPalette *palette = BFP();
    BFDrawTextBlock(dc, starred ? L"★" : L"☆", rect, font, starred ? palette->accent : palette->muted, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

static void BFDrawMediaCard(HDC dc, RECT card, const BFFonts *fonts, const BFMediaItem *item, BFMediaHit *hit)
{
    const BFPalette *palette = BFP();
    RECT title;
    RECT meta;
    RECT time;
    RECT desc;
    RECT status;
    RECT star;
    RECT preview;
    RECT link;
    RECT accent;
    int starred = BFIsStarred(item->id);
    int previewing = BFIsPreviewing(item);

    BFDrawBox(dc, card, previewing ? palette->selected : palette->panel, starred ? palette->accent : palette->border, starred ? 2 : 1);
    accent.left = card.left;
    accent.top = card.top;
    accent.right = card.left + 6;
    accent.bottom = card.bottom;
    BFFillRectColor(dc, &accent, palette->selected);

    star.left = card.right - 42;
    star.top = card.top + 10;
    star.right = card.right - 10;
    star.bottom = card.top + 42;
    BFDrawStarGlyph(dc, star, fonts->ui, starred);

    title.left = card.left + 18;
    title.top = card.top + 12;
    title.right = star.left - 8;
    title.bottom = card.top + 42;
    BFDrawTextBlock(dc, item->title, title, fonts->ui, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    time.left = card.right - 74;
    time.top = card.top + 43;
    time.right = card.right - 18;
    time.bottom = card.top + 68;
    BFDrawTextBlock(dc, item->length, time, fonts->small, palette->muted, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    meta.left = card.left + 18;
    meta.top = card.top + 43;
    meta.right = time.left - 10;
    meta.bottom = card.top + 68;
    {
        wchar_t buffer[192];
        swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"%ls  |  %ls", item->category, item->date);
        BFDrawTextBlock(dc, buffer, meta, fonts->small, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    desc.left = card.left + 18;
    desc.top = card.top + 70;
    desc.right = card.right - 18;
    desc.bottom = card.top + 112;
    BFDrawTextBlock(dc, item->description, desc, fonts->small, palette->text, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);

    status.left = card.left + 18;
    status.top = card.top + 113;
    status.right = card.right - 18;
    status.bottom = card.top + 134;
    BFDrawTextBlock(dc, previewing ? BFT(BF_TX_PREVIEW_RUNNING) : L"", status, fonts->small, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    preview.left = card.left + 18;
    preview.top = card.bottom - 42;
    preview.right = BFMinInt(card.right - 110, preview.left + 150);
    preview.bottom = card.bottom - 12;
    link.left = preview.right + 8;
    link.top = preview.top;
    link.right = BFMinInt(card.right - 18, link.left + 92);
    link.bottom = preview.bottom;
    BFDrawButton(dc, preview, BFT(BF_TX_PREVIEW), fonts->small, previewing);
    BFDrawButton(dc, link, BFT(BF_TX_LINK), fonts->small, 0);

    if (hit != NULL) {
        hit->body = card;
        hit->star = star;
        hit->preview = preview;
        hit->link = link;
        hit->item = item;
    }
}

static void BFDrawNoItems(HDC dc, RECT area, const BFFonts *fonts)
{
    const BFPalette *palette = BFP();
    RECT box = area;
    box.bottom = box.top + 80;
    BFDrawBox(dc, box, palette->panel, palette->border, 1);
    InflateRect(&box, -16, -12);
    BFDrawTextBlock(dc, BFT(BF_TX_NO_ITEMS), box, fonts->small, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void BFDrawMediaGrid(HDC dc, RECT area, const BFFonts *fonts, const BFMediaRef *items, int count, int scrollY, int *contentHeight, BFMediaHit *hits, int *hitCount)
{
    int columns = BFColumnCount(area.right - area.left);
    int cardWidth = (area.right - area.left - (columns - 1) * BF_GAP) / columns;
    int rows = columns > 0 ? (count + columns - 1) / columns : 0;
    int startY = area.top - scrollY;
    int i;
    HRGN region;

    *hitCount = 0;
    *contentHeight = rows == 0 ? 90 : rows * BF_MEDIA_CARD_H + BFMaxInt(0, rows - 1) * BF_GAP + 24;
    region = CreateRectRgn(area.left, area.top, area.right, area.bottom);
    SelectClipRgn(dc, region);
    if (count == 0) {
        BFDrawNoItems(dc, area, fonts);
    }
    for (i = 0; i < count; ++i) {
        int row = i / columns;
        int column = i % columns;
        RECT card;
        card.left = area.left + column * (cardWidth + BF_GAP);
        card.top = startY + row * (BF_MEDIA_CARD_H + BF_GAP);
        card.right = card.left + cardWidth;
        card.bottom = card.top + BF_MEDIA_CARD_H;
        if (card.bottom < area.top || card.top > area.bottom) {
            continue;
        }
        if (*hitCount < BF_MAX_HITS) {
            BFDrawMediaCard(dc, card, fonts, items[i].item, &hits[*hitCount]);
            ++(*hitCount);
        } else {
            BFDrawMediaCard(dc, card, fonts, items[i].item, NULL);
        }
    }
    SelectClipRgn(dc, NULL);
    DeleteObject(region);
}

static void BFDrawGoodsCard(HDC dc, RECT card, const BFFonts *fonts, const BFGoods *goods)
{
    const BFPalette *palette = BFP();
    RECT name;
    RECT meta;
    RECT line;
    RECT desc;
    RECT link;
    RECT accent;

    BFDrawBox(dc, card, palette->panel, palette->border, 1);
    accent.left = card.left;
    accent.top = card.top;
    accent.right = card.left + 6;
    accent.bottom = card.bottom;
    BFFillRectColor(dc, &accent, palette->selected);

    name.left = card.left + 18;
    name.top = card.top + 14;
    name.right = card.right - 18;
    name.bottom = card.top + 44;
    BFDrawTextBlock(dc, goods->name, name, fonts->ui, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    meta.left = card.left + 18;
    meta.top = card.top + 45;
    meta.right = card.right - 18;
    meta.bottom = card.top + 70;
    {
        wchar_t buffer[192];
        swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"%ls  |  %ls", goods->type, goods->price);
        BFDrawTextBlock(dc, buffer, meta, fonts->small, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    line.left = card.left + 18;
    line.top = card.top + 72;
    line.right = card.right - 18;
    line.bottom = card.top + 96;
    {
        wchar_t buffer[192];
        swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"%ls  |  %ls", goods->line, goods->date);
        BFDrawTextBlock(dc, buffer, line, fonts->small, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
    BFDrawLine(dc, card.left + 18, card.top + 98, card.right - 18, card.top + 98, palette->border, 1);

    desc.left = card.left + 18;
    desc.top = card.top + 104;
    desc.right = card.right - 18;
    desc.bottom = card.bottom - 36;
    BFDrawTextBlock(dc, goods->description, desc, fonts->small, palette->text, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);

    link.left = card.left + 18;
    link.top = card.bottom - 34;
    link.right = card.right - 18;
    link.bottom = card.bottom - 10;
    BFDrawTextBlock(dc, L"LINK >", link, fonts->small, palette->accent, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void BFDrawGoodsGrid(HDC dc, RECT area, const BFFonts *fonts, int scrollY, int *contentHeight, BFGoodsHit *hits, int *hitCount)
{
    int columns = BFColumnCount(area.right - area.left);
    int cardWidth = (area.right - area.left - (columns - 1) * BF_GAP) / columns;
    int rows = columns > 0 ? ((int)BF_GOODS_COUNT + columns - 1) / columns : 0;
    int startY = area.top - scrollY;
    int i;
    HRGN region;

    *hitCount = 0;
    *contentHeight = rows == 0 ? 90 : rows * BF_GOODS_CARD_H + BFMaxInt(0, rows - 1) * BF_GAP + 24;
    region = CreateRectRgn(area.left, area.top, area.right, area.bottom);
    SelectClipRgn(dc, region);
    if (BF_GOODS_COUNT == 0) {
        BFDrawNoItems(dc, area, fonts);
    }
    for (i = 0; i < (int)BF_GOODS_COUNT; ++i) {
        int row = i / columns;
        int column = i % columns;
        RECT card;
        card.left = area.left + column * (cardWidth + BF_GAP);
        card.top = startY + row * (BF_GOODS_CARD_H + BF_GAP);
        card.right = card.left + cardWidth;
        card.bottom = card.top + BF_GOODS_CARD_H;
        if (card.bottom < area.top || card.top > area.bottom) {
            continue;
        }
        BFDrawGoodsCard(dc, card, fonts, &BF_GOODS[i]);
        if (*hitCount < BF_MAX_HITS) {
            hits[*hitCount].body = card;
            hits[*hitCount].goods = &BF_GOODS[i];
            ++(*hitCount);
        }
    }
    SelectClipRgn(dc, NULL);
    DeleteObject(region);
}

void BFHandleMediaClick(HWND hwnd, BFMediaHit *hits, int hitCount, int x, int y)
{
    int i;
    for (i = 0; i < hitCount; ++i) {
        if (!BFPointInRect(&hits[i].body, x, y)) {
            continue;
        }
        if (BFPointInRect(&hits[i].star, x, y)) {
            BFToggleStar(hits[i].item->id);
            return;
        }
        if (BFPointInRect(&hits[i].preview, x, y)) {
            BFStartPreview(hwnd, hits[i].item);
            return;
        }
        BFOpenLink(hwnd, hits[i].item->link);
        return;
    }
}

int BFRegisterAppWindows(HINSTANCE instance)
{
    return BFRegisterBlackFixWindow(instance) &&
           BFRegisterAlbumWindow(instance) &&
           BFRegisterMemberWindow(instance) &&
           BFRegisterSettingsWindow(instance);
}

int BFRunApplication(HINSTANCE instance, int showCommand)
{
    HWND existing;
    MSG message;
    INITCOMMONCONTROLSEX controls;

    BF_Instance = instance;
    SetProcessDPIAware();

    BF_SingletonMutex = CreateMutexW(NULL, TRUE, L"Local\\BlackFixSingleton");
    if (BF_SingletonMutex != NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
        existing = FindWindowW(BF_MAIN_CLASS, NULL);
        if (existing != NULL) {
            ShowWindow(existing, SW_SHOWNORMAL);
            SetForegroundWindow(existing);
        }
        CloseHandle(BF_SingletonMutex);
        BF_SingletonMutex = NULL;
        return 0;
    }

    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&controls);
    BFLoadState();
    BFCheckForUpdate();

    if (!BFRegisterAppWindows(instance)) {
        MessageBoxW(NULL, L"창 클래스를 등록할 수 없습니다.", L"BlackFix", MB_OK | MB_ICONERROR);
        return 1;
    }

    BFOpenBlackFixWindow(instance, showCommand);
    if (BF_MainWindow == NULL) {
        MessageBoxW(NULL, L"BlackFix 창을 만들 수 없습니다.", L"BlackFix", MB_OK | MB_ICONERROR);
        return 1;
    }

    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        if (message.message == WM_KEYDOWN && message.wParam == VK_ESCAPE) {
            BFToggleSettingsWindow(message.hwnd);
            continue;
        }
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    BFStopPreview();
    BFSaveState();
    if (BF_SingletonMutex != NULL) {
        CloseHandle(BF_SingletonMutex);
        BF_SingletonMutex = NULL;
    }
    return (int)message.wParam;
}
