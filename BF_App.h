#ifndef BF_APP_H
#define BF_APP_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stddef.h>
#include "BF_Content.h"

#define BF_MAIN_CLASS L"BF_BlackFix_Window"
#define BF_ALBUM_CLASS L"BF_Album_Window"
#define BF_MEMBER_CLASS L"BF_Member_Window"
#define BF_SETTINGS_CLASS L"BF_Settings_Window"

#define BF_MARGIN 24
#define BF_HEADER_HEIGHT 92
#define BF_CONTENT_TOP 106
#define BF_GAP 14
#define BF_MEDIA_CARD_H 166
#define BF_ALBUM_CARD_H 142
#define BF_GOODS_CARD_H 156
#define BF_MAX_HITS 256
#define BF_MAX_ALBUMS 64
#define BF_MAX_STARS 512
#define BF_MAX_VISIBLE_ITEMS 512
#define BF_PATH_CAPACITY 1024
#define BF_TIMER_PREVIEW 4001
#define BF_TIMER_ANIMATION 4002
#define BF_CLICK_EFFECT_MAX 48
#define BF_CLICK_EFFECT_LIFE 18

typedef enum BFSortMode {
    BF_SORT_NEWEST,
    BF_SORT_OLDEST,
    BF_SORT_STARRED
} BFSortMode;

typedef enum BFThemeId {
    BF_THEME_TERMINAL,
    BF_THEME_DARK,
    BF_THEME_SPACE,
    BF_THEME_BOOK,
    BF_THEME_LIGHT,
    BF_THEME_DEEP,
    BF_THEME_COUNT
} BFThemeId;

typedef enum BFLanguage {
    BF_LANG_KO,
    BF_LANG_EN,
    BF_LANG_JA,
    BF_LANG_COUNT
} BFLanguage;

typedef enum BFTextKey {
    BF_TX_ALBUMS,
    BF_TX_SONGS,
    BF_TX_GOODS,
    BF_TX_CHEON,
    BF_TX_CIO,
    BF_TX_PIXEL,
    BF_TX_ALBUM_LIST,
    BF_TX_ALL_SONGS,
    BF_TX_GOODS_TITLE,
    BF_TX_LATEST,
    BF_TX_OLDEST,
    BF_TX_STAR_SORT,
    BF_TX_STARRED,
    BF_TX_VIDEO_LIST,
    BF_TX_SHORT_LIST,
    BF_TX_PREVIEW,
    BF_TX_LINK,
    BF_TX_VOLUME,
    BF_TX_SETTINGS,
    BF_TX_SCREEN,
    BF_TX_CLICK_EFFECT,
    BF_TX_THEME,
    BF_TX_SOUND,
    BF_TX_LANGUAGE,
    BF_TX_NO_ITEMS,
    BF_TX_THEME_TERMINAL,
    BF_TX_THEME_DARK,
    BF_TX_THEME_SPACE,
    BF_TX_THEME_BOOK,
    BF_TX_THEME_LIGHT,
    BF_TX_THEME_DEEP,
    BF_TX_LANG_KO,
    BF_TX_LANG_EN,
    BF_TX_LANG_JA,
    BF_TX_PREVIEW_MISSING,
    BF_TX_PREVIEW_FAIL,
    BF_TX_PREVIEW_RUNNING,
    BF_TX_COUNT
} BFTextKey;

typedef struct BFPalette {
    COLORREF bg;
    COLORREF panel;
    COLORREF panelAlt;
    COLORREF grid;
    COLORREF text;
    COLORREF muted;
    COLORREF accent;
    COLORREF border;
    COLORREF selected;
    COLORREF pressed;
} BFPalette;

typedef struct BFAppSettings {
    int theme;
    int sound;
    int language;
    int volume;
    int clickEffect;
} BFAppSettings;

typedef struct BFDragScroll {
    int active;
    int moved;
    int startX;
    int startY;
    int startScroll;
} BFDragScroll;

typedef struct BFClickEffect {
    int active;
    int x;
    int y;
    int age;
} BFClickEffect;

typedef struct BFClickEffects {
    BFClickEffect items[BF_CLICK_EFFECT_MAX];
    int next;
} BFClickEffects;

typedef struct BFFonts {
    HFONT title;
    HFONT ui;
    HFONT small;
} BFFonts;

typedef struct BFPaintBuffer {
    PAINTSTRUCT ps;
    HDC windowDc;
    HDC memoryDc;
    HBITMAP bitmap;
    HGDIOBJ oldBitmap;
    RECT client;
} BFPaintBuffer;

typedef struct BFMediaHit {
    RECT body;
    RECT star;
    RECT preview;
    RECT link;
    const BFMediaItem *item;
} BFMediaHit;

typedef struct BFGoodsHit {
    RECT body;
    const BFGoods *goods;
} BFGoodsHit;

typedef struct BFAlbumHit {
    RECT body;
    size_t index;
} BFAlbumHit;

typedef struct BFSideHit {
    RECT rect;
    int listIndex;
    int starred;
} BFSideHit;

typedef struct BFMediaRef {
    const BFMediaItem *item;
} BFMediaRef;

extern HINSTANCE BF_Instance;
extern HWND BF_MainWindow;
extern HWND BF_CheonWindow;
extern HWND BF_CioWindow;
extern HWND BF_SettingsWindow;
extern HWND BF_AlbumWindows[BF_MAX_ALBUMS];
extern BFAppSettings BF_Settings;
extern int BF_AnimTick;
extern HWND BF_VolumeCaptureWindow;

int BFRunApplication(HINSTANCE instance, int showCommand);
int BFRegisterAppWindows(HINSTANCE instance);

int BFRegisterBlackFixWindow(HINSTANCE instance);
int BFRegisterAlbumWindow(HINSTANCE instance);
int BFRegisterMemberWindow(HINSTANCE instance);
int BFRegisterSettingsWindow(HINSTANCE instance);

void BFOpenBlackFixWindow(HINSTANCE instance, int showCommand);
void BFOpenAlbumWindow(HWND parent, size_t albumIndex);
void BFOpenMemberWindow(HWND parent, int personId);
void BFOpenSettingsWindow(HWND parent);
void BFToggleSettingsWindow(HWND parent);
void BFSettingsSyncControls(void);

const wchar_t *BFT(BFTextKey key);
const BFPalette *BFP(void);
int BFClampInt(int value, int minimum, int maximum);
int BFMaxInt(int a, int b);
int BFMinInt(int a, int b);
int BFPointInRect(const RECT *rect, int x, int y);
int BFHasText(const wchar_t *text);
void BFCopyString(wchar_t *target, size_t capacity, const wchar_t *source);
void BFAppendString(wchar_t *target, size_t capacity, const wchar_t *source);

void BFCreateFonts(BFFonts *fonts, int titlePoint);
void BFDestroyFonts(BFFonts *fonts);
int BFBeginBufferedPaint(HWND hwnd, BFPaintBuffer *buffer);
void BFEndBufferedPaint(HWND hwnd, BFPaintBuffer *buffer);
void BFDrawGrid(HDC dc, const RECT *client);
void BFDrawHeader(HDC dc, const RECT *client, const BFFonts *fonts, const wchar_t *title, const wchar_t *subtitle);
void BFDrawButton(HDC dc, RECT rect, const wchar_t *text, HFONT font, int selected);
void BFDrawTextBlock(HDC dc, const wchar_t *text, RECT rect, HFONT font, COLORREF color, UINT format);
void BFFillRectColor(HDC dc, const RECT *rect, COLORREF color);
void BFDrawBox(HDC dc, RECT rect, COLORREF fill, COLORREF border, int width);
void BFDrawLine(HDC dc, int x1, int y1, int x2, int y2, COLORREF color, int width);
void BFDrawVolume(HDC dc, RECT bounds, HFONT font, RECT *trackOut);
void BFSetVolumeFromTrack(RECT track, int x);
void BFBeginDragScroll(BFDragScroll *drag, int x, int y, int scrollY);
int BFUpdateDragScroll(BFDragScroll *drag, int x, int y, int *scrollY);
int BFEndDragScroll(BFDragScroll *drag);
void BFAddClickEffect(BFClickEffects *effects, int x, int y);
int BFStepClickEffects(BFClickEffects *effects);
void BFDrawClickEffects(HDC dc, const BFClickEffects *effects);

int BFColumnCount(int width);
void BFDrawSortBar(HDC dc, RECT bounds, HFONT font, RECT sortButtons[3], RECT *volumeTrack, BFSortMode sort);
int BFCompareMedia(const BFMediaItem *a, const BFMediaItem *b, BFSortMode sort);
void BFSortMediaRefs(BFMediaRef *items, int count, BFSortMode sort);
int BFIsStarred(const wchar_t *id);
unsigned int BFStarOrder(const wchar_t *id);
void BFToggleStar(const wchar_t *id);
void BFSaveState(void);
void BFLoadState(void);
void BFInvalidateAllWindows(void);

void BFOpenLink(HWND hwnd, const wchar_t *link);
void BFStartPreview(HWND hwnd, const BFMediaItem *item);
void BFStopPreview(void);
int BFIsPreviewing(const BFMediaItem *item);
void BFHandlePreviewTimer(HWND hwnd, WPARAM timerId);
void BFHandleMediaClick(HWND hwnd, BFMediaHit *hits, int hitCount, int x, int y);
void BFDrawMediaGrid(HDC dc, RECT area, const BFFonts *fonts, const BFMediaRef *items, int count, int scrollY, int *contentHeight, BFMediaHit *hits, int *hitCount);
void BFDrawGoodsGrid(HDC dc, RECT area, const BFFonts *fonts, int scrollY, int *contentHeight, BFGoodsHit *hits, int *hitCount);

#endif
