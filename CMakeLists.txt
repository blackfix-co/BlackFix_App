cmake_minimum_required(VERSION 3.20)

project(BlackFix C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(BF_BUILD_VERSION "local" CACHE STRING "BlackFix build version")

add_executable(BlackFix WIN32
    BFMain.c
    Source/App/BF_App.c
    Source/BlackFix/BF_BlackFix.c
    Source/Album/BF_Album.c
    Source/Member/BF_Member.c
    Source/Settings/BF_Settings.c
    Source/Installer/BF_Installer.c
    Source/Update/BF_Update.c
    Source/Content/BF_Content.c
    Source/Resource/BF_Resources.rc
)

target_include_directories(BlackFix PRIVATE
    Source/App
    Source/BlackFix
    Source/Album
    Source/Member
    Source/Settings
    Source/Installer
    Source/Update
    Source/Content
    Source/Resource
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
