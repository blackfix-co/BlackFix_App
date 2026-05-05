#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <urlmon.h>
#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include "BF_Custom.h"

#define BF_CUSTOM_CONFIG_URL L"https://github.com/blackfix-co/BlackFix_App/releases/latest/download/BF_Custom.ini"

static BFPalette BF_CustomPalette = {
    RGB(2, 6, 4),
    RGB(6, 14, 9),
    RGB(9, 25, 14),
    RGB(10, 35, 16),
    RGB(198, 255, 207),
    RGB(118, 184, 126),
    RGB(42, 255, 121),
    RGB(18, 68, 30),
    RGB(13, 69, 31),
    RGB(21, 93, 42)
};
static wchar_t BF_CustomThemeName[64] = L"Custom";
static wchar_t BF_CustomEffectName[64] = L"Custom";
static COLORREF BF_CustomEffectPrimary = RGB(255, 96, 56);
static COLORREF BF_CustomEffectSecondary = RGB(255, 228, 92);
static HBITMAP BF_CustomBackground;
static HBITMAP BF_CustomEffectImage;

static wchar_t *BFCustomTrim(wchar_t *text)
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

static int BFParseColor(const wchar_t *text, COLORREF *color)
{
    unsigned int value;
    if (!BFHasText(text) || text[0] != L'#' || wcslen(text) < 7) {
        return 0;
    }
    if (swscanf(text + 1, L"%x", &value) != 1) {
        return 0;
    }
    *color = RGB((value >> 16) & 255, (value >> 8) & 255, value & 255);
    return 1;
}

static void BFWriteDefaultCustomConfig(const wchar_t *path)
{
    FILE *file = _wfopen(path, L"w");
    if (file == NULL) {
        return;
    }
    fwprintf(file, L"themeName=Custom\n");
    fwprintf(file, L"themeBg=#020604\n");
    fwprintf(file, L"themePanel=#060E09\n");
    fwprintf(file, L"themePanelAlt=#09190E\n");
    fwprintf(file, L"themeGrid=#0A2310\n");
    fwprintf(file, L"themeText=#C6FFCF\n");
    fwprintf(file, L"themeMuted=#76B87E\n");
    fwprintf(file, L"themeAccent=#2AFF79\n");
    fwprintf(file, L"themeBorder=#12441E\n");
    fwprintf(file, L"themeSelected=#0D451F\n");
    fwprintf(file, L"themePressed=#155D2A\n");
    fwprintf(file, L"backgroundImage=\n");
    fwprintf(file, L"effectName=Custom\n");
    fwprintf(file, L"effectPrimary=#FF6038\n");
    fwprintf(file, L"effectSecondary=#FFE45C\n");
    fwprintf(file, L"effectImage=\n");
    fclose(file);
}

static int BFCustomConfigLooksValid(const wchar_t *path)
{
    FILE *file = _wfopen(path, L"r");
    wchar_t line[256];
    int valid = 0;
    if (file == NULL) {
        return 0;
    }
    while (fgetws(line, sizeof(line) / sizeof(line[0]), file) != NULL) {
        if (wcsncmp(BFCustomTrim(line), L"themeName=", 10) == 0) {
            valid = 1;
            break;
        }
    }
    fclose(file);
    return valid;
}

int BFGetCustomConfigPath(wchar_t *path, size_t capacity)
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
        BFAppendString(path, capacity, L"BF_Custom.ini");
        return 1;
    }

    BFAppendString(path, capacity, L"\\BlackFix");
    CreateDirectoryW(path, NULL);
    BFAppendString(path, capacity, L"\\BF_Custom.ini");
    return 1;
}

static void BFResolveCustomPath(const wchar_t *configPath, const wchar_t *source, wchar_t *target, size_t capacity)
{
    wchar_t *slash;
    if (!BFHasText(source)) {
        target[0] = L'\0';
        return;
    }
    if (wcsstr(source, L"://") != NULL || (iswalpha(source[0]) && source[1] == L':') || (source[0] == L'\\' && source[1] == L'\\')) {
        BFCopyString(target, capacity, source);
        return;
    }
    BFCopyString(target, capacity, configPath);
    slash = wcsrchr(target, L'\\');
    if (slash != NULL) {
        *(slash + 1) = L'\0';
    } else {
        target[0] = L'\0';
    }
    BFAppendString(target, capacity, source);
}

static HBITMAP BFLoadCustomBitmap(const wchar_t *configPath, const wchar_t *source)
{
    wchar_t path[BF_PATH_CAPACITY];
    if (!BFHasText(source)) {
        return NULL;
    }
    BFResolveCustomPath(configPath, source, path, sizeof(path) / sizeof(path[0]));
    return (HBITMAP)LoadImageW(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
}

void BFEnsureCustomConfig(void)
{
    wchar_t path[BF_PATH_CAPACITY];
    if (!BFGetCustomConfigPath(path, sizeof(path) / sizeof(path[0]))) {
        return;
    }
    if (GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES) {
        return;
    }
    if (FAILED(URLDownloadToFileW(NULL, BF_CUSTOM_CONFIG_URL, path, 0, NULL)) || !BFCustomConfigLooksValid(path)) {
        BFWriteDefaultCustomConfig(path);
    }
}

void BFLoadCustomConfig(void)
{
    wchar_t path[BF_PATH_CAPACITY];
    wchar_t line[512];
    wchar_t backgroundSource[BF_PATH_CAPACITY] = L"";
    wchar_t effectImageSource[BF_PATH_CAPACITY] = L"";
    FILE *file;

    if (!BFGetCustomConfigPath(path, sizeof(path) / sizeof(path[0]))) {
        return;
    }

    file = _wfopen(path, L"r");
    if (file == NULL) {
        return;
    }

    while (fgetws(line, sizeof(line) / sizeof(line[0]), file) != NULL) {
        wchar_t *text = BFCustomTrim(line);
        wchar_t *value = wcschr(text, L'=');
        if (value == NULL) {
            continue;
        }
        *value = L'\0';
        ++value;
        text = BFCustomTrim(text);
        value = BFCustomTrim(value);

        if (wcscmp(text, L"themeName") == 0) {
            BFCopyString(BF_CustomThemeName, sizeof(BF_CustomThemeName) / sizeof(BF_CustomThemeName[0]), value);
        } else if (wcscmp(text, L"themeBg") == 0) {
            BFParseColor(value, &BF_CustomPalette.bg);
        } else if (wcscmp(text, L"themePanel") == 0) {
            BFParseColor(value, &BF_CustomPalette.panel);
        } else if (wcscmp(text, L"themePanelAlt") == 0) {
            BFParseColor(value, &BF_CustomPalette.panelAlt);
        } else if (wcscmp(text, L"themeGrid") == 0) {
            BFParseColor(value, &BF_CustomPalette.grid);
        } else if (wcscmp(text, L"themeText") == 0) {
            BFParseColor(value, &BF_CustomPalette.text);
        } else if (wcscmp(text, L"themeMuted") == 0) {
            BFParseColor(value, &BF_CustomPalette.muted);
        } else if (wcscmp(text, L"themeAccent") == 0) {
            BFParseColor(value, &BF_CustomPalette.accent);
        } else if (wcscmp(text, L"themeBorder") == 0) {
            BFParseColor(value, &BF_CustomPalette.border);
        } else if (wcscmp(text, L"themeSelected") == 0) {
            BFParseColor(value, &BF_CustomPalette.selected);
        } else if (wcscmp(text, L"themePressed") == 0) {
            BFParseColor(value, &BF_CustomPalette.pressed);
        } else if (wcscmp(text, L"backgroundImage") == 0) {
            BFCopyString(backgroundSource, sizeof(backgroundSource) / sizeof(backgroundSource[0]), value);
        } else if (wcscmp(text, L"effectName") == 0) {
            BFCopyString(BF_CustomEffectName, sizeof(BF_CustomEffectName) / sizeof(BF_CustomEffectName[0]), value);
        } else if (wcscmp(text, L"effectPrimary") == 0) {
            BFParseColor(value, &BF_CustomEffectPrimary);
        } else if (wcscmp(text, L"effectSecondary") == 0) {
            BFParseColor(value, &BF_CustomEffectSecondary);
        } else if (wcscmp(text, L"effectImage") == 0) {
            BFCopyString(effectImageSource, sizeof(effectImageSource) / sizeof(effectImageSource[0]), value);
        }
    }
    fclose(file);

    if (BF_CustomBackground != NULL) {
        DeleteObject(BF_CustomBackground);
        BF_CustomBackground = NULL;
    }
    if (BF_CustomEffectImage != NULL) {
        DeleteObject(BF_CustomEffectImage);
        BF_CustomEffectImage = NULL;
    }
    BF_CustomBackground = BFLoadCustomBitmap(path, backgroundSource);
    BF_CustomEffectImage = BFLoadCustomBitmap(path, effectImageSource);
}

const BFPalette *BFCustomPalette(void)
{
    return &BF_CustomPalette;
}

const wchar_t *BFCustomThemeName(void)
{
    return BFHasText(BF_CustomThemeName) ? BF_CustomThemeName : L"Custom";
}

const wchar_t *BFCustomEffectName(void)
{
    return BFHasText(BF_CustomEffectName) ? BF_CustomEffectName : L"Custom";
}

COLORREF BFCustomEffectPrimary(void)
{
    return BF_CustomEffectPrimary;
}

COLORREF BFCustomEffectSecondary(void)
{
    return BF_CustomEffectSecondary;
}

int BFDrawCustomBackground(HDC dc, const RECT *client)
{
    HDC memoryDc;
    HGDIOBJ oldBitmap;
    BITMAP bitmap;

    if (BF_Settings.theme != BF_THEME_CUSTOM || BF_CustomBackground == NULL) {
        return 0;
    }

    memoryDc = CreateCompatibleDC(dc);
    if (memoryDc == NULL) {
        return 0;
    }
    oldBitmap = SelectObject(memoryDc, BF_CustomBackground);
    GetObjectW(BF_CustomBackground, sizeof(bitmap), &bitmap);
    StretchBlt(dc, client->left, client->top, client->right - client->left, client->bottom - client->top, memoryDc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);
    SelectObject(memoryDc, oldBitmap);
    DeleteDC(memoryDc);
    return 1;
}

int BFDrawCustomEffectImage(HDC dc, int x, int y, int size, int opacity)
{
    HDC memoryDc;
    HGDIOBJ oldBitmap;
    BITMAP bitmap;
    BLENDFUNCTION blend;
    int drawSize = BFMaxInt(4, size);

    if (BF_Settings.clickEffectStyle != BF_EFFECT_CUSTOM || BF_CustomEffectImage == NULL) {
        return 0;
    }

    memoryDc = CreateCompatibleDC(dc);
    if (memoryDc == NULL) {
        return 0;
    }
    oldBitmap = SelectObject(memoryDc, BF_CustomEffectImage);
    GetObjectW(BF_CustomEffectImage, sizeof(bitmap), &bitmap);
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = (BYTE)BFClampInt(opacity * 255 / 100, 0, 255);
    blend.AlphaFormat = 0;
    AlphaBlend(dc, x - drawSize / 2, y - drawSize / 2, drawSize, drawSize, memoryDc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, blend);
    SelectObject(memoryDc, oldBitmap);
    DeleteDC(memoryDc);
    return 1;
}
