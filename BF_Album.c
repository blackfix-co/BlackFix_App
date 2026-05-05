#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "BF_Album.h"

typedef struct BFAlbumState {
    size_t albumIndex;
    BFSortMode sort;
    BFFonts fonts;
    int scrollY;
    int contentHeight;
    RECT sortButtons[3];
    RECT volumeTrack;
    BFMediaHit mediaHits[BF_MAX_HITS];
    int mediaHitCount;
    BFDragScroll drag;
    BFClickEffects effects;
} BFAlbumState;

static LRESULT CALLBACK BFAlbumWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static int BFCollectAlbumSongs(const BFAlbum *album, BFMediaRef *items)
{
    int count = 0;
    size_t i;

    for (i = 0; i < BF_SONG_COUNT && count < BF_MAX_VISIBLE_ITEMS; ++i) {
        if (wcscmp(BF_SONGS[i].albumId, album->id) == 0) {
            items[count++].item = &BF_SONGS[i];
        }
    }

    return count;
}

static void BFDrawAlbumContent(HWND hwnd, HDC dc, const RECT *client, BFAlbumState *state)
{
    const BFPalette *palette = BFP();
    const BFAlbum *album = &BF_ALBUMS[state->albumIndex];
    BFMediaRef items[BF_MAX_VISIBLE_ITEMS];
    RECT title;
    RECT bar;
    RECT area;
    int count;

    (void)hwnd;
    title.left = BF_MARGIN;
    title.top = BF_CONTENT_TOP;
    title.right = client->right - BF_MARGIN;
    title.bottom = title.top + 34;
    BFDrawTextBlock(dc, album->title, title, state->fonts.ui, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    bar.left = BF_MARGIN;
    bar.top = title.bottom + 8;
    bar.right = client->right - BF_MARGIN;
    bar.bottom = bar.top + 34;
    BFDrawSortBar(dc, bar, state->fonts.small, state->sortButtons, &state->volumeTrack, state->sort);

    area.left = BF_MARGIN;
    area.top = bar.bottom + 18;
    area.right = client->right - BF_MARGIN;
    area.bottom = client->bottom - BF_MARGIN;

    count = BFCollectAlbumSongs(album, items);
    BFSortMediaRefs(items, count, state->sort);
    BFDrawMediaGrid(dc, area, &state->fonts, items, count, state->scrollY, &state->contentHeight, state->mediaHits, &state->mediaHitCount);
}

static void BFSetAlbumScroll(HWND hwnd, BFAlbumState *state, int value)
{
    RECT client;
    int pageHeight;
    int maxScroll;

    GetClientRect(hwnd, &client);
    pageHeight = BFMaxInt(1, client.bottom - BF_CONTENT_TOP - 92);
    maxScroll = BFMaxInt(0, state->contentHeight - pageHeight);
    state->scrollY = BFClampInt(value, 0, maxScroll);
    InvalidateRect(hwnd, NULL, FALSE);
}

static void BFHandleAlbumClick(HWND hwnd, BFAlbumState *state, int x, int y)
{
    int i;

    for (i = 0; i < 3; ++i) {
        if (BFPointInRect(&state->sortButtons[i], x, y)) {
            state->sort = (BFSortMode)i;
            state->scrollY = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
    }
    if (!IsRectEmpty(&state->volumeTrack) && BFPointInRect(&state->volumeTrack, x, y)) {
        BFSetVolumeFromTrack(state->volumeTrack, x);
        BF_VolumeCaptureWindow = hwnd;
        SetCapture(hwnd);
        return;
    }
    BFHandleMediaClick(hwnd, state->mediaHits, state->mediaHitCount, x, y);
}

int BFRegisterAlbumWindow(HINSTANCE instance)
{
    WNDCLASSEXW cls;
    ZeroMemory(&cls, sizeof(cls));
    cls.cbSize = sizeof(cls);
    cls.lpfnWndProc = BFAlbumWindowProc;
    cls.hInstance = instance;
    cls.hCursor = LoadCursorW(NULL, IDC_ARROW);
    cls.lpszClassName = BF_ALBUM_CLASS;
    return RegisterClassExW(&cls) != 0;
}

void BFOpenAlbumWindow(HWND parent, size_t albumIndex)
{
    wchar_t title[192];
    if (albumIndex >= BF_ALBUM_COUNT || albumIndex >= BF_MAX_ALBUMS) {
        return;
    }
    if (BF_AlbumWindows[albumIndex] != NULL && IsWindow(BF_AlbumWindows[albumIndex])) {
        ShowWindow(BF_AlbumWindows[albumIndex], SW_SHOWNORMAL);
        SetForegroundWindow(BF_AlbumWindows[albumIndex]);
        return;
    }
    swprintf(title, sizeof(title) / sizeof(title[0]), L"BlackFix - %ls", BF_ALBUMS[albumIndex].title);
    BF_AlbumWindows[albumIndex] = CreateWindowExW(WS_EX_APPWINDOW, BF_ALBUM_CLASS, title, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1060, 720, parent, NULL, BF_Instance, (LPVOID)(albumIndex + 1));
    if (BF_AlbumWindows[albumIndex] != NULL) {
        ShowWindow(BF_AlbumWindows[albumIndex], SW_SHOWNORMAL);
        UpdateWindow(BF_AlbumWindows[albumIndex]);
    }
}

static LRESULT CALLBACK BFAlbumWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BFAlbumState *state = (BFAlbumState *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (message) {
    case WM_NCCREATE:
    {
        CREATESTRUCTW *create = (CREATESTRUCTW *)lParam;
        size_t albumIndex = (size_t)create->lpCreateParams - 1;
        state = (BFAlbumState *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*state));
        if (state == NULL || albumIndex >= BF_ALBUM_COUNT) {
            return FALSE;
        }
        state->albumIndex = albumIndex;
        state->sort = BF_SORT_NEWEST;
        BFCreateFonts(&state->fonts, 22);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        return TRUE;
    }

    case WM_CREATE:
        SetTimer(hwnd, BF_TIMER_ANIMATION, BF_EFFECT_TIMER_MS, NULL);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lParam)->ptMinTrackSize.x = 860;
        ((MINMAXINFO *)lParam)->ptMinTrackSize.y = 620;
        return 0;

    case WM_LBUTTONDOWN:
        if (state != NULL) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            BFAddClickEffect(&state->effects, x, y);
            if (!IsRectEmpty(&state->volumeTrack) && BFPointInRect(&state->volumeTrack, x, y)) {
                BFSetVolumeFromTrack(state->volumeTrack, x);
                BF_VolumeCaptureWindow = hwnd;
                SetCapture(hwnd);
                return 0;
            }
            BFBeginDragScroll(&state->drag, x, y, state->scrollY);
            SetCapture(hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;

    case WM_MOUSEMOVE:
        if (state != NULL && BF_VolumeCaptureWindow == hwnd && (wParam & MK_LBUTTON) != 0 && !IsRectEmpty(&state->volumeTrack)) {
            BFSetVolumeFromTrack(state->volumeTrack, GET_X_LPARAM(lParam));
            return 0;
        }
        if (state != NULL && state->drag.active && (wParam & MK_LBUTTON) != 0) {
            int next = state->scrollY;
            int oldX = state->drag.lastX;
            int oldY = state->drag.lastY;
            if (BFUpdateDragScroll(&state->drag, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &next)) {
                BFSetAlbumScroll(hwnd, state, next);
            }
            if (state->drag.moved && (oldX != state->drag.lastX || oldY != state->drag.lastY)) {
                BFAddDragEffect(&state->effects, oldX, oldY, state->drag.lastX, state->drag.lastY);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        }
        break;

    case WM_LBUTTONUP:
        if (BF_VolumeCaptureWindow == hwnd) {
            BF_VolumeCaptureWindow = NULL;
            ReleaseCapture();
            return 0;
        }
        if (state != NULL && state->drag.active) {
            int click = BFEndDragScroll(&state->drag);
            ReleaseCapture();
            if (click) {
                BFHandleAlbumClick(hwnd, state, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            }
            return 0;
        }
        break;

    case WM_MOUSEWHEEL:
        if (state != NULL) {
            int delta = (short)HIWORD(wParam);
            BFSetAlbumScroll(hwnd, state, state->scrollY + (delta >= 0 ? -120 : 120));
        }
        return 0;

    case WM_TIMER:
        if (state != NULL && wParam == BF_TIMER_ANIMATION) {
            BFStepClickEffects(&state->effects);
            BFHandlePreviewTimer(hwnd, wParam);
            return 0;
        }
        BFHandlePreviewTimer(hwnd, wParam);
        return 0;

    case WM_SIZE:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_PAINT:
        if (state != NULL) {
            BFPaintBuffer paint;
            wchar_t title[192];
            if (BFBeginBufferedPaint(hwnd, &paint)) {
                swprintf(title, sizeof(title) / sizeof(title[0]), L"BlackFix :: %ls", BF_ALBUMS[state->albumIndex].title);
                BFDrawGrid(paint.memoryDc, &paint.client);
                BFDrawHeader(paint.memoryDc, &paint.client, &state->fonts, title, BF_ALBUMS[state->albumIndex].subtitle);
                BFDrawAlbumContent(hwnd, paint.memoryDc, &paint.client, state);
                BFDrawClickEffects(paint.memoryDc, &state->effects);
                BFEndBufferedPaint(hwnd, &paint);
                return 0;
            }
        }
        break;

    case WM_NCDESTROY:
        if (state != NULL) {
            if (state->albumIndex < BF_MAX_ALBUMS) {
                BF_AlbumWindows[state->albumIndex] = NULL;
            }
            BFDestroyFonts(&state->fonts);
            HeapFree(GetProcessHeap(), 0, state);
        }
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
