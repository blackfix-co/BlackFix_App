#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <objbase.h>
#include <stdio.h>
#include <wchar.h>
#include "BF_App.h"
#include "BF_Installer.h"
#include "BF_Resource.h"

#define BF_INSTALLER_CLASS L"BF_Installer_Window"
#define BF_INSTALL_EXE_NAME L"BlackFix_App.exe"
#define BF_SHORTCUT_NAME L"BlackFix_App.lnk"

#define BF_ID_LEGAL_EDIT 5001
#define BF_ID_AGREE 5002
#define BF_ID_NEXT 5003
#define BF_ID_CANCEL 5004
#define BF_ID_PATH_LABEL 5005
#define BF_ID_PATH_EDIT 5006
#define BF_ID_BROWSE 5007
#define BF_ID_DESKTOP 5008
#define BF_ID_BACK 5009
#define BF_ID_INSTALL 5010

typedef enum BFInstallerPage {
    BF_INSTALLER_PAGE_LEGAL,
    BF_INSTALLER_PAGE_PATH
} BFInstallerPage;

typedef struct BFInstallerState {
    HINSTANCE instance;
    HWND hwnd;
    HWND legalEdit;
    HWND agreeBox;
    HWND nextButton;
    HWND cancelButton;
    HWND pathLabel;
    HWND pathEdit;
    HWND browseButton;
    HWND desktopBox;
    HWND backButton;
    HWND installButton;
    HFONT titleFont;
    BFInstallerPage page;
    int done;
    int proceed;
    int launchInstalled;
    wchar_t installPath[BF_PATH_CAPACITY];
    wchar_t installedExe[BF_PATH_CAPACITY];
} BFInstallerState;

static LRESULT CALLBACK BFInstallerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

static const wchar_t BF_LEGAL_TEXT[] =
    L"BlackFix App 이용 전 저작권 및 콘텐츠 이용 고지\r\n"
    L"\r\n"
    L"본 고지는 대한민국 저작권법, 정보통신망을 통한 콘텐츠 이용 관행, 온라인 플랫폼의 서비스 약관을 함께 고려하여 작성되었습니다. "
    L"이 문구는 사용자가 BlackFix App을 이용하면서 앨범, 노래, 영상, 쇼츠, 굿즈 정보와 외부 링크를 다룰 때 지켜야 할 기준을 설명하기 위한 안내이며, 개별 사건에 대한 법률 자문은 아닙니다.\r\n"
    L"\r\n"
    L"1. 보호되는 저작물의 범위\r\n"
    L"대한민국 저작권법상 인간의 사상 또는 감정을 표현한 창작물은 저작물로 보호될 수 있습니다. 음악, 가사, 음반, 영상, 사진, 이미지, 일러스트, 로고, 글, 편집물, 데이터베이스, 썸네일, 자막, 방송 클립, 공연 영상, 굿즈 디자인과 같은 자료는 각각 또는 결합된 형태로 보호될 수 있습니다. "
    L"단순한 아이디어, 콘셉트, 스타일, 장르 자체는 보호 범위가 제한될 수 있으나, 실제로 고정되거나 표현된 결과물은 저작권, 저작인접권, 상표권, 초상권, 퍼블리시티 관련 권리, 부정경쟁방지 관련 이익과 함께 문제될 수 있습니다.\r\n"
    L"\r\n"
    L"2. 저작재산권자의 허락이 필요한 대표 행위\r\n"
    L"저작권법은 저작재산권으로 복제, 공연, 공중송신, 전시, 배포, 대여, 2차적저작물작성 등의 권리를 정하고 있습니다. "
    L"파일을 앱 폴더나 서버에 저장하는 행위는 복제에 해당할 수 있고, 인터넷에 올리거나 불특정 또는 다수인이 접근할 수 있게 하는 행위는 공중송신 또는 전송에 해당할 수 있습니다. "
    L"영상 일부를 잘라 쇼츠로 만들거나 음악을 배경음으로 넣거나 자막, 번역, 리믹스, 편집본, 하이라이트를 만드는 행위는 원저작물의 복제 및 2차적저작물작성과 관련될 수 있습니다. "
    L"따라서 권리자의 명시적 허락, 적법한 라이선스, 플랫폼이 허용한 공유 기능, 법에서 정한 제한 사유가 없는 경우에는 업로드, 다운로드, 배포, 재편집, 저장, 공유를 하지 않아야 합니다.\r\n"
    L"\r\n"
    L"3. 링크와 미리보기의 구분\r\n"
    L"BlackFix App은 기본적으로 사용자가 입력한 제목, 설명, 날짜, 외부 링크를 보여주고, 링크 버튼을 누르면 YouTube 등 외부 사이트로 이동하는 구조입니다. "
    L"합법적으로 공개된 페이지의 단순 링크는 일반적인 파일 업로드와 다를 수 있으나, 불법 복제물임을 알거나 알 수 있는 자료로 연결하거나, 우회 접근을 돕거나, 불법 스트리밍 또는 다운로드를 적극적으로 유도하면 법적 책임이 문제될 수 있습니다. "
    L"또한 앱 안의 1분 미리보기 기능에 로컬 음원 또는 영상 파일을 넣는 경우, 그 파일을 보유하고 재생하는 행위 자체가 적법해야 합니다. 구매, 구독, 스트리밍 시청 권한이 있다고 해서 별도의 파일 복제, 편집, 배포, 앱 내 재생 권한까지 자동으로 허락되는 것은 아닙니다.\r\n"
    L"\r\n"
    L"4. 업로드 및 직접 추가 콘텐츠 책임\r\n"
    L"사용자가 C 코드 또는 콘텐츠 파일에 앨범, 노래, 영상, 쇼츠, 굿즈 데이터를 직접 추가하는 경우, 제목, 설명, 링크, 미리보기 파일, 이미지, 아이콘, 굿즈 디자인, 로고, 멤버명 표시가 제3자의 권리를 침해하지 않는지 사용자가 확인해야 합니다. "
    L"공식 계정, 권리자, 소속사, 유통사, 플랫폼 정책상 허용된 자료만 사용해야 하며, 출처를 표시하더라도 허락 없는 복제나 공중송신이 항상 적법해지는 것은 아닙니다. "
    L"팬 제작 자료, 캡처 이미지, 방송 클립, 유료 콘텐츠, 멤버십 영상, 비공개 자료, 음원 파일, MR, 커버 영상, 로고 변형 이미지는 특히 권리 관계를 확인해야 합니다.\r\n"
    L"\r\n"
    L"5. 저작권 제한과 공정한 이용에 관한 주의\r\n"
    L"저작권법에는 인용, 사적 이용을 위한 복제, 교육 목적 이용, 시사보도 등 일정한 제한 규정이 있으나, 각 규정은 목적, 범위, 방법, 이용량, 시장 대체 가능성, 출처 표시, 비영리성 등 구체적 사정을 따집니다. "
    L"앱에 콘텐츠를 모아 배포하거나 다른 사람이 내려받을 수 있게 하는 행위는 개인적 감상 범위를 넘어설 수 있습니다. "
    L"짧은 길이의 클립, 1분 미리보기, 썸네일, 일부 가사만 사용했다는 사정만으로 자동 면책되는 것은 아니며, 상업성이 없더라도 침해가 성립할 수 있습니다.\r\n"
    L"\r\n"
    L"6. 플랫폼 약관과 삭제 요청\r\n"
    L"YouTube, 음원 플랫폼, SNS, 팬 커뮤니티 등 외부 서비스는 각자의 약관과 API/링크 정책을 둘 수 있습니다. "
    L"외부 링크가 현재 접근 가능하더라도 해당 플랫폼이 임베드, 다운로드, 자동 재생, 우회 저장, 썸네일 재사용을 허용한다는 뜻은 아닙니다. "
    L"권리자, 플랫폼, 당사자로부터 삭제 요청이나 수정 요청이 오면 사용자는 즉시 문제 콘텐츠를 제거하거나 링크를 수정해야 하며, 반복 침해가 발생하지 않도록 직접 추가한 데이터와 미리보기 파일을 관리해야 합니다.\r\n"
    L"\r\n"
    L"7. 굿즈와 이미지 권리\r\n"
    L"굿즈 명칭, 디자인, 로고, 사진, 포장 이미지, 캐릭터, 멤버 초상, 서체, 색상 조합은 저작권 외에도 상표권, 디자인권, 초상권, 부정경쟁방지 관련 권리와 연결될 수 있습니다. "
    L"정식 판매 페이지로 이동하는 링크와 단순 소개 정보는 상대적으로 위험이 낮을 수 있으나, 공식 이미지를 무단 저장하여 앱에 포함하거나, 실제 상품처럼 보이게 복제하거나, 판매를 오인하게 만드는 표시는 피해야 합니다.\r\n"
    L"\r\n"
    L"8. 책임 범위\r\n"
    L"BlackFix App은 콘텐츠 목록을 표시하고 사용자가 지정한 외부 링크 또는 로컬 미리보기 파일을 열 수 있게 하는 도구입니다. "
    L"사용자가 직접 추가한 자료의 적법성, 권리자 허락 여부, 링크 대상의 적법성, 로컬 파일 보유 및 이용 권한, 배포로 인한 책임은 사용자에게 있습니다. "
    L"법령, 판례, 플랫폼 정책은 변경될 수 있으므로 공개 배포, 팬 사이트 운영, 수익화, 광고, 행사 상영, 단체 공유, 굿즈 판매와 연결되는 사용은 권리자 허락 또는 전문가 검토를 받은 뒤 진행해야 합니다.\r\n"
    L"\r\n"
    L"아래 확인란을 선택하면, 사용자는 위 내용을 읽고 이해했으며, 본 앱에 추가하거나 연결하는 모든 콘텐츠를 적법하게 관리할 책임이 본인에게 있음을 확인한 것으로 봅니다.";

static void BFInstallerAppendPath(wchar_t *path, size_t capacity, const wchar_t *part)
{
    size_t length;
    if (path == NULL || capacity == 0 || part == NULL) {
        return;
    }
    length = wcslen(path);
    if (length > 0 && path[length - 1] != L'\\') {
        BFAppendString(path, capacity, L"\\");
    }
    BFAppendString(path, capacity, part);
}

static int BFInstallerGetAppDataPath(wchar_t *path, size_t capacity, const wchar_t *fileName)
{
    DWORD length;
    if (capacity == 0) {
        return 0;
    }
    length = GetEnvironmentVariableW(L"APPDATA", path, (DWORD)capacity);
    if (length == 0 || length >= capacity) {
        return 0;
    }
    BFInstallerAppendPath(path, capacity, L"BlackFix");
    CreateDirectoryW(path, NULL);
    BFInstallerAppendPath(path, capacity, fileName);
    return 1;
}

static int BFInstallerGetInstallStatePath(wchar_t *path, size_t capacity)
{
    return BFInstallerGetAppDataPath(path, capacity, L"BFInstall.txt");
}

static int BFInstallerAlreadyDone(void)
{
    wchar_t path[BF_PATH_CAPACITY];
    wchar_t line[256];
    FILE *file;
    int installed = 0;

    if (!BFInstallerGetInstallStatePath(path, sizeof(path) / sizeof(path[0]))) {
        return 0;
    }
    file = _wfopen(path, L"r");
    if (file == NULL) {
        return 0;
    }
    while (fgetws(line, sizeof(line) / sizeof(line[0]), file) != NULL) {
        if (wcsncmp(line, L"installed=1", 11) == 0) {
            installed = 1;
            break;
        }
    }
    fclose(file);
    return installed;
}

static void BFInstallerSaveState(const wchar_t *installPath, int desktopShortcut)
{
    wchar_t path[BF_PATH_CAPACITY];
    FILE *file;

    if (!BFInstallerGetInstallStatePath(path, sizeof(path) / sizeof(path[0]))) {
        return;
    }
    file = _wfopen(path, L"w");
    if (file == NULL) {
        return;
    }
    fwprintf(file, L"installed=1\n");
    fwprintf(file, L"path=%ls\n", installPath != NULL ? installPath : L"");
    fwprintf(file, L"desktopShortcut=%d\n", desktopShortcut != 0);
    fclose(file);
}

static void BFInstallerDefaultPath(wchar_t *path, size_t capacity)
{
    DWORD length = GetEnvironmentVariableW(L"LOCALAPPDATA", path, (DWORD)capacity);
    if (length == 0 || length >= capacity) {
        GetModuleFileNameW(NULL, path, (DWORD)capacity);
        path[capacity - 1] = L'\0';
        {
            wchar_t *slash = wcsrchr(path, L'\\');
            if (slash != NULL) {
                *slash = L'\0';
            }
        }
    }
    BFInstallerAppendPath(path, capacity, L"BlackFix_App");
}

static int BFInstallerCreateDirectoryTree(const wchar_t *path)
{
    wchar_t current[BF_PATH_CAPACITY];
    size_t i;
    size_t length;

    if (!BFHasText(path)) {
        return 0;
    }
    BFCopyString(current, sizeof(current) / sizeof(current[0]), path);
    length = wcslen(current);
    for (i = 3; i < length; ++i) {
        if (current[i] == L'\\') {
            current[i] = L'\0';
            CreateDirectoryW(current, NULL);
            current[i] = L'\\';
        }
    }
    if (!CreateDirectoryW(current, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return 0;
    }
    return GetFileAttributesW(current) != INVALID_FILE_ATTRIBUTES;
}

static int BFInstallerCreateDesktopShortcut(const wchar_t *targetExe, const wchar_t *installPath)
{
    wchar_t desktop[BF_PATH_CAPACITY];
    wchar_t shortcut[BF_PATH_CAPACITY];
    IShellLinkW *link = NULL;
    IPersistFile *persist = NULL;
    HRESULT hr;
    int coinited = 0;
    int ok = 0;

    if (SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, desktop) != S_OK) {
        return 0;
    }
    BFCopyString(shortcut, sizeof(shortcut) / sizeof(shortcut[0]), desktop);
    BFInstallerAppendPath(shortcut, sizeof(shortcut) / sizeof(shortcut[0]), BF_SHORTCUT_NAME);

    hr = CoInitialize(NULL);
    if (SUCCEEDED(hr)) {
        coinited = 1;
    } else if (hr != RPC_E_CHANGED_MODE) {
        return 0;
    }

    hr = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (void **)&link);
    if (SUCCEEDED(hr) && link != NULL) {
        link->lpVtbl->SetPath(link, targetExe);
        link->lpVtbl->SetWorkingDirectory(link, installPath);
        link->lpVtbl->SetIconLocation(link, targetExe, 0);
        link->lpVtbl->SetDescription(link, L"BlackFix App");
        hr = link->lpVtbl->QueryInterface(link, &IID_IPersistFile, (void **)&persist);
        if (SUCCEEDED(hr) && persist != NULL) {
            ok = SUCCEEDED(persist->lpVtbl->Save(persist, shortcut, TRUE));
            persist->lpVtbl->Release(persist);
        }
        link->lpVtbl->Release(link);
    }
    if (coinited) {
        CoUninitialize();
    }
    return ok;
}

static int BFInstallerInstall(BFInstallerState *state)
{
    wchar_t source[BF_PATH_CAPACITY];
    wchar_t target[BF_PATH_CAPACITY];
    wchar_t editedPath[BF_PATH_CAPACITY];
    HWND desktopBox = GetDlgItem(state->hwnd, BF_ID_DESKTOP);
    int desktopShortcut;

    editedPath[0] = L'\0';
    GetDlgItemTextW(state->hwnd, BF_ID_PATH_EDIT, editedPath, (int)(sizeof(editedPath) / sizeof(editedPath[0])));
    if (BFHasText(editedPath)) {
        BFCopyString(state->installPath, sizeof(state->installPath) / sizeof(state->installPath[0]), editedPath);
    }
    if (!BFHasText(state->installPath)) {
        MessageBoxW(state->hwnd, L"설치 경로를 입력하세요.", L"BlackFix 설치", MB_OK | MB_ICONWARNING);
        return 0;
    }
    if (!BFInstallerCreateDirectoryTree(state->installPath)) {
        MessageBoxW(state->hwnd, L"설치 폴더를 만들 수 없습니다. 다른 경로를 선택하세요.", L"BlackFix 설치", MB_OK | MB_ICONERROR);
        return 0;
    }

    if (GetModuleFileNameW(NULL, source, sizeof(source) / sizeof(source[0])) == 0) {
        MessageBoxW(state->hwnd, L"현재 실행 파일 경로를 확인할 수 없습니다.", L"BlackFix 설치", MB_OK | MB_ICONERROR);
        return 0;
    }
    BFCopyString(target, sizeof(target) / sizeof(target[0]), state->installPath);
    BFInstallerAppendPath(target, sizeof(target) / sizeof(target[0]), BF_INSTALL_EXE_NAME);

    if (_wcsicmp(source, target) != 0 && !CopyFileW(source, target, FALSE)) {
        MessageBoxW(state->hwnd, L"BlackFix_App.exe를 설치 경로에 복사할 수 없습니다.", L"BlackFix 설치", MB_OK | MB_ICONERROR);
        return 0;
    }

    desktopShortcut = SendMessageW(desktopBox != NULL ? desktopBox : state->desktopBox, BM_GETCHECK, 0, 0) == BST_CHECKED;
    if (desktopShortcut && !BFInstallerCreateDesktopShortcut(target, state->installPath)) {
        MessageBoxW(state->hwnd, L"바탕화면 바로가기를 만들 수 없습니다. 설치는 계속 진행합니다.", L"BlackFix 설치", MB_OK | MB_ICONWARNING);
    }

    BFInstallerSaveState(state->installPath, desktopShortcut);
    BFCopyString(state->installedExe, sizeof(state->installedExe) / sizeof(state->installedExe[0]), target);
    if (_wcsicmp(source, target) != 0) {
        ShellExecuteW(NULL, L"open", target, NULL, state->installPath, SW_SHOWNORMAL);
        state->launchInstalled = 1;
    }
    state->proceed = 1;
    state->done = 1;
    DestroyWindow(state->hwnd);
    return 1;
}

static void BFInstallerShowPage(BFInstallerState *state, BFInstallerPage page)
{
    int legal = page == BF_INSTALLER_PAGE_LEGAL;
    state->page = page;

    ShowWindow(state->legalEdit, legal ? SW_SHOW : SW_HIDE);
    ShowWindow(state->agreeBox, legal ? SW_SHOW : SW_HIDE);
    ShowWindow(state->nextButton, legal ? SW_SHOW : SW_HIDE);

    ShowWindow(state->pathLabel, legal ? SW_HIDE : SW_SHOW);
    ShowWindow(state->pathEdit, legal ? SW_HIDE : SW_SHOW);
    ShowWindow(state->browseButton, legal ? SW_HIDE : SW_SHOW);
    ShowWindow(state->desktopBox, legal ? SW_HIDE : SW_SHOW);
    ShowWindow(state->backButton, legal ? SW_HIDE : SW_SHOW);
    ShowWindow(state->installButton, legal ? SW_HIDE : SW_SHOW);
}

static void BFInstallerLayout(BFInstallerState *state)
{
    RECT client;
    int width;
    int height;

    GetClientRect(state->hwnd, &client);
    width = client.right - client.left;
    height = client.bottom - client.top;

    MoveWindow(state->legalEdit, 24, 54, width - 48, height - 138, TRUE);
    MoveWindow(state->agreeBox, 24, height - 74, width - 220, 26, TRUE);
    MoveWindow(state->cancelButton, width - 108, height - 42, 84, 28, TRUE);
    MoveWindow(state->nextButton, width - 202, height - 42, 84, 28, TRUE);

    MoveWindow(state->pathLabel, 24, 82, width - 48, 26, TRUE);
    MoveWindow(state->pathEdit, 24, 118, width - 148, 28, TRUE);
    MoveWindow(state->browseButton, width - 112, 118, 88, 28, TRUE);
    MoveWindow(state->desktopBox, 24, 160, width - 48, 28, TRUE);
    MoveWindow(state->backButton, width - 296, height - 42, 84, 28, TRUE);
    MoveWindow(state->installButton, width - 202, height - 42, 84, 28, TRUE);
}

static int BFInstallerBrowseFolder(BFInstallerState *state)
{
    BROWSEINFOW browse;
    PIDLIST_ABSOLUTE pidl;
    wchar_t path[BF_PATH_CAPACITY];

    ZeroMemory(&browse, sizeof(browse));
    browse.hwndOwner = state->hwnd;
    browse.lpszTitle = L"BlackFix App을 설치할 폴더를 선택하세요.";
    browse.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
    pidl = SHBrowseForFolderW(&browse);
    if (pidl == NULL) {
        return 0;
    }
    if (SHGetPathFromIDListW(pidl, path)) {
        SetWindowTextW(state->pathEdit, path);
    }
    CoTaskMemFree(pidl);
    return 1;
}

static void BFInstallerCreateControls(BFInstallerState *state)
{
    state->legalEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", BF_LEGAL_TEXT, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_LEGAL_EDIT, state->instance, NULL);
    state->agreeBox = CreateWindowExW(0, L"BUTTON", L"위 저작권 및 콘텐츠 이용 고지를 모두 읽고 확인했습니다.", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_AGREE, state->instance, NULL);
    state->nextButton = CreateWindowExW(0, L"BUTTON", L"다음", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_NEXT, state->instance, NULL);
    state->cancelButton = CreateWindowExW(0, L"BUTTON", L"취소", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_CANCEL, state->instance, NULL);

    state->pathLabel = CreateWindowExW(0, L"STATIC", L"설치할 경로를 선택하세요. 설치 파일 이름은 BlackFix_App.exe로 저장됩니다.", WS_CHILD, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_PATH_LABEL, state->instance, NULL);
    state->pathEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", state->installPath, WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_PATH_EDIT, state->instance, NULL);
    state->browseButton = CreateWindowExW(0, L"BUTTON", L"경로 설정", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_BROWSE, state->instance, NULL);
    state->desktopBox = CreateWindowExW(0, L"BUTTON", L"바탕화면에 BlackFix_App 바로가기 추가", WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_DESKTOP, state->instance, NULL);
    state->backButton = CreateWindowExW(0, L"BUTTON", L"이전", WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_BACK, state->instance, NULL);
    state->installButton = CreateWindowExW(0, L"BUTTON", L"설치", WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON, 0, 0, 0, 0, state->hwnd, (HMENU)BF_ID_INSTALL, state->instance, NULL);

    SendMessageW(state->desktopBox, BM_SETCHECK, BST_CHECKED, 0);
    EnableWindow(state->nextButton, FALSE);
    BFInstallerShowPage(state, BF_INSTALLER_PAGE_LEGAL);
}

static LRESULT CALLBACK BFInstallerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BFInstallerState *state = (BFInstallerState *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    switch (message) {
    case WM_NCCREATE:
    {
        CREATESTRUCTW *create = (CREATESTRUCTW *)lParam;
        state = (BFInstallerState *)create->lpCreateParams;
        state->hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)state);
        return TRUE;
    }

    case WM_CREATE:
        BFInstallerCreateControls(state);
        BFInstallerLayout(state);
        return 0;

    case WM_SIZE:
        if (state != NULL) {
            BFInstallerLayout(state);
        }
        return 0;

    case WM_COMMAND:
        if (state == NULL) {
            break;
        }
        switch (LOWORD(wParam)) {
        case BF_ID_AGREE:
            EnableWindow(state->nextButton, SendMessageW(state->agreeBox, BM_GETCHECK, 0, 0) == BST_CHECKED);
            return 0;
        case BF_ID_NEXT:
            if (SendMessageW(state->agreeBox, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                BFInstallerShowPage(state, BF_INSTALLER_PAGE_PATH);
                BFInstallerLayout(state);
            }
            return 0;
        case BF_ID_BACK:
            BFInstallerShowPage(state, BF_INSTALLER_PAGE_LEGAL);
            BFInstallerLayout(state);
            return 0;
        case BF_ID_BROWSE:
            BFInstallerBrowseFolder(state);
            return 0;
        case BF_ID_INSTALL:
            BFInstallerInstall(state);
            return 0;
        case BF_ID_PATH_EDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                GetDlgItemTextW(state->hwnd, BF_ID_PATH_EDIT, state->installPath, (int)(sizeof(state->installPath) / sizeof(state->installPath[0])));
            }
            return 0;
        case BF_ID_CANCEL:
            state->done = 1;
            state->proceed = 0;
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    case WM_CLOSE:
        if (state != NULL) {
            state->done = 1;
            state->proceed = 0;
        }
        DestroyWindow(hwnd);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}

static int BFInstallerRegisterClass(HINSTANCE instance)
{
    WNDCLASSEXW cls;
    ZeroMemory(&cls, sizeof(cls));
    cls.cbSize = sizeof(cls);
    cls.lpfnWndProc = BFInstallerProc;
    cls.hInstance = instance;
    cls.hCursor = LoadCursorW(NULL, IDC_ARROW);
    cls.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(BF_ICON_APP));
    cls.hIconSm = LoadIconW(instance, MAKEINTRESOURCEW(BF_ICON_APP));
    cls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    cls.lpszClassName = BF_INSTALLER_CLASS;
    return RegisterClassExW(&cls) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

int BFRunInstallerIfNeeded(HINSTANCE instance)
{
    BFInstallerState state;
    MSG message;

    if (BFInstallerAlreadyDone()) {
        return 1;
    }

    if (!BFInstallerRegisterClass(instance)) {
        return 1;
    }

    ZeroMemory(&state, sizeof(state));
    state.instance = instance;
    BFInstallerDefaultPath(state.installPath, sizeof(state.installPath) / sizeof(state.installPath[0]));

    state.hwnd = CreateWindowExW(WS_EX_APPWINDOW, BF_INSTALLER_CLASS, L"BlackFix App 설치", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 820, 640, NULL, NULL, instance, &state);
    if (state.hwnd == NULL) {
        return 1;
    }

    SetWindowTextW(state.hwnd, L"BlackFix App 설치");
    ShowWindow(state.hwnd, SW_SHOWNORMAL);
    UpdateWindow(state.hwnd);

    while (!state.done && GetMessageW(&message, NULL, 0, 0) > 0) {
        if (!IsDialogMessageW(state.hwnd, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    if (state.launchInstalled) {
        return 0;
    }
    return state.proceed;
}
