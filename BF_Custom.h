#ifndef BF_CUSTOM_H
#define BF_CUSTOM_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "BF_App.h"

void BFEnsureCustomConfig(void);
void BFLoadCustomConfig(void);
int BFGetCustomConfigPath(wchar_t *path, size_t capacity);
const BFPalette *BFCustomPalette(void);
const wchar_t *BFCustomThemeName(void);
const wchar_t *BFCustomEffectName(void);
COLORREF BFCustomEffectPrimary(void);
COLORREF BFCustomEffectSecondary(void);
int BFDrawCustomBackground(HDC dc, const RECT *client);
int BFDrawCustomEffectImage(HDC dc, int x, int y, int size, int opacity);

#endif
