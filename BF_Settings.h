#ifndef BF_SETTINGS_H
#define BF_SETTINGS_H

#include "BF_App.h"

int BFRegisterSettingsWindow(HINSTANCE instance);
void BFOpenSettingsWindow(HWND parent);
void BFToggleSettingsWindow(HWND parent);
void BFSettingsSyncControls(void);

#endif
