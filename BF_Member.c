#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "BF_Member.h"

typedef struct BFMemberOpenData {
    int personId;
    const wchar_t *name;
    const BFMediaList *lists;
    size_t listCount;
} BFMemberOpenData;

typedef struct BFMemberState {
    int personId;
    const wchar_t *name;
    const BFMediaList *lists;
    size_t listCount;
    int selectedList;
    int showStarred;
    BFSortMode sort;
    BFFonts fonts;
    int scrollY;
    int contentHeight;
    RECT sideRect;
    RECT sortButtons[3];
    RECT volumeTrack;
    BFSideHit sideHits[BF_MAX_HITS];
    int sideHitCount;
    BFMediaHit mediaHits[BF_MAX_HITS];
    int mediaHitCount;
} BFMemberState;

static LRESULT CALLBACK BFMemberWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static int BFCollectMemberItems(const BFMemberState *state, BFMediaRef *items)
{
    int count = 0;
    size_t i;

    if (state->showStarred) {
        for (i = 0; i < state->listCount; ++i) {
            size_t j;
            for (j = 0; j < state->lists[i].count && count < BF_MAX_VISIBLE_ITEMS; ++j) {
                if (BFIsStarred(state->lists[i].items[j].id)) {
                    items[count++].item = &state->lists[i].items[j];
                }
            }
        }
        return count;
    }

    if (state->selectedList < 0 || state->selectedList >= (int)state->listCount) {
        return 0;
    }

    for (i = 0; i < state->lists[state->selectedList].count && count < BF_MAX_VISIBLE_ITEMS; ++i) {
        items[count++].item = &state->lists[state->selectedList].items[i];
    }

    return count;
}

static void BFDrawSidebar(HDC dc, BFMemberState *state)
{
    const BFPalette *palette = BFP();
    RECT rect = state->sideRect;
    RECT row;
    int y;
    size_t i;

    state->sideHitCount = 0;
    BFDrawBox(dc, rect, palette->panel, palette->border, 1);

    row.left = rect.left + 14;
    row.top = rect.top + 14;
    row.right = rect.right - 14;
    row.bottom = row.top + 34;
    BFDrawButton(dc, row, BFT(BF_TX_STARRED), state->fonts.small, state->showStarred);
    state->sideHits[state->sideHitCount].rect = row;
    state->sideHits[state->sideHitCount].listIndex = -1;
    state->sideHits[state->sideHitCount].starred = 1;
    ++state->sideHitCount;

    y = row.bottom + 18;
    row.left = rect.left + 16;
    row.top = y;
    row.right = rect.right - 16;
    row.bottom = y + 26;
    BFDrawTextBlock(dc, BFT(BF_TX_VIDEO_LIST), row, state->fonts.small, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    y = row.bottom + 8;

    for (i = 0; i < state->listCount; ++i) {
        if (state->lists[i].group != BF_LIST_VIDEO) {
            continue;
        }
        row.left = rect.left + 14;
        row.top = y;
        row.right = rect.right - 14;
        row.bottom = y + 34;
        BFDrawButton(dc, row, state->lists[i].name, state->fonts.small, !state->showStarred && state->selectedList == (int)i);
        if (state->sideHitCount < BF_MAX_HITS) {
            state->sideHits[state->sideHitCount].rect = row;
            state->sideHits[state->sideHitCount].listIndex = (int)i;
            state->sideHits[state->sideHitCount].starred = 0;
            ++state->sideHitCount;
        }
        y += 42;
    }

    y += 10;
    row.left = rect.left + 16;
    row.top = y;
    row.right = rect.right - 16;
    row.bottom = y + 26;
    BFDrawTextBlock(dc, BFT(BF_TX_SHORT_LIST), row, state->fonts.small, palette->muted, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    y = row.bottom + 8;

    for (i = 0; i < state->listCount; ++i) {
        if (state->lists[i].group != BF_LIST_SHORT) {
            continue;
        }
        row.left = rect.left + 14;
        row.top = y;
        row.right = rect.right - 14;
        row.bottom = y + 34;
        BFDrawButton(dc, row, state->lists[i].name, state->fonts.small, !state->showStarred && state->selectedList == (int)i);
        if (state->sideHitCount < BF_MAX_HITS) {
            state->sideHits[state->sideHitCount].rect = row;
            state->sideHits[state->sideHitCount].listIndex = (int)i;
            state->sideHits[state->sideHitCount].starred = 0;
            ++state->sideHitCount;
        }
        y += 42;
    }
}

static void BFDrawMemberContent(HWND hwnd, HDC dc, const RECT *client, BFMemberState *state)
{
    const BFPalette *palette = BFP();
    BFMediaRef items[BF_MAX_VISIBLE_ITEMS];
    RECT side;
    RECT title;
    RECT bar;
    RECT area;
    int count;

    (void)hwnd;
    side.right = client->right - BF_MARGIN;
    side.left = side.right - 286;
    side.top = BF_CONTENT_TOP;
    side.bottom = client->bottom - BF_MARGIN;
    state->sideRect = side;

    title.left = BF_MARGIN;
    title.top = BF_CONTENT_TOP;
    title.right = side.left - 18;
    title.bottom = title.top + 34;
    if (state->showStarred) {
        BFDrawTextBlock(dc, BFT(BF_TX_STARRED), title, state->fonts.ui, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    } else if (state->selectedList >= 0 && state->selectedList < (int)state->listCount) {
        BFDrawTextBlock(dc, state->lists[state->selectedList].name, title, state->fonts.ui, palette->accent, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    bar.left = BF_MARGIN;
    bar.top = title.bottom + 8;
    bar.right = side.left - 18;
    bar.bottom = bar.top + 34;
    BFDrawSortBar(dc, bar, state->fonts.small, state->sortButtons, &state->volumeTrack, state->sort);

    area.left = BF_MARGIN;
    area.top = bar.bottom + 18;
    area.right = side.left - 18;
    area.bottom = client->bottom - BF_MARGIN;

    count = BFCollectMemberItems(state, items);
    BFSortMediaRefs(items, count, state->sort);
    BFDrawMediaGrid(dc, area, &state->fonts, items, count, state->scrollY, &state->contentHeight, state->mediaHits, &state->mediaHitCount);
    BFDrawSidebar(dc, state);
}

static void BFSetMemberScroll(HWND hwnd, BFMemberState *state, int value)
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

static void BFHandleMemberClick(HWND hwnd, BFMemberState *state, int x, int y)
{
    int i;

    for (i = 0; i < state->sideHitCount; ++i) {
        if (BFPointInRect(&state->sideHits[i].rect, x, y)) {
            state->showStarred = state->sideHits[i].starred;
            if (!state->showStarred) {
                state->selectedList = state->sideHits[i].listIndex;
            }
            state->scrollY = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            return;
        }
    }
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

int BFRegisterMemberWindow(HINSTANCE instance)
{
    WNDCLASSEXW cls;
    ZeroMemory(&cls, sizeof(cls));
    cls.cbSize = sizeof(cls);
    cls.lpfnWndProc = BFMemberWindowProc;
    cls.hInstance = instance;
    cls.hCursor = LoadCursorW(NULL, IDC_ARROW);
    cls.lpszClassName = BF_MEMBER_CLASS;
    return RegisterClassExW(&cls) != 0;
}

void BFOpenMemberWindow(HWND parent, int personId)
{
    BFMemberOpenData data;
    HWND *slot = personId == 0 ? &BF_CheonWindow : &BF_CioWindow;
    const wchar_t *name = personId == 0 ? L"천" : L"씨오";
    const wchar_t *title = personId == 0 ? L"BlackFix - 천" : L"BlackFix - 씨오";

    if (*slot != NULL && IsWindow(*slot)) {
        ShowWindow(*slot, SW_SHOWNORMAL);
        SetForegroundWindow(*slot);
        return;
    }

    data.personId = personId;
    data.name = name;
    data.lists = personId == 0 ? BF_CHEON_LISTS : BF_CIO_LISTS;
    data.listCount = personId == 0 ? BF_CHEON_LIST_COUNT : BF_CIO_LIST_COUNT;

    *slot = CreateWindowExW(WS_EX_APPWINDOW, BF_MEMBER_CLASS, title, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1160, 760, parent, NULL, BF_Instance, &data);
    if (*slot != NULL) {
        ShowWindow(*slot, SW_SHOWNORMAL);
        UpdateWindow(*slot);
    }
}

static LRESULT CALLBACK BFMemberWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BFMemberState *state = (BFMemberState *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (message) {
    case WM_NCCREATE:
    {
        CREATESTRUCTW *create = (CREATESTRUCTW *)lParam;
        BFMemberOpenData *data = (BFMemberOpenData *)create->lpCreateParams;
        state = (BFMemberState *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*state));
        if (state == NULL) {
            return FALSE;
        }
        state->personId = data->personId;
        state->name = data->name;
        state->lists = data->lists;
        state->listCount = data->listCount;
        state->selectedList = 0;
        state->sort = BF_SORT_NEWEST;
        BFCreateFonts(&state->fonts, 22);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        return TRUE;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lParam)->ptMinTrackSize.x = 980;
        ((MINMAXINFO *)lParam)->ptMinTrackSize.y = 650;
        return 0;

    case WM_LBUTTONDOWN:
        if (state != NULL) {
            BFHandleMemberClick(hwnd, state, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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

    case WM_MOUSEWHEEL:
        if (state != NULL) {
            int delta = (short)HIWORD(wParam);
            BFSetMemberScroll(hwnd, state, state->scrollY + (delta >= 0 ? -120 : 120));
        }
        return 0;

    case WM_TIMER:
        BFHandlePreviewTimer(hwnd, wParam);
        return 0;

    case WM_SIZE:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;

    case WM_PAINT:
        if (state != NULL) {
            BFPaintBuffer paint;
            wchar_t title[128];
            if (BFBeginBufferedPaint(hwnd, &paint)) {
                swprintf(title, sizeof(title) / sizeof(title[0]), L"BlackFix :: %ls", state->name);
                BFDrawGrid(paint.memoryDc, &paint.client);
                BFDrawHeader(paint.memoryDc, &paint.client, &state->fonts, title, BFT(BF_TX_VIDEO_LIST));
                BFDrawMemberContent(hwnd, paint.memoryDc, &paint.client, state);
                BFEndBufferedPaint(hwnd, &paint);
                return 0;
            }
        }
        break;

    case WM_NCDESTROY:
        if (state != NULL) {
            if (state->personId == 0) {
                BF_CheonWindow = NULL;
            } else {
                BF_CioWindow = NULL;
            }
            BFDestroyFonts(&state->fonts);
            HeapFree(GetProcessHeap(), 0, state);
        }
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
