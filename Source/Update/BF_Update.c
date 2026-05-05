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

#define BF_UPDATE_VERSION_URL L"https://github.com/blackfix-co/BlackFix_App/releases/download/BlackFix-App/version.txt"
#define BF_UPDATE_EXE_URL L"https://github.com/blackfix-co/BlackFix_App/releases/download/BlackFix-App/BlackFix.exe"

static void BFWritePowerShellQuoted(FILE *file, const wchar_t *text)
{
    size_t i;

    fputwc(L'\'', file);
    if (text != NULL) {
        for (i = 0; text[i] != L'\0'; ++i) {
            if (text[i] == L'\'') {
                fputwc(L'\'', file);
                fputwc(L'\'', file);
            } else {
                fputwc(text[i], file);
            }
        }
    }
    fputwc(L'\'', file);
}

static int BFRunPowerShellScriptAndWait(const wchar_t *scriptPath, DWORD timeoutMs)
{
    wchar_t parameters[BF_PATH_CAPACITY + 128];
    SHELLEXECUTEINFOW info;
    DWORD exitCode = 1;

    swprintf(parameters, sizeof(parameters) / sizeof(parameters[0]), L"-NoProfile -ExecutionPolicy Bypass -File \"%ls\"", scriptPath);
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
    info.lpFile = L"powershell.exe";
    info.lpParameters = parameters;
    info.nShow = SW_HIDE;

    if (!ShellExecuteExW(&info) || info.hProcess == NULL) {
        return 0;
    }
    WaitForSingleObject(info.hProcess, timeoutMs);
    GetExitCodeProcess(info.hProcess, &exitCode);
    CloseHandle(info.hProcess);
    return exitCode == 0;
}

static int BFDownloadFileWithPowerShell(const wchar_t *url, const wchar_t *path)
{
    wchar_t tempPath[BF_PATH_CAPACITY];
    wchar_t scriptPath[BF_PATH_CAPACITY];
    FILE *file;
    int ok;

    if (GetTempPathW(sizeof(tempPath) / sizeof(tempPath[0]), tempPath) == 0) {
        return 0;
    }
    BFCopyString(scriptPath, sizeof(scriptPath) / sizeof(scriptPath[0]), tempPath);
    BFAppendString(scriptPath, sizeof(scriptPath) / sizeof(scriptPath[0]), L"BlackFix_download.ps1");

    file = _wfopen(scriptPath, L"w, ccs=UTF-8");
    if (file == NULL) {
        return 0;
    }
    fwprintf(file, L"$ErrorActionPreference = 'Stop'\n");
    fwprintf(file, L"[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12\n");
    fwprintf(file, L"Invoke-WebRequest -UseBasicParsing -Uri ");
    BFWritePowerShellQuoted(file, url);
    fwprintf(file, L" -OutFile ");
    BFWritePowerShellQuoted(file, path);
    fwprintf(file, L"\n");
    fclose(file);

    ok = BFRunPowerShellScriptAndWait(scriptPath, 120000);
    DeleteFileW(scriptPath);
    return ok;
}

static void BFBuildFreshUrl(const wchar_t *url, wchar_t *target, size_t capacity)
{
    wchar_t stamp[64];

    BFCopyString(target, capacity, url);
    BFAppendString(target, capacity, wcschr(url, L'?') != NULL ? L"&bfCache=" : L"?bfCache=");
    swprintf(stamp, sizeof(stamp) / sizeof(stamp[0]), L"%lu_%lu", GetTickCount(), GetCurrentProcessId());
    BFAppendString(target, capacity, stamp);
}

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
    wchar_t freshUrl[BF_PATH_CAPACITY * 2];
    HRESULT result;

    BFBuildFreshUrl(url, freshUrl, sizeof(freshUrl) / sizeof(freshUrl[0]));
    DeleteFileW(path);
    result = URLDownloadToFileW(NULL, freshUrl, path, 0, NULL);
    if (SUCCEEDED(result)) {
        return 1;
    }
    DeleteFileW(path);
    return BFDownloadFileWithPowerShell(freshUrl, path);
}

static int BFWriteReplacementScript(const wchar_t *scriptPath, const wchar_t *downloadedExe, const wchar_t *currentExe)
{
    FILE *file = _wfopen(scriptPath, L"w, ccs=UTF-8");
    if (file == NULL) {
        return 0;
    }
    fwprintf(file, L"$ErrorActionPreference = 'SilentlyContinue'\n");
    fwprintf(file, L"$targetPid = %lu\n", GetCurrentProcessId());
    fwprintf(file, L"$src = ");
    BFWritePowerShellQuoted(file, downloadedExe);
    fwprintf(file, L"\n$dst = ");
    BFWritePowerShellQuoted(file, currentExe);
    fwprintf(file, L"\ntry { Wait-Process -Id $targetPid -Timeout 10 -ErrorAction SilentlyContinue } catch {}\n");
    fwprintf(file, L"for ($i = 0; $i -lt 80; $i++) {\n");
    fwprintf(file, L"  try {\n");
    fwprintf(file, L"    Copy-Item -LiteralPath $src -Destination $dst -Force -ErrorAction Stop\n");
    fwprintf(file, L"    Start-Process -FilePath $dst\n");
    fwprintf(file, L"    Remove-Item -LiteralPath $src -Force -ErrorAction SilentlyContinue\n");
    fwprintf(file, L"    Remove-Item -LiteralPath $PSCommandPath -Force -ErrorAction SilentlyContinue\n");
    fwprintf(file, L"    exit 0\n");
    fwprintf(file, L"  } catch {\n");
    fwprintf(file, L"    Start-Sleep -Milliseconds 250\n");
    fwprintf(file, L"  }\n");
    fwprintf(file, L"}\n");
    fwprintf(file, L"Start-Process -FilePath $src\n");
    fclose(file);
    return 1;
}

static void BFRunReplacement(const wchar_t *downloadedExe)
{
    wchar_t currentExe[BF_PATH_CAPACITY];
    wchar_t tempPath[BF_PATH_CAPACITY];
    wchar_t scriptPath[BF_PATH_CAPACITY];
    wchar_t psParameters[BF_PATH_CAPACITY + 128];
    wchar_t parameters[BF_PATH_CAPACITY * 3];

    if (GetModuleFileNameW(NULL, currentExe, sizeof(currentExe) / sizeof(currentExe[0])) == 0) {
        return;
    }

    if (GetTempPathW(sizeof(tempPath) / sizeof(tempPath[0]), tempPath) != 0) {
        BFCopyString(scriptPath, sizeof(scriptPath) / sizeof(scriptPath[0]), tempPath);
        BFAppendString(scriptPath, sizeof(scriptPath) / sizeof(scriptPath[0]), L"BlackFix_replace.ps1");
        if (BFWriteReplacementScript(scriptPath, downloadedExe, currentExe)) {
            swprintf(psParameters, sizeof(psParameters) / sizeof(psParameters[0]), L"-NoProfile -ExecutionPolicy Bypass -File \"%ls\"", scriptPath);
            ShellExecuteW(NULL, L"open", L"powershell.exe", psParameters, NULL, SW_HIDE);
            ExitProcess(0);
        }
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
