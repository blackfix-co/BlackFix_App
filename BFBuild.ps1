$ErrorActionPreference = 'Stop'

$mingwBin = 'C:\msys64\mingw64\bin'
$msysBin = 'C:\msys64\usr\bin'

if (-not (Test-Path $mingwBin)) {
    throw "MinGW GCC not found: $mingwBin"
}

if (-not (Test-Path $msysBin)) {
    throw "MSYS make not found: $msysBin"
}

$env:PATH = "$mingwBin;$msysBin;$env:PATH"

cmake -S . -B build -G "MSYS Makefiles" -DCMAKE_C_COMPILER=gcc
cmake --build build
