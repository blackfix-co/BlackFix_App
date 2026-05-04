#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "BF_App.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previousInstance, PWSTR commandLine, int showCommand)
{
    (void)previousInstance;
    (void)commandLine;
    return BFRunApplication(instance, showCommand);
}
