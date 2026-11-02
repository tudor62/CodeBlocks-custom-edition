<div align="center">

# Code::Blocks Custom Edition v25.03 (Dark Optimized)

*Un IDE C/C++ rapid, ușor și modern, construit pe un nucleu nativ solid, cu temă întunecată completă și configurat pentru productivitate instantanee.*

[![Release v25.03](https://img.shields.io/badge/release-v25.03-2ea44f?style=for-the-badge)](../../releases)
[![License GPLv3](https://img.shields.io/badge/license-GPLv3-blue?style=for-the-badge)](COPYING)
[![Platform Windows x64](https://img.shields.io/badge/platform-Windows%20x64-0078D6?style=for-the-badge&logo=windows&logoColor=white)](#descărcare-și-pornire-rapidă)
[![Languages RO / EN](https://img.shields.io/badge/languages-RO%20%7C%20EN-d73a49?style=for-the-badge)](#funcționalități-cheie)

[English](README.md) · **Română**

</div>

---

**Code::Blocks Custom Edition (Dark Optimized)** este o modernizare concentrată a IDE-ului clasic Code::Blocks. Păstrează nucleul nativ solid, fluxul familiar de proiecte și compilare, precum și ecosistemul de plugin-uri, adăugând în același timp o interfață întunecată completă, elemente vizuale moderne, optimizări țintite de performanță și un installer curat, per utilizator.

Scopul este simplu: un IDE C/C++ care se simte rapid și modern pe Windows 10/11 x64 — fără overhead-ul unui Electron, fără telemetrie și fără solicitarea drepturilor de administrator.

## Prezentare generală

Această ediție este construită pe **Code::Blocks 25.03**, cu stratul de interfață portat la **wxWidgets 3.2.11** și compilat cu **GCC 16.1.0** în mediul MSYS2 UCRT64. Arhitectura nativă originală este păstrată; modificările sunt concentrate în zonele care afectează confortul și viteza de zi cu zi: randarea temei, iconițele, culorile editorului, comportamentul la pornire, logarea și integrarea compilatorului.

| | |
| --- | --- |
| **Versiune** | 25.03 |
| **IDE de bază** | Code::Blocks 25.03 |
| **Toolkit UI** | wxWidgets 3.2.11 |
| **Lanț de compilare** | GCC 16.1.0 · MSYS2 UCRT64 |
| **Platformă** | Windows 10 / 11 · x64 |
| **Installer** | Inno Setup 6 · per utilizator · fără UAC |
| **Licență** | GNU GPL v3.0 |

## Funcționalități cheie

### 🌑 Interfață întunecată completă și stil Scintilla

Fiecare suprafață importantă este întunecată: bara de meniu, bara de titlu, bara de instrumente, panourile AUI, taburile, logurile, dialogurile, barele de derulare și controalele native uzuale. Editorul Scintilla folosește o paletă reglată, cu cuvinte-cheie cyan, comentarii verde-soft și literali magenta. **Parantezele și acoladele pereche sunt pre-configurate cu roșu intens `#FF0000`, bold, pe un fundal întunecat subtil (`#13181D`)** — perechea este imposibil de ratat.

Stratul vizual include, de asemenea, iconițe SVG moderne pentru bara de instrumente, o pagină Start Here reproiectată, un splash screen nou și o iconiță de aplicație actualizată, toate clare la rezoluții High-DPI.

### ⚡ Performanță nativă

Construit pe nucleul nativ Code::Blocks, nu pe un wrapper web. Sesiunile obișnuite de editare rămân în jurul a **~50 MB RAM**, iar IDE-ul pornește fără așteptarea unui splash screen. Încărcarea workspace-ului este amânată până când fereastra principală devine vizibilă, tema este aplicată într-o singură trecere, scrierile în log sunt grupate în loturi, iar setările compilatorului sunt memorate în cache.

Optimizările din Etapa 1 sunt documentate în detaliu în notele de release.

### 🌍 Suport bilingv

Installerul este complet bilingv (**Română / English**) și configurează localizarea IDE-ului (`ro_RO` sau `en`) în funcție de alegerea ta. Documentația și notele de release sunt întreținute în ambele limbi, iar pagina finală a installerului deschide automat fișierul corect.

### 🧩 Integrare cu Windows Explorer

Sarcinile opționale per utilizator îți permit să:

- asociezi fișierele `.cbp`, `.cpp`, `.c` și `.h` cu Code::Blocks;
- adaugi folderul `bin` al Code::Blocks în `%PATH%`;
- adaugi **Open with Code::Blocks** în meniul contextual al Explorerului pentru foldere și fișiere C/C++.

Toate modificările sunt scrise sub `HKCU` și sunt reversate curat la dezinstalare. Asocierile existente și intrările `OpenWithProgids` sunt păstrate.

### 📦 Installer per utilizator, fără UAC

Installerul vizează implicit `%LOCALAPPDATA%\Programs\CodeBlocks Custom Edition` și **nu necesită drepturi de administrator**. Fișierul existent `%APPDATA%\codeblocks\default.conf` este păstrat, astfel încât actualizările nu îți șterg setările.

Scurtătura de pe Desktop se numește exact **CodeBlocks**, iar pagina finală oferă o scurtătură opțională către notele de release. Modul de instalare silențioasă este disponibil pentru implementări automatizate.

## Descărcare și pornire rapidă

### 1. Descărcare

Deschide pagina **[Releases](../../releases)** și descarcă ultimul `setup.exe` (aproximativ 28 MB).

> **Windows SmartScreen:** executabilul nu este semnat digital. Dacă Windows afișează un avertisment, verifică faptul că l-ai descărcat din acest repozitoriu, apoi alege **More info → Run anyway**.

### 2. Instalare

1. Rulează `setup.exe`.
2. Alege **Română** sau **English**.
3. Păstrează locația implicită per utilizator sau selectează alt folder.
4. Selectează sarcinile opționale dorite: scurtătură pe Desktop, asocieri de fișiere, `%PATH%`, meniu contextual în Explorer.
5. Clic pe **Instalează**, apoi pe **Finalizează**.

Nu este necesară nicio solicitare UAC. Installerul scrie doar în profilul tău de utilizator și în `HKCU`.

### 3. Prima lansare

- Code::Blocks pornește cu tema Dark și cu localizarea selectată.
- Dacă nu este găsit niciun compilator extern, instalează **MSYS2 UCRT64** sau **MinGW-w64**, apoi configurează-l la `Settings → Compiler → Toolchain executables`.
- O cale tipică pentru MSYS2 UCRT64 este `C:\msys64\ucrt64\bin`.
- IDE-ul poate detecta automat un compilator instalat corect la prima rulare.

## Documentație și note de release

Jurnalul complet de modificări este împărțit pe limbi:

| Fișier | Limbă | Conținut |
| --- | --- | --- |
| [MODIFICATIONS_RO.md](MODIFICATIONS_RO.md) | Română | Jurnalul complet în română |
| [MODIFICATIONS_EN.md](MODIFICATIONS_EN.md) | English | Jurnalul complet în engleză |

Notele de release acoperă reproiectarea vizuală, implementarea temei întunecate native, migrarea la wxWidgets 3.2, optimizările de performanță din Etapa 1 (încărcarea amânată a workspace-ului, aplicarea temei într-o singură trecere, scrierea logurilor în loturi, cache-ul setărilor compilatorului), actualizările paginii Start Here și verificarea finală a build-ului.

Pagina **Finalizează** a installerului poate deschide notele de release pentru limba selectată în timpul instalării.

Pentru instrucțiuni de compilare, vezi [BUILD](BUILD). Pentru textul licenței upstream, vezi [COPYING](COPYING).

## Credite și licență

**Creat și optimizat de tudor62.**

- Nucleul original Code::Blocks, SDK-ul și arhitectura de plugin-uri: Yiannis Mandravellos și contribuitorii Code::Blocks.
- Construit cu wxWidgets 3.2.11 și GCC 16.1.0 (MSYS2 UCRT64).
- Iconițele moderne, splash screen-ul și elementele Start Here au fost proiectate pentru această ediție.

**Licență:** GNU General Public License v3.0 — vezi [COPYING](COPYING).

Aceasta este o ediție personalizată independentă și neoficială. Nu este afiliată și nu este aprobată de proiectul oficial Code::Blocks. Code::Blocks rămâne proprietatea autorilor și contribuitorilor săi originali.

---

<div align="center">

[English](README.md) · **Română**

<sub>Întreținut de tudor62 · Construit pentru comunitatea C/C++</sub>

</div>
