#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "BF_BlackFix.h"
#include "BF_Album.h"
#include "BF_Member.h"

typedef enum BFBlackFixView {
    BF_BLACKFIX_ALBUMS,
    BF_BLACKFIX_SONGS,
    BF_BLACKFIX_GOODS
} BFBlackFixView;

typedef struct BFBlackFixState {
    BFBlackFixView view;
    BFSortMode sort;
    BFFonts fonts;
    int scrollY;
    int contentHeight;
    RECT buttons[5];
    RECT sortButtons[3];
    RECT volumeTrack;
    BFAlbumHit albumHits[BF_MAX_HITS];
    int albumHitCount;
    BFMediaHit mediaHits[BF_MAX_HITS];
    int mediaHitCount;
    BFGoodsHit goodsHits[BF_MAX_HITS];
    int goodsHitCount;
    BFDragScroll drag;
    BFClickEffects effects;
} BFBlackFixState;

static LRESULT CALLBACK BFBlackFixWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static void BFDrawAlbumCard(HDC dc, RECT card, const BFFonts *fonts, const BFAlbum *album)
{
    const BFPalette *palette = BFP();
    RECT title;
    RECT meta;
    RECT release;
    RECT desc;
    RECT accent;

    BFDrawBox(dc, card, palette->panel, palette->border, 1);

    accent.left = card.left;
    accent.top = card.top;
    accent.right = card.left + 6;
    accent.bottom = card.bottom;
    BFFillRectColor(dc, &accent, palette->selected);

    title.left = card.left + 18;
    title.top = card.top + 14;
    title.right = card.right - 18;
    title.bottom = card.top + 44;
    BFDrawTextBlock(dc, album->title, title, fonts->ui, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    release.left = card.right - 112;
    release.top = card.top + 45;
    release.right = card.right - 18;
    release.bottom = card.top + 72;
    BFDrawTextBlock(dc, album->release, release, fonts->small, palette->muted, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    meta.left = card.left + 18;
    meta.top = card.top + 45;
    meta.right = release.left - 10;
    meta.bottom = card.top + 72;
    BFDrawTextBlock(dc, album->subtitle, meta, fonts->small, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    desc.left = card.left + 18;
    desc.top = card.top + 78;
    desc.right = card.right - 18;
    desc.bottom = card.bottom - 16;
    BFDrawTextBlock(dc, album->description, desc, fonts->small, palette->text, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);
}

static void BFDrawAlbumGrid(HDC dc, RECT area, const BFFonts *fonts, int scrollY, int *contentHeight, BFAlbumHit *hits, int *hitCount)
{
    int columns = BFColumnCount(area.right - area.left);
    int cardWidth = (area.right - area.left - (columns - 1) * BF_GAP) / columns;
    int rows = columns > 0 ? ((int)BF_ALBUM_COUNT + columns - 1) / columns : 0;
    int startY = area.top - scrollY;
    int i;
    HRGN region;

    *hitCount = 0;
    *contentHeight = rows == 0 ? 90 : rows * BF_ALBUM_CARD_H + BFMaxInt(0, rows - 1) * BF_GAP + 24;
    region = CreateRectRgn(area.left, area.top, area.right, area.bottom);
    SelectClipRgn(dc, region);

    for (i = 0; i < (int)BF_ALBUM_COUNT; ++i) {
        int row = i / columns;
        int column = i % columns;
        RECT card;

        card.left = area.left + column * (cardWidth + BF_GAP);
        card.top = startY + row * (BF_ALBUM_CARD_H + BF_GAP);
        card.right = card.left + cardWidth;
        card.bottom = card.top + BF_ALBUM_CARD_H;

        if (card.bottom < area.top || card.top > area.bottom) {
            continue;
        }

        BFDrawAlbumCard(dc, card, fonts, &BF_ALBUMS[i]);
        if (*hitCount < BF_MAX_HITS) {
            hits[*hitCount].body = card;
            hits[*hitCount].index = (size_t)i;
            ++(*hitCount);
        }
    }

    SelectClipRgn(dc, NULL);
    DeleteObject(region);
}

static int BFCollectSongs(BFMediaRef *items)
{
    int count = 0;
    size_t i;

    for (i = 0; i < BF_SONG_COUNT && count < BF_MAX_VISIBLE_ITEMS; ++i) {
        items[count++].item = &BF_SONGS[i];
    }

    return count;
}

static void BFDrawTopButtons(HDC dc, const RECT *client, BFBlackFixState *state)
{
    int widths[5] = {76, 76, 76, 68, 68};
    int gap = 10;
    int total = widths[0] + widths[1] + widths[2] + widths[3] + widths[4] + gap * 4;
    int x = client->right - BF_MARGIN - total;
    int y = 32;
    int i;

    for (i = 0; i < 5; ++i) {
        state->buttons[i].left = x;
        state->buttons[i].top = y;
        state->buttons[i].right = x + widths[i];
        state->buttons[i].bottom = y + 38;
        x += widths[i] + gap;
    }

    BFDrawButton(dc, state->buttons[0], BFT(BF_TX_ALBUMS), state->fonts.small, state->view == BF_BLACKFIX_ALBUMS);
    BFDrawButton(dc, state->buttons[1], BFT(BF_TX_SONGS), state->fonts.small, state->view == BF_BLACKFIX_SONGS);
    BFDrawButton(dc, state->buttons[2], BFT(BF_TX_GOODS), state->fonts.small, state->view == BF_BLACKFIX_GOODS);
    BFDrawButton(dc, state->buttons[3], BFT(BF_TX_CHEON), state->fonts.small, 0);
    BFDrawButton(dc, state->buttons[4], BFT(BF_TX_CIO), state->fonts.small, 0);
}

static void BFDrawBlackFixContent(HWND hwnd, HDC dc, const RECT *client, BFBlackFixState *state)
{
    const BFPalette *palette = BFP();
    RECT title;
    RECT bar;
    RECT area;
    RECT volumeBounds;

    (void)hwnd;
    title.left = BF_MARGIN;
    title.top = BF_CONTENT_TOP;
    title.right = client->right - BF_MARGIN;
    title.bottom = title.top + 34;

    bar.left = BF_MARGIN;
    bar.top = title.bottom + 8;
    bar.right = client->right - BF_MARGIN;
    bar.bottom = bar.top + 34;

    area.left = BF_MARGIN;
    area.top = bar.bottom + 18;
    area.right = client->right - BF_MARGIN;
    area.bottom = client->bottom - BF_MARGIN;

    SetRectEmpty(&state->volumeTrack);
    state->mediaHitCount = 0;
    state->goodsHitCount = 0;
    state->albumHitCount = 0;

    if (state->view == BF_BLACKFIX_ALBUMS) {
        BFDrawTextBlock(dc, BFT(BF_TX_ALBUM_LIST), title, state->fonts.ui, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        volumeBounds.left = BFMaxInt(BF_MARGIN, client->right - BF_MARGIN - 260);
        volumeBounds.top = bar.top;
        volumeBounds.right = client->right - BF_MARGIN;
        volumeBounds.bottom = bar.bottom;
        BFDrawVolume(dc, volumeBounds, state->fonts.small, &state->volumeTrack);
        BFDrawAlbumGrid(dc, area, &state->fonts, state->scrollY, &state->contentHeight, state->albumHits, &state->albumHitCount);
    } else if (state->view == BF_BLACKFIX_SONGS) {
        BFMediaRef items[BF_MAX_VISIBLE_ITEMS];
        int count = BFCollectSongs(items);
        BFDrawTextBlock(dc, BFT(BF_TX_ALL_SONGS), title, state->fonts.ui, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        BFDrawSortBar(dc, bar, state->fonts.small, state->sortButtons, &state->volumeTrack, state->sort);
        BFSortMediaRefs(items, count, state->sort);
        BFDrawMediaGrid(dc, area, &state->fonts, items, count, state->scrollY, &state->contentHeight, state->mediaHits, &state->mediaHitCount);
    } else {
        BFDrawTextBlock(dc, BFT(BF_TX_GOODS_TITLE), title, state->fonts.ui, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        volumeBounds.left = BFMaxInt(BF_MARGIN, client->right - BF_MARGIN - 260);
        volumeBounds.top = bar.top;
        volumeBounds.right = client->right - BF_MARGIN;
        volumeBounds.bottom = bar.bottom;
        BFDrawVolume(dc, volumeBounds, state->fonts.small, &state->volumeTrack);
        BFDrawGoodsGrid(dc, area, &state->fonts, state->scrollY, &state->contentHeight, state->goodsHits, &state->goodsHitCount);
    }
}

static void BFSetScroll(HWND hwnd, BFBlackFixState *state, int value)
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

static void BFHandleClick(HWND hwnd, BFBlackFixState *state, int x, int y)
{
    int i;

    if (BFPointInRect(&state->buttons[0], x, y)) {
        state->view = BF_BLACKFIX_ALBUMS;
        state->scrollY = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    if (BFPointInRect(&state->buttons[1], x, y)) {
        state->view = BF_BLACKFIX_SONGS;
        state->scrollY = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    if (BFPointInRect(&state->buttons[2], x, y)) {
        state->view = BF_BLACKFIX_GOODS;
        state->scrollY = 0;
        InvalidateRect(hwnd, NULL, FALSE);
        return;
    }
    if (BFPointInRect(&state->buttons[3], x, y)) {
        BFOpenMemberWindow(hwnd, 0);
        return;
    }
    if (BFPointInRect(&state->buttons[4], x, y)) {
        BFOpenMemberWindow(hwnd, 1);
        return;
    }
    if (!IsRectEmpty(&state->volumeTrack) && BFPointInRect(&state->volumeTrack, x, y)) {
        BFSetVolumeFromTrack(state->volumeTrack, x);
        BF_VolumeCaptureWindow = hwnd;
        SetCapture(hwnd);
        return;
    }
    if (state->view == BF_BLACKFIX_SONGS) {
        for (i = 0; i < 3; ++i) {
            if (BFPointInRect(&state->sortButtons[i], x, y)) {
                state->sort = (BFSortMode)i;
                state->scrollY = 0;
                InvalidateRect(hwnd, NULL, FALSE);
                return;
            }
        }
        BFHandleMediaClick(hwnd, state->mediaHits, state->mediaHitCount, x, y);
    } else if (state->view == BF_BLACKFIX_ALBUMS) {
        for (i = 0; i < state->albumHitCount; ++i) {
            if (BFPointInRect(&state->albumHits[i].body, x, y)) {
                BFOpenAlbumWindow(hwnd, state->albumHits[i].index);
                return;
            }
        }
    } else {
        for (i = 0; i < state->goodsHitCount; ++i) {
            if (BFPointInRect(&state->goodsHits[i].body, x, y)) {
                BFOpenLink(hwnd, state->goodsHits[i].goods->link);
                return;
            }
        }
    }
}

int BFRegisterBlackFixWindow(HINSTANCE instance)
{
    WNDCLASSEXW cls;
    ZeroMemory(&cls, sizeof(cls));
    cls.cbSize = sizeof(cls);
    cls.lpfnWndProc = BFBlackFixWindowProc;
    cls.hInstance = instance;
    cls.hCursor = LoadCursorW(NULL, IDC_ARROW);
    cls.lpszClassName = BF_MAIN_CLASS;
    return RegisterClassExW(&cls) != 0;
}

void BFOpenBlackFixWindow(HINSTANCE instance, int showCommand)
{
    BF_MainWindow = CreateWindowExW(0, BF_MAIN_CLASS, L"BlackFix", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1180, 780, NULL, NULL, instance, NULL);
    if (BF_MainWindow != NULL) {
        ShowWindow(BF_MainWindow, showCommand);
        UpdateWindow(BF_MainWindow);
    }
}

static LRESULT CALLBACK BFBlackFixWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BFBlackFixState *state = (BFBlackFixState *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (message) {
    case WM_CREATE:
        state = (BFBlackFixState *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*state));
        if (state == NULL) {
            return -1;
        }
        state->view = BF_BLACKFIX_ALBUMS;
        state->sort = BF_SORT_NEWEST;
        BFCreateFonts(&state->fonts, 24);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        BF_MainWindow = hwnd;
        SetTimer(hwnd, BF_TIMER_ANIMATION, 80, NULL);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lParam)->ptMinTrackSize.x = 900;
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
            if (BFUpdateDragScroll(&state->drag, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), &next)) {
                BFSetScroll(hwnd, state, next);
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
                BFHandleClick(hwnd, state, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            }
            return 0;
        }
        break;

    case WM_MOUSEWHEEL:
        if (state != NULL) {
            int delta = (short)HIWORD(wParam);
            BFSetScroll(hwnd, state, state->scrollY + (delta >= 0 ? -120 : 120));
        }
        return 0;

    case WM_TIMER:
        if (state != NULL && wParam == BF_TIMER_ANIMATION) {
            if (BFStepClickEffects(&state->effects)) {
                InvalidateRect(hwnd, NULL, FALSE);
            }
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
            if (BFBeginBufferedPaint(hwnd, &paint)) {
                BFDrawGrid(paint.memoryDc, &paint.client);
                BFDrawHeader(paint.memoryDc, &paint.client, &state->fonts, L"BlackFix", BFT(BF_TX_PIXEL));
                BFDrawTopButtons(paint.memoryDc, &paint.client, state);
                BFDrawBlackFixContent(hwnd, paint.memoryDc, &paint.client, state);
                BFDrawClickEffects(paint.memoryDc, &state->effects);
                BFEndBufferedPaint(hwnd, &paint);
                return 0;
            }
        }
        break;

    case WM_DESTROY:
        BFStopPreview();
        PostQuitMessage(0);
        return 0;

    case WM_NCDESTROY:
        if (state != NULL) {
            BFDestroyFonts(&state->fonts);
            HeapFree(GetProcessHeap(), 0, state);
        }
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        BF_MainWindow = NULL;
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
