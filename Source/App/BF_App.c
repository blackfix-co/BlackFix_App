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
#include "BF_Installer.h"

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
BFAppSettings BF_Settings = {BF_THEME_TERMINAL, 1, BF_LANG_KO, 80, 1, BF_EFFECT_PIXEL, 5, 30, 80};
int BF_AnimTick;
HWND BF_VolumeCaptureWindow;

static HANDLE BF_SingletonMutex;
static BFStarEntry BF_Stars[BF_MAX_STARS];
static size_t BF_StarCount;
static unsigned int BF_NextStarOrder = 1;
static unsigned int BF_EffectSeed = 0x4bf123u;
static ULONGLONG BF_AnimStartMs;
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
    {RGB(1, 5, 2), RGB(3, 17, 7), RGB(5, 28, 11), RGB(7, 43, 14), RGB(183, 255, 195), RGB(92, 176, 105), RGB(76, 255, 94), RGB(24, 92, 35), RGB(16, 86, 28), RGB(22, 112, 41)},
    {RGB(199, 150, 78), RGB(218, 168, 91), RGB(188, 133, 63), RGB(75, 52, 31), RGB(30, 25, 20), RGB(78, 54, 33), RGB(20, 20, 17), RGB(99, 70, 43), RGB(235, 220, 184), RGB(32, 30, 26)},
    {RGB(34, 35, 38), RGB(238, 238, 230), RGB(90, 97, 106), RGB(48, 52, 58), RGB(24, 27, 31), RGB(70, 76, 84), RGB(27, 132, 184), RGB(86, 92, 101), RGB(196, 224, 242), RGB(170, 207, 230)},
    {RGB(247, 249, 250), RGB(255, 255, 255), RGB(235, 240, 244), RGB(212, 222, 230), RGB(25, 31, 36), RGB(91, 103, 113), RGB(15, 124, 90), RGB(169, 184, 194), RGB(210, 237, 227), RGB(224, 232, 238)},
    {RGB(246, 242, 231), RGB(255, 253, 244), RGB(236, 229, 214), RGB(99, 109, 111), RGB(24, 25, 26), RGB(93, 91, 86), RGB(29, 107, 154), RGB(163, 151, 130), RGB(216, 235, 244), RGB(227, 219, 201)}
};

static const wchar_t *BF_Text[BF_LANG_COUNT][BF_TX_COUNT] = {
    {
        L"앨범", L"노래", L"굿즈", L"천", L"씨오", L"PIXEL CATALOG", L"앨범 목록", L"전체 노래", L"굿즈",
        L"최신순", L"오래된순", L"별표 순", L"별표", L"영상 목록", L"쇼츠 목록",
        L"1분 미리보기", L"이동", L"소리", L"설정", L"화면", L"마우스 클릭 이펙트",
        L"이펙트 종류", L"이펙트 속도", L"이펙트 시간", L"이펙트 불투명도", L"테마", L"소리", L"언어설정", L"전체 영상",
        L"표시할 항목이 없습니다.", L"터미널", L"다크", L"우주", L"책", L"밝은 픽셀", L"딥 그린", L"바둑판", L"체스판", L"화이트", L"악보",
        L"불", L"물", L"우주", L"픽셀", L"바둑돌", L"체스말", L"음표",
        L"한국어", L"영어", L"일본어",
        L"미리보기 파일이 없습니다.",
        L"미리보기를 재생할 수 없습니다.",
        L"1분 미리보기 재생 중"
    },
    {
        L"Albums", L"Songs", L"Goods", L"Cheon", L"Cio", L"PIXEL CATALOG", L"Albums", L"All Songs", L"Goods",
        L"Newest", L"Oldest", L"Star Order", L"Stars", L"Videos", L"Shorts",
        L"1 min preview", L"Open", L"Sound", L"Settings", L"Screen", L"Mouse click effect",
        L"Effect type", L"Effect speed", L"Effect time", L"Effect opacity", L"Theme", L"Sound", L"Language", L"All videos",
        L"No items to show.", L"Terminal", L"Dark", L"Space", L"Book", L"Light Pixel", L"Deep Green", L"Go Board", L"Chess Board", L"White", L"Score",
        L"Fire", L"Water", L"Space", L"Pixel", L"Go Stone", L"Chess Piece", L"Notes",
        L"Korean", L"English", L"Japanese",
        L"Preview file is missing.",
        L"Could not play the preview.",
        L"Playing 1 minute preview"
    },
    {
        L"アルバム", L"曲", L"グッズ", L"チョン", L"シオ", L"PIXEL CATALOG", L"アルバム", L"全曲", L"グッズ",
        L"新しい順", L"古い順", L"星順", L"星", L"動画リスト", L"ショート",
        L"1分プレビュー", L"移動", L"音量", L"設定", L"画面", L"クリック効果",
        L"効果タイプ", L"効果速度", L"効果時間", L"効果不透明度", L"テーマ", L"音", L"言語", L"全動画",
        L"表示する項目がありません。", L"ターミナル", L"ダーク", L"宇宙", L"本", L"ライトピクセル", L"ディープグリーン", L"碁盤", L"チェス盤", L"ホワイト", L"楽譜",
        L"火", L"水", L"宇宙", L"ピクセル", L"碁石", L"チェス駒", L"音符",
        L"韓国語", L"英語", L"日本語",
        L"プレビューファイルがありません。",
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

void BFRedrawNow(HWND hwnd)
{
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOERASE);
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

static int BFCurrentAnimationTick(void)
{
    ULONGLONG now = GetTickCount64();
    if (BF_AnimStartMs == 0) {
        BF_AnimStartMs = now;
    }
    return (int)((now - BF_AnimStartMs) / BF_EFFECT_TIMER_MS);
}

static void BFDrawBadukBoard(HDC dc, const RECT *client)
{
    const BFPalette *palette = BFP();
    int gap = 36;
    int left = client->left + 18;
    int top = client->top + 18;
    int right = client->right - 18;
    int bottom = client->bottom - 18;
    int x;
    int y;

    BFFillRectColor(dc, client, palette->bg);
    for (x = left; x <= right; x += gap) {
        BFDrawLine(dc, x, top, x, bottom, palette->grid, 1);
    }
    for (y = top; y <= bottom; y += gap) {
        BFDrawLine(dc, left, y, right, y, palette->grid, 1);
    }
    for (x = left + gap * 3; x <= right; x += gap * 6) {
        for (y = top + gap * 3; y <= bottom; y += gap * 6) {
            RECT dot;
            dot.left = x - 3;
            dot.top = y - 3;
            dot.right = x + 4;
            dot.bottom = y + 4;
            BFFillRectColor(dc, &dot, palette->grid);
        }
    }
}

static void BFDrawChessBoard(HDC dc, const RECT *client)
{
    const BFPalette *palette = BFP();
    int size = 48;
    int x;
    int y;
    RECT square;

    BFFillRectColor(dc, client, palette->bg);
    for (y = client->top; y < client->bottom; y += size) {
        for (x = client->left; x < client->right; x += size) {
            square.left = x;
            square.top = y;
            square.right = BFMinInt(x + size, client->right);
            square.bottom = BFMinInt(y + size, client->bottom);
            BFFillRectColor(dc, &square, (((x - client->left) / size) + ((y - client->top) / size)) % 2 == 0 ? palette->panel : palette->panelAlt);
        }
    }
}

static void BFDrawSpaceBoard(HDC dc, const RECT *client)
{
    const BFPalette *palette = BFP();
    int width = BFMaxInt(1, client->right - client->left);
    int height = BFMaxInt(1, client->bottom - client->top);
    int tick = BFCurrentAnimationTick();
    int i;

    BFFillRectColor(dc, client, palette->bg);

    for (i = 0; i < 150; ++i) {
        unsigned int seed = (unsigned int)(i * 1103515245u + 0x4bf123u);
        int x = client->left + (int)((seed >> 7) % (unsigned int)width);
        int y = client->top + (int)((seed >> 18) % (unsigned int)height);
        int phase = (tick + i * 3) % 48;
        int bright = phase < 24 ? phase : 48 - phase;
        int size = 1 + (int)((seed >> 28) % 3);
        COLORREF color = bright > 16 ? palette->text : (i % 4 == 0 ? palette->accent : palette->muted);
        RECT star;

        star.left = x;
        star.top = y;
        star.right = x + size;
        star.bottom = y + size;
        BFFillRectColor(dc, &star, color);
        if ((seed & 15u) == 0u) {
            BFDrawLine(dc, x - 3, y, x + 4, y, color, 1);
            BFDrawLine(dc, x, y - 3, x, y + 4, color, 1);
        }
    }

    for (i = 0; i < 9; ++i) {
        int y = client->top + 48 + i * 84 + (tick % 28);
        int x1 = client->left + 18 + i * 46;
        int x2 = x1 + 86;
        if (y >= client->bottom) {
            continue;
        }
        BFDrawLine(dc, x1, y, x2, y - 34, i % 2 == 0 ? palette->border : palette->selected, 1);
    }
}

static void BFDrawScoreBoard(HDC dc, const RECT *client)
{
    const BFPalette *palette = BFP();
    static const wchar_t *notes[8] = {L"♪", L"♫", L"♬", L"♩", L"♭", L"♯", L"♮", L"♬"};
    int width = BFMaxInt(1, client->right - client->left);
    int height = BFMaxInt(1, client->bottom - client->top);
    int tick = BFCurrentAnimationTick();
    int i;

    BFFillRectColor(dc, client, palette->bg);
    for (i = 0; i < 120; ++i) {
        unsigned int seed = (unsigned int)(i * 1664525u + 1013904223u);
        int x = client->left + (int)((seed >> 5) % (unsigned int)width);
        int y = client->top + (int)((seed >> 17) % (unsigned int)height);
        int phase = (tick + i * 5) % 72;
        int size = 18 + (int)((seed >> 28) % 24);
        RECT note;
        HFONT font;
        COLORREF color;

        if (i % 5 == 0) {
            y += phase / 6;
        }
        note.left = x - size;
        note.top = y - size;
        note.right = x + size;
        note.bottom = y + size;
        color = i % 7 == 0 ? palette->accent : (i % 3 == 0 ? palette->muted : palette->text);
        font = CreateFontW(-size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
        BFDrawTextBlock(dc, notes[(i + phase / 12) % 8], note, font != NULL ? font : GetStockObject(DEFAULT_GUI_FONT), color, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        if (font != NULL) {
            DeleteObject(font);
        }
    }
}

void BFDrawGrid(HDC dc, const RECT *client)
{
    const BFPalette *palette = BFP();
    int x;
    int y;
    int tick = BFCurrentAnimationTick();
    int offset = tick % 16;
    int scanY = client->top + (tick * 2) % BFMaxInt(1, client->bottom - client->top);
    HPEN pen;
    HGDIOBJ oldPen;

    BF_AnimTick = tick;
    if (BF_Settings.theme == BF_THEME_SPACE) {
        BFDrawSpaceBoard(dc, client);
        return;
    }
    if (BF_Settings.theme == BF_THEME_BADUK) {
        BFDrawBadukBoard(dc, client);
        BFDrawLine(dc, client->left, scanY, client->right, scanY, palette->selected, 1);
        return;
    }
    if (BF_Settings.theme == BF_THEME_CHESS) {
        BFDrawChessBoard(dc, client);
        BFDrawLine(dc, client->left, scanY, client->right, scanY, palette->accent, 1);
        return;
    }
    if (BF_Settings.theme == BF_THEME_SCORE) {
        BFDrawScoreBoard(dc, client);
        BFDrawLine(dc, client->left, scanY, client->right, scanY, palette->selected, 1);
        return;
    }
    BFFillRectColor(dc, client, palette->bg);
    pen = CreatePen(PS_SOLID, 1, palette->grid);
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

void BFBeginDragScroll(BFDragScroll *drag, int x, int y, int scrollY)
{
    drag->active = 1;
    drag->moved = 0;
    drag->startX = x;
    drag->startY = y;
    drag->startScroll = scrollY;
    drag->lastX = x;
    drag->lastY = y;
    drag->effectX = x;
    drag->effectY = y;
}

int BFUpdateDragScroll(BFDragScroll *drag, int x, int y, int *scrollY)
{
    int dx;
    int dy;
    int next;

    if (drag == NULL || !drag->active) {
        return 0;
    }

    dx = x - drag->startX;
    dy = y - drag->startY;
    if (!drag->moved && (dx * dx + dy * dy) < 25) {
        return 0;
    }

    drag->moved = 1;
    next = drag->startScroll - dy;
    drag->lastX = x;
    drag->lastY = y;
    if (*scrollY != next) {
        *scrollY = next;
        return 1;
    }

    return 0;
}

int BFEndDragScroll(BFDragScroll *drag)
{
    int wasClick;

    if (drag == NULL || !drag->active) {
        return 0;
    }

    wasClick = !drag->moved;
    drag->active = 0;
    drag->moved = 0;
    return wasClick;
}

static int BFNextEffectRand(int modulo)
{
    BF_EffectSeed = BF_EffectSeed * 1103515245u + 12345u;
    return modulo > 0 ? (int)((BF_EffectSeed >> 16) % (unsigned int)modulo) : 0;
}

static COLORREF BFBlendEffectColor(COLORREF base, COLORREF color, int opacity)
{
    int r = GetRValue(base) + (GetRValue(color) - GetRValue(base)) * opacity / 100;
    int g = GetGValue(base) + (GetGValue(color) - GetGValue(base)) * opacity / 100;
    int b = GetBValue(base) + (GetBValue(color) - GetBValue(base)) * opacity / 100;
    return RGB(BFClampInt(r, 0, 255), BFClampInt(g, 0, 255), BFClampInt(b, 0, 255));
}

static int BFScaledVelocity(int value)
{
    int speed = BFClampInt(BF_Settings.clickEffectSpeed, 1, 10);
    if (value >= 0) {
        return (value * speed + 2) / 4;
    }
    return -((-value * speed + 2) / 4);
}

static void BFEffectColors(COLORREF *primary, COLORREF *secondary)
{
    switch (BFClampInt(BF_Settings.clickEffectStyle, 0, BF_EFFECT_COUNT - 1)) {
    case BF_EFFECT_FIRE:
        *primary = RGB(255, 82, 32);
        *secondary = RGB(255, 202, 74);
        break;
    case BF_EFFECT_WATER:
        *primary = RGB(50, 190, 255);
        *secondary = RGB(160, 245, 255);
        break;
    case BF_EFFECT_SPACE:
        *primary = RGB(162, 91, 255);
        *secondary = RGB(88, 221, 255);
        break;
    case BF_EFFECT_PIXEL:
    case BF_EFFECT_BADUK:
    default:
        *primary = RGB(42, 255, 121);
        *secondary = RGB(221, 255, 230);
        break;
    case BF_EFFECT_CHESS:
        *primary = RGB(188, 193, 199);
        *secondary = RGB(78, 207, 255);
        break;
    case BF_EFFECT_NOTE:
        *primary = RGB(22, 92, 145);
        *secondary = RGB(24, 25, 26);
        break;
    }
}

static BFClickEffect *BFNextClickEffect(BFClickEffects *effects)
{
    BFClickEffect *effect = &effects->items[effects->next % BF_CLICK_EFFECT_MAX];
    ZeroMemory(effect, sizeof(*effect));
    effects->next = (effects->next + 1) % BF_CLICK_EFFECT_MAX;
    effect->active = 1;
    effect->life = BFClampInt(BF_Settings.clickEffectDuration, 10, 90);
    if (effects->lastStepMs == 0) {
        effects->lastStepMs = GetTickCount64();
    }
    return effect;
}

void BFAddClickEffect(BFClickEffects *effects, int x, int y)
{
    BFClickEffect *effect;
    COLORREF primary;
    COLORREF secondary;
    int style;
    int count;
    int i;

    if (effects == NULL || !BF_Settings.clickEffect) {
        return;
    }

    style = BFClampInt(BF_Settings.clickEffectStyle, 0, BF_EFFECT_COUNT - 1);
    BFEffectColors(&primary, &secondary);

    if (style == BF_EFFECT_BADUK) {
        effect = BFNextClickEffect(effects);
        effect->kind = 2;
        effect->x = x;
        effect->y = y;
        effect->size = 11 + BFNextEffectRand(5);
        effect->extra = BFNextEffectRand(2);
        effect->color = effect->extra == 0 ? RGB(12, 12, 12) : RGB(238, 232, 218);
        effect->life = BFClampInt(BF_Settings.clickEffectDuration + 20, 24, 110);
        return;
    }

    if (style == BF_EFFECT_CHESS) {
        effect = BFNextClickEffect(effects);
        effect->kind = 3;
        effect->x = x;
        effect->y = y;
        effect->size = 30 + BFNextEffectRand(10);
        effect->extra = BFNextEffectRand(12);
        effect->color = effect->extra < 6 ? primary : RGB(24, 27, 31);
        effect->life = BFClampInt(BF_Settings.clickEffectDuration + 16, 24, 106);
        return;
    }

    if (style == BF_EFFECT_NOTE) {
        for (i = 0; i < 8; ++i) {
            effect = BFNextClickEffect(effects);
            effect->kind = 6;
            effect->x = x + BFNextEffectRand(23) - 11;
            effect->y = y + BFNextEffectRand(17) - 8;
            effect->size = 20 + BFNextEffectRand(12);
            effect->extra = BFNextEffectRand(6);
            effect->dx = BFNextEffectRand(7) - 3;
            effect->dy = -2 - BFNextEffectRand(5);
            effect->color = (i % 3 == 0) ? secondary : primary;
        }
        return;
    }

    count = style == BF_EFFECT_PIXEL ? 8 : 10;

    for (i = 0; i < count; ++i) {
        int angle = i % 8;
        effect = BFNextClickEffect(effects);
        effect->kind = 0;
        effect->x = x + BFNextEffectRand(9) - 4;
        effect->y = y + BFNextEffectRand(9) - 4;
        effect->size = 2 + BFNextEffectRand(style == BF_EFFECT_PIXEL ? 4 : 5);
        effect->color = (i % 3 == 0) ? secondary : primary;

        if (style == BF_EFFECT_FIRE) {
            effect->dx = BFNextEffectRand(7) - 3;
            effect->dy = -2 - BFNextEffectRand(6);
        } else if (style == BF_EFFECT_WATER) {
            effect->dx = BFNextEffectRand(9) - 4;
            effect->dy = BFNextEffectRand(7) - 3;
        } else if (style == BF_EFFECT_SPACE) {
            static const int vx[8] = {4, 3, 0, -3, -4, -3, 0, 3};
            static const int vy[8] = {0, -3, -4, -3, 0, 3, 4, 3};
            effect->dx = vx[angle] + BFNextEffectRand(3) - 1;
            effect->dy = vy[angle] + BFNextEffectRand(3) - 1;
        } else if (style == BF_EFFECT_PIXEL) {
            effect->dx = (BFNextEffectRand(5) - 2) * 2;
            effect->dy = (BFNextEffectRand(5) - 2) * 2;
        } else {
            effect->dx = BFNextEffectRand(9) - 4;
            effect->dy = -BFNextEffectRand(5) + BFNextEffectRand(3);
        }
    }
}

void BFAddDragEffect(BFClickEffects *effects, int x1, int y1, int x2, int y2)
{
    BFClickEffect *effect;
    COLORREF primary;
    COLORREF secondary;
    int length;
    int style;
    int i;

    if (effects == NULL || !BF_Settings.clickEffect) {
        return;
    }

    length = x2 - x1;
    if (length < 0) {
        length = -length;
    }
    if (length < 10 && (y2 - y1) * (y2 - y1) < 100) {
        return;
    }

    style = BFClampInt(BF_Settings.clickEffectStyle, 0, BF_EFFECT_COUNT - 1);
    BFEffectColors(&primary, &secondary);

    if (style == BF_EFFECT_CHESS) {
        for (i = 0; i < BF_CLICK_EFFECT_MAX; ++i) {
            if (effects->items[i].active && effects->items[i].kind == 4) {
                effects->items[i].active = 0;
            }
        }
        effect = BFNextClickEffect(effects);
        effect->kind = 4;
        effect->x = x1;
        effect->y = y1;
        effect->x2 = x2;
        effect->y2 = y2;
        effect->life = BFClampInt(BF_Settings.clickEffectDuration / 3 + 8, 10, 44);
        effect->size = 2 + BFClampInt(BF_Settings.clickEffectSpeed, 1, 10) / 3;
        effect->color = secondary;
        return;
    }

    if (style == BF_EFFECT_SPACE) {
        int dx = x2 - x1;
        int dy = y2 - y1;
        for (i = 0; i < BF_CLICK_EFFECT_MAX; ++i) {
            if (effects->items[i].active && effects->items[i].kind == 5) {
                effects->items[i].active = 0;
            }
        }
        effect = BFNextClickEffect(effects);
        effect->kind = 5;
        effect->x = x1;
        effect->y = y1;
        effect->x2 = x2;
        effect->y2 = y2;
        effect->life = BFClampInt(BF_Settings.clickEffectDuration / 4 + 6, 8, 34);
        effect->size = 1 + BFClampInt(BF_Settings.clickEffectSpeed, 1, 10) / 4;
        effect->color = BFNextEffectRand(2) == 0 ? primary : secondary;
        for (i = 0; i < 5; ++i) {
            BFClickEffect *spark = BFNextClickEffect(effects);
            spark->kind = 0;
            spark->x = x2 + BFNextEffectRand(25) - 12;
            spark->y = y2 + BFNextEffectRand(21) - 10;
            spark->dx = (dx >= 0 ? -3 : 3) + BFNextEffectRand(5) - 2;
            spark->dy = (dy >= 0 ? -2 : 2) + BFNextEffectRand(5) - 2;
            spark->size = 2 + BFNextEffectRand(3);
            spark->life = BFClampInt(BF_Settings.clickEffectDuration / 4 + 4, 6, 28);
            spark->color = i % 2 == 0 ? secondary : primary;
        }
        return;
    }

    effect = BFNextClickEffect(effects);
    effect->kind = 1;
    effect->x = x1;
    effect->y = y2;
    effect->x2 = x2;
    effect->y2 = y2;
    if (effect->x == effect->x2) {
        effect->x -= 10;
        effect->x2 += 10;
    }
    effect->life = BFClampInt(BF_Settings.clickEffectDuration / 8 + 3, 4, 14);
    effect->size = 1 + BFClampInt(BF_Settings.clickEffectSpeed, 1, 10) / 4;
    effect->color = primary;
    if (BF_Settings.clickEffectStyle == BF_EFFECT_SPACE && BFNextEffectRand(2) == 0) {
        effect->color = secondary;
    }
}

int BFStepClickEffects(BFClickEffects *effects)
{
    int i;
    int changed = 0;
    int activeAfter = 0;
    int steps;
    ULONGLONG now;
    ULONGLONG elapsed;

    if (effects == NULL) {
        return 0;
    }
    if (!BF_Settings.clickEffect) {
        BFClearClickEffects(effects);
        return 0;
    }

    now = GetTickCount64();
    if (effects->lastStepMs == 0) {
        effects->lastStepMs = now;
    }
    elapsed = now - effects->lastStepMs;
    steps = (int)(elapsed / BF_EFFECT_TIMER_MS);
    if (steps < 1) {
        return 0;
    }
    if (steps > 8) {
        steps = 8;
    }
    effects->lastStepMs = now;

    for (i = 0; i < BF_CLICK_EFFECT_MAX; ++i) {
        if (effects->items[i].active) {
            changed = 1;
            effects->items[i].age += steps;
            if (effects->items[i].kind == 0 || effects->items[i].kind == 6) {
                effects->items[i].x += BFScaledVelocity(effects->items[i].dx) * steps;
                effects->items[i].y += BFScaledVelocity(effects->items[i].dy) * steps;
            }
            if (effects->items[i].age > effects->items[i].life) {
                effects->items[i].active = 0;
            }
            if (effects->items[i].active) {
                activeAfter = 1;
            }
        }
    }
    if (!activeAfter) {
        effects->lastStepMs = 0;
    }

    return changed;
}

void BFClearClickEffects(BFClickEffects *effects)
{
    if (effects == NULL) {
        return;
    }
    ZeroMemory(effects->items, sizeof(effects->items));
    effects->lastStepMs = GetTickCount64();
}

static void BFDrawEffectPixel(HDC dc, RECT rect, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &rect, brush);
    DeleteObject(brush);
}

static void BFDrawEffectParticle(HDC dc, const BFClickEffect *effect, COLORREF color)
{
    int style = BFClampInt(BF_Settings.clickEffectStyle, 0, BF_EFFECT_COUNT - 1);
    RECT rect;
    int size = BFMaxInt(2, effect->size);

    rect.left = effect->x - size;
    rect.top = effect->y - size;
    rect.right = effect->x + size;
    rect.bottom = effect->y + size;

    if (style == BF_EFFECT_PIXEL) {
        rect.left = effect->x - size;
        rect.top = effect->y - size;
        rect.right = rect.left + size * 2;
        rect.bottom = rect.top + size * 2;
        BFDrawEffectPixel(dc, rect, color);
    } else {
        HBRUSH brush = CreateSolidBrush(color);
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HGDIOBJ oldBrush = SelectObject(dc, brush);
        HGDIOBJ oldPen = SelectObject(dc, pen);
        Ellipse(dc, rect.left, rect.top, rect.right, rect.bottom);
        SelectObject(dc, oldPen);
        SelectObject(dc, oldBrush);
        DeleteObject(pen);
        DeleteObject(brush);
    }
}

static void BFDrawBadukStone(HDC dc, const BFClickEffect *effect, COLORREF color)
{
    RECT stone;
    RECT shine;
    int size = BFMaxInt(8, effect->size);
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 2, effect->extra == 0 ? RGB(0, 0, 0) : RGB(75, 68, 55));
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);

    stone.left = effect->x - size;
    stone.top = effect->y - size;
    stone.right = effect->x + size;
    stone.bottom = effect->y + size;
    Ellipse(dc, stone.left, stone.top, stone.right, stone.bottom);

    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);

    shine.left = effect->x - size / 3;
    shine.top = effect->y - size / 3;
    shine.right = shine.left + BFMaxInt(3, size / 4);
    shine.bottom = shine.top + BFMaxInt(3, size / 4);
    BFFillRectColor(dc, &shine, effect->extra == 0 ? RGB(64, 64, 62) : RGB(255, 252, 240));
}

static void BFDrawChessPiece(HDC dc, const BFClickEffect *effect, COLORREF color)
{
    static const wchar_t *pieces[12] = {L"♔", L"♕", L"♖", L"♗", L"♘", L"♙", L"♚", L"♛", L"♜", L"♝", L"♞", L"♟"};
    RECT rect;
    HFONT font;
    HGDIOBJ oldFont;
    int oldMode;
    COLORREF oldColor;
    int index = BFClampInt(effect->extra, 0, 11);
    int size = BFMaxInt(24, effect->size);

    rect.left = effect->x - size;
    rect.top = effect->y - size;
    rect.right = effect->x + size;
    rect.bottom = effect->y + size;
    font = CreateFontW(-size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
    oldFont = SelectObject(dc, font != NULL ? font : GetStockObject(DEFAULT_GUI_FONT));
    oldMode = SetBkMode(dc, TRANSPARENT);
    oldColor = SetTextColor(dc, color);
    DrawTextW(dc, pieces[index], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SetTextColor(dc, oldColor);
    SetBkMode(dc, oldMode);
    SelectObject(dc, oldFont);
    if (font != NULL) {
        DeleteObject(font);
    }
}

static void BFDrawNotePiece(HDC dc, const BFClickEffect *effect, COLORREF color)
{
    static const wchar_t *notes[6] = {L"♪", L"♫", L"♬", L"♩", L"♭", L"♯"};
    RECT rect;
    HFONT font;
    HGDIOBJ oldFont;
    int oldMode;
    COLORREF oldColor;
    int size = BFMaxInt(18, effect->size);

    rect.left = effect->x - size;
    rect.top = effect->y - size;
    rect.right = effect->x + size;
    rect.bottom = effect->y + size;
    font = CreateFontW(-size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
    oldFont = SelectObject(dc, font != NULL ? font : GetStockObject(DEFAULT_GUI_FONT));
    oldMode = SetBkMode(dc, TRANSPARENT);
    oldColor = SetTextColor(dc, color);
    DrawTextW(dc, notes[BFClampInt(effect->extra, 0, 5)], -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SetTextColor(dc, oldColor);
    SetBkMode(dc, oldMode);
    SelectObject(dc, oldFont);
    if (font != NULL) {
        DeleteObject(font);
    }
}

static void BFDrawMeteorEffect(HDC dc, const BFClickEffect *effect, COLORREF color)
{
    RECT star;
    HFONT font;
    HGDIOBJ oldFont;
    int oldMode;
    COLORREF oldColor;
    int size = BFMaxInt(2, effect->size);
    int starSize = 17 + size * 3;
    int dx = effect->x2 - effect->x;
    int dy = effect->y2 - effect->y;
    int tailX = dx >= 0 ? -16 : 16;
    int tailY = dy >= 0 ? -12 : 12;

    BFDrawLine(dc, effect->x, effect->y, effect->x2, effect->y2, color, size + 1);
    BFDrawLine(dc, effect->x + tailY / 3, effect->y - tailX / 3, effect->x2 + tailX, effect->y2 + tailY, color, 1);
    BFDrawLine(dc, effect->x - tailY / 3, effect->y + tailX / 3, effect->x2 + tailX * 2, effect->y2 + tailY * 2, color, 1);

    star.left = effect->x2 - starSize / 2;
    star.top = effect->y2 - starSize / 2;
    star.right = effect->x2 + starSize / 2;
    star.bottom = effect->y2 + starSize / 2;
    font = CreateFontW(-starSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
    oldFont = SelectObject(dc, font != NULL ? font : GetStockObject(DEFAULT_GUI_FONT));
    oldMode = SetBkMode(dc, TRANSPARENT);
    oldColor = SetTextColor(dc, color);
    DrawTextW(dc, L"★", -1, &star, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SetTextColor(dc, oldColor);
    SetBkMode(dc, oldMode);
    SelectObject(dc, oldFont);
    if (font != NULL) {
        DeleteObject(font);
    }
}

static void BFDrawArrowEffect(HDC dc, const BFClickEffect *effect, COLORREF color)
{
    int dx = effect->x2 - effect->x;
    int dy = effect->y2 - effect->y;
    int backX = effect->x2 - (dx >= 0 ? 14 : -14);
    int backY = effect->y2 - (dy >= 0 ? 14 : -14);
    int sideX = dy >= 0 ? 7 : -7;
    int sideY = dx >= 0 ? -7 : 7;

    BFDrawLine(dc, effect->x, effect->y, effect->x2, effect->y2, color, effect->size);
    BFDrawLine(dc, effect->x2, effect->y2, backX + sideX, backY + sideY, color, effect->size);
    BFDrawLine(dc, effect->x2, effect->y2, backX - sideX, backY - sideY, color, effect->size);
}

void BFDrawClickEffects(HDC dc, const BFClickEffects *effects)
{
    const BFPalette *palette = BFP();
    int i;

    if (effects == NULL || !BF_Settings.clickEffect) {
        return;
    }

    for (i = 0; i < BF_CLICK_EFFECT_MAX; ++i) {
        const BFClickEffect *effect = &effects->items[i];
        COLORREF color;
        int fade;
        int opacity;

        if (!effect->active) {
            continue;
        }

        fade = effect->life > 0 ? (effect->life - effect->age) * 100 / effect->life : 0;
        if (effect->kind == 2 || effect->kind == 3) {
            color = effect->color;
        } else {
            opacity = BFClampInt(BF_Settings.clickEffectOpacity, 10, 100) * BFClampInt(fade, 0, 100) / 100;
            color = BFBlendEffectColor(palette->bg, effect->color, opacity);
        }

        if (effect->kind == 6) {
            BFDrawNotePiece(dc, effect, color);
        } else if (effect->kind == 5) {
            BFDrawMeteorEffect(dc, effect, color);
        } else if (effect->kind == 4) {
            BFDrawArrowEffect(dc, effect, color);
        } else if (effect->kind == 3) {
            BFDrawChessPiece(dc, effect, color);
        } else if (effect->kind == 2) {
            BFDrawBadukStone(dc, effect, color);
        } else if (effect->kind == 1) {
            BFDrawLine(dc, effect->x, effect->y, effect->x2, effect->y2, color, effect->size);
        } else {
            BFDrawEffectParticle(dc, effect, color);
        }
    }
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
    fwprintf(file, L"clickEffect=%d\n", BF_Settings.clickEffect);
    fwprintf(file, L"clickEffectStyle=%d\n", BF_Settings.clickEffectStyle);
    fwprintf(file, L"clickEffectSpeed=%d\n", BF_Settings.clickEffectSpeed);
    fwprintf(file, L"clickEffectDuration=%d\n", BF_Settings.clickEffectDuration);
    fwprintf(file, L"clickEffectOpacity=%d\n", BF_Settings.clickEffectOpacity);
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
    BF_Settings.clickEffect = 1;
    BF_Settings.clickEffectStyle = BF_EFFECT_PIXEL;
    BF_Settings.clickEffectSpeed = 5;
    BF_Settings.clickEffectDuration = 30;
    BF_Settings.clickEffectOpacity = 80;
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
        } else if (wcsncmp(text, L"clickEffect=", 12) == 0) {
            BF_Settings.clickEffect = _wtoi(text + 12) != 0;
        } else if (wcsncmp(text, L"clickEffectStyle=", 17) == 0) {
            BF_Settings.clickEffectStyle = BFClampInt(_wtoi(text + 17), 0, BF_EFFECT_COUNT - 1);
        } else if (wcsncmp(text, L"clickEffectSpeed=", 17) == 0) {
            BF_Settings.clickEffectSpeed = BFClampInt(_wtoi(text + 17), 1, 10);
        } else if (wcsncmp(text, L"clickEffectDuration=", 20) == 0) {
            BF_Settings.clickEffectDuration = BFClampInt(_wtoi(text + 20), 10, 90);
        } else if (wcsncmp(text, L"clickEffectOpacity=", 19) == 0) {
            BF_Settings.clickEffectOpacity = BFClampInt(_wtoi(text + 19), 10, 100);
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
        BF_AnimTick = BFCurrentAnimationTick();
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

    if (!BFRunInstallerIfNeeded(instance)) {
        if (BF_SingletonMutex != NULL) {
            CloseHandle(BF_SingletonMutex);
            BF_SingletonMutex = NULL;
        }
        return 0;
    }

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
