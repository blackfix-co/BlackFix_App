#ifndef BF_INSTALLER_H
#define BF_INSTALLER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

int BFRunInstallerIfNeeded(HINSTANCE instance);

#endif
