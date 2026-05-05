cmake_minimum_required(VERSION 3.20)

project(BlackFix C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(BF_BUILD_VERSION "local" CACHE STRING "BlackFix build version")

add_executable(BlackFix WIN32
    BFMain.c
    BF_App.c
    BF_BlackFix.c
    BF_Album.c
    BF_Member.c
    BF_Settings.c
    BF_Installer.c
    BF_Update.c
    BF_Content.c
    BF_Resources.rc
)

target_compile_definitions(BlackFix PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN)
target_compile_definitions(BlackFix PRIVATE "BF_BUILD_VERSION=L\"${BF_BUILD_VERSION}\"")

if(MSVC)
    target_compile_options(BlackFix PRIVATE /W4 /utf-8)
endif()

if(MINGW)
    target_compile_options(BlackFix PRIVATE -Wall -Wextra -Wpedantic -finput-charset=UTF-8)
    target_link_options(BlackFix PRIVATE -municode -static -static-libgcc)
endif()

target_link_libraries(BlackFix PRIVATE user32 gdi32 shell32 comctl32 winmm urlmon ole32 uuid)
