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

## Kurulum komutlari

### Windows

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\install.ps1
```

Belirli bir kurulum klasoru:

```powershell
.\install.ps1 -Prefix C:\Tools\mdcompiler
```

Bagnamsiz paketi yeniden kurmadan derlemek icin:

```powershell
.\install.ps1 -SkipDependencies
```

### Linux/macOS

```sh
chmod +x install.sh update-modules.sh export-to-directory.sh
./install.sh
./install.sh "$HOME/.local"
```

Calistirma ve derleme betigi:

```powershell
.\run.bat
```

## R kütüphanesi

R için bağımlılıksız genel veri işleme paketi `r/mdutils` klasöründe bulunur:

- `md_remove_empty()`: boş satır ve sütunları kaldırır
- `md_normalize_names()`: sütun adlarını lowercase snake_case biçimine çevirir
- `md_fill_missing()`: sayıları medyanla, metinleri kolon modu ile doldurur
- `md_summarize()`: kolon türü, eksik ve benzersiz değer sayılarını üretir

```r
install.packages("devtools")
devtools::load_all("r/mdutils")

data <- data.frame("Full Name" = c("Ada", NA), Age = c(36, NA))
data <- md_normalize_names(data)
md_summarize(data)
md_fill_missing(data)
```
