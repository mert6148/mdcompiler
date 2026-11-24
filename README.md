# mdcompiler

C ve C++ kaynak kodlarini derleyen, calistiran, hafif komut satiri
derleyici yoneticisidir. Bu surumde desteklenen programlama dili yalnizca C
ve C++'tir.

## Gereksinimler

- CMake 3.16 veya yeni
- C++17 derleyicisi (`g++`, `clang++`, MSVC `cl` veya uyumlu baska bir derleyici)
- Kaynak derlemek icin sisteminizde `gcc`/`clang` veya `g++`/`clang++` bulunmali

## Derleme

```sh
cmake -S . -B build
cmake --build build
```

Windows'ta Visual Studio Developer Command Prompt icinden MSVC:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Kullanim

```sh
# C++ dosyasi derle
mdcompiler build main.cpp

# C dosyasi derle
mdcompiler build main.c

# Derle ve calistir
mdcompiler run main.cpp -- Merhaba 42

# Cikti yolunu belirle
mdcompiler build main.cpp --output dist/main

# Standart, optimizasyon ve debug secenekleri
mdcompiler build main.cpp --std c++20 --optimize 2 --debug

# Baslik klasoru ve makro
mdcompiler build main.cpp --include include --define APP_VERSION=1
```

Derleyiciyi otomatik olarak algilar. Farkli bir derleyici icin ortam
degiskenlerini kullanabilirsiniz:

```sh
MDCOMPILER_CXX=clang++ mdcompiler build main.cpp
MDCOMPILER_CC=clang mdcompiler build main.c
```

Windows'ta PowerShell icinde ayni islemler:

```powershell
$env:MDCOMPILER_CXX = "cl"
$env:MDCOMPILER_CC = "cl"
mdcompiler build main.cpp
```
