#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#include <urlmon.h>
#include <stdio.h>
#include <wchar.h>
#include "BF_Update.h"
#include "BF_App.h"

#ifndef BF_BUILD_VERSION
#define BF_BUILD_VERSION L"local"
#endif

#define BF_UPDATE_VERSION_URL L"https://github.com/blackfix-co/BlackFix_App/releases/latest/download/version.txt"
#define BF_UPDATE_EXE_URL L"https://github.com/blackfix-co/BlackFix_App/releases/latest/download/BlackFix.exe"

static void BFTrimLine(wchar_t *text)
{
    size_t length = wcslen(text);
    while (length > 0 && (text[length - 1] == L'\r' || text[length - 1] == L'\n' || text[length - 1] == L' ' || text[length - 1] == L'\t')) {
        text[length - 1] = L'\0';
        --length;
    }
}

static int BFReadRemoteVersion(const wchar_t *path, wchar_t *version, size_t capacity)
{
    FILE *file;

    if (capacity == 0) {
        return 0;
    }

    version[0] = L'\0';
    file = _wfopen(path, L"r");
    if (file == NULL) {
        return 0;
    }

    if (fgetws(version, (int)capacity, file) == NULL) {
        fclose(file);
        return 0;
    }

    fclose(file);
    BFTrimLine(version);
    return BFHasText(version);
}

static int BFDownloadFile(const wchar_t *url, const wchar_t *path)
{
    HRESULT result = URLDownloadToFileW(NULL, url, path, 0, NULL);
    return SUCCEEDED(result);
}

static void BFRunReplacement(const wchar_t *downloadedExe)
{
    wchar_t currentExe[BF_PATH_CAPACITY];
    wchar_t parameters[BF_PATH_CAPACITY * 3];

    if (GetModuleFileNameW(NULL, currentExe, sizeof(currentExe) / sizeof(currentExe[0])) == 0) {
        return;
    }

    swprintf(parameters, sizeof(parameters) / sizeof(parameters[0]), L"/C ping 127.0.0.1 -n 3 >nul & copy /Y \"%ls\" \"%ls\" >nul & start \"\" \"%ls\"", downloadedExe, currentExe, currentExe);
    ShellExecuteW(NULL, L"open", L"cmd.exe", parameters, NULL, SW_HIDE);
    ExitProcess(0);
}

void BFCheckForUpdate(void)
{
    wchar_t tempPath[BF_PATH_CAPACITY];
    wchar_t versionPath[BF_PATH_CAPACITY];
    wchar_t exePath[BF_PATH_CAPACITY];
    wchar_t remoteVersion[128];

    if (wcscmp(BF_BUILD_VERSION, L"local") == 0) {
        return;
    }

    if (GetTempPathW(sizeof(tempPath) / sizeof(tempPath[0]), tempPath) == 0) {
        return;
    }

    BFCopyString(versionPath, sizeof(versionPath) / sizeof(versionPath[0]), tempPath);
    BFAppendString(versionPath, sizeof(versionPath) / sizeof(versionPath[0]), L"BlackFix_version.txt");

    if (!BFDownloadFile(BF_UPDATE_VERSION_URL, versionPath)) {
        return;
    }

    if (!BFReadRemoteVersion(versionPath, remoteVersion, sizeof(remoteVersion) / sizeof(remoteVersion[0]))) {
        DeleteFileW(versionPath);
        return;
    }

    DeleteFileW(versionPath);

    if (wcscmp(remoteVersion, BF_BUILD_VERSION) == 0) {
        return;
    }

    BFCopyString(exePath, sizeof(exePath) / sizeof(exePath[0]), tempPath);
    BFAppendString(exePath, sizeof(exePath) / sizeof(exePath[0]), L"BlackFix_update.exe");

    if (!BFDownloadFile(BF_UPDATE_EXE_URL, exePath)) {
        return;
    }

    BFRunReplacement(exePath);
}
