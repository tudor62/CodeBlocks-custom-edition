# Modificări efectuate

## 1) Iconițe bara de instrumente (toolbar) - 16x16 și 32x32
Am actualizat următoarele iconițe din `src/src/resources/images/` păstrând **numele și dimensiunile originale**:

### `images/16x16/`
- `filenew.png` (16x16)
- `fileopen.png` (16x16)
- `filesave.png` (16x16)
- `editcopy.png` (16x16)
- `editcut.png` (16x16)
- `editpaste.png` (16x16)
- `undo.png` (16x16)
- `redo.png` (16x16)
- `searchreplace.png` (16x16)

### `images/32x32/`
- `filenew.png` (32x32)
- `fileopen.png` (32x32)
- `filesave.png` (32x32)
- `editcopy.png` (32x32)
- `editcut.png` (32x32)
- `editpaste.png` (32x32)
- `undo.png` (32x32)
- `redo.png` (32x32)
- `searchreplace.png` (32x32)

Design modernizat: contur curat, accent albastru pentru acțiuni, aspect coerent. Script folosit: `scripts_gen_icons.js`.

## 2) Pagina "Start Here" - `src/src/resources/start_here/`
Am actualizat următoarele iconițe păstrând **numele și dimensiunile originale**:

- `new.png` (47x42) - document modern cu simbol `+` albastru, sugestiv pentru creare proiect/fișier
- `open.png` (47x47) - folder modern cu săgeată albastră care indică deschiderea, mai sugestiv și coerent vizual

Script folosit: `scripts_gen_start_here.js`.

## 3) Butoane mari din images/ (root)
Am actualizat următoarele iconițe din `src/src/resources/images/` păstrând **numele și dimensiunile originale**:

- `closebutton.png` (12x12) - simbol X modernizat, mai curat
- `compile.png` (22x22) - roată/engrenaj modern în albastru, sugestiv pentru compilare
- `compilerun.png` (22x22) - combinație roată + săgeată verde, sugestiv pentru compilare și execuție

Script folosit: `scripts_gen_big_buttons.js`.

## 4) Folder - `src/src/resources/images/folder.png`
- `folder.png` (22x22) - folder modern în stil flat, cu colțuri rotunjite, gradient albastru și highlight subtil pe muchia de sus (desenat cu anti-aliasing prin supersampling)

Script folosit: `scripts_gen_folder.js`.

## 5) Stop - `src/src/resources/images/stop.png`
- `stop.png` (22x22) - redesign: pătrat rotunjit umplut roșu (#EF4444 → #B91C1C) cu un X alb centrat — metafora modernă și clară de „stop”

Script folosit: `scripts_gen_stop.js`.

## 6) Temă aplicație Light/Dark (Full Dark Theme)
- Opțiune în `Settings -> Environment -> View`, secțiunea „Application theme” (`wxChoice name="chTheme"`).
- Valoarea este salvată în config în namespace-ul `app`, cheia `/environment/theme` (`light` / `dark`), implicit `light`.
- Paleta exactă folosită de tema Dark:
  - bară de meniu, bară de titlu, tab-uri inactive: `#0B0E12`;
  - panouri laterale și editor Scintilla: `#13181D`;
  - panoul „Logs & others”: `#0F1318`;
  - selecție/element activ: `#1E293B` / `#1F2D3D`;
  - text principal/cod: `#E2E8F0` / `#D4D4D4`;
  - linkuri Start Here: `#38BDF8`.
- Pe Windows se activează modul dark nativ prin `uxtheme.dll`/`dwmapi.dll` încărcate dinamic (compatibil wxWidgets 3.2):
  - `SetPreferredAppMode(2)`, `AllowDarkModeForWindow()`, `SetWindowTheme()` și `DwmSetWindowAttribute(DWMWA_USE_IMMERSIVE_DARK_MODE)`;
  - pe Windows 11 build >= 26200 se folosește `DarkMode_DarkTheme`; pe versiuni mai vechi `DarkMode_CFD` pentru edit/combo și `DarkMode_Explorer` pentru restul;
  - scrollbar-ele native, listele, tree-urile, combobox-urile, spin-urile și dialogurile primesc varianta dark.
- Bara de meniu este desenată de aplicație prin mesajele UAH folosite și de wxWidgets 3.3 (`WM_MENUBAR_DRAWMENU` 0x91, `WM_MENUBAR_DRAWMENUITEM` 0x92, `WM_UAHMEASUREMENUITEM` 0x94): fundal `#0B0E12`, text `#E2E8F0`, element activ `#1E293B`. Meniurile popup rămân desenate nativ dark de Windows.
- Bara de instrumente are fundal `#0B0E12`; iconițele `wxBitmapBundle`/SVG rămân vizibile.
- Tab-urile folosesc un art provider propriu (`NbStyleDark`, derivat din `wxAuiGenericTabArt`) pentru a evita orice fallback nativ alb: tab activ `#13181D`, tab-uri inactive `#0B0E12`, text `#94A3B8`, buton de închidere `#94A3B8`.
- Panourile AUI primesc fundal/sash/border/caption dark în `MainFrame::ApplyTheme()`; la revenirea pe Light, culorile configurabile sunt recitite din config, iar celelalte sunt restaurate din `wxAuiDefaultDockArt`.
- Controalele native care nu au temă dark completă sunt desenate parțial de aplicație:
  - `SysHeader32` prin `NM_CUSTOMDRAW` (fundal `#0B0E12`, text `#E2E8F0`, bordură `#1E293B`, filler după ultima coloană);
  - `wxStaticLine` prin `WM_NCPAINT` (linia de 2 px este integral non-client);
  - `wxSlider` prin `NM_CUSTOMDRAW` pentru canal (`#1E293B`), păstrând thumb-ul nativ albastru;
  - câmpuri `Edit`/`ComboBox` dezactivate prin `WM_CTLCOLORSTATIC`/`WM_CTLCOLOREDIT`/`WM_CTLCOLORLISTBOX`.
- `cbMessageBox()` folosește `wxGenericMessageDialog` când tema Dark este activă, apoi aplică tema și afișează dialogul; mesajele Information/Question nu mai apar albe. În Light rămâne dialogul nativ.
- La comutarea Dark -> Light, `cbApplyAppThemeToWindow()` restaurează explicit culorile de sistem pentru tree/list (`TVM_SETBKCOLOR`, `TVM_SETTEXTCOLOR`, `LVM_SETBKCOLOR`, `LVM_SETTEXTCOLOR`, `LVM_SETTEXTBKCOLOR`) și pentru panouri, astfel încât Management/tree și celelalte liste să redevindă albe cu text închis.
- Schimbarea temei este aplicată imediat pentru widgeturi; meniul nativ, bara de titlu și unele controale comune se stabilizează complet după repornire, de aceea este afișat promptul „Code::Blocks needs to be restarted for the changes to take effect.”.
- Editorul Scintilla folosește tema dark: cuvinte cheie cyan `#38BDF8`, tipuri albastru-deschis, numere/constante roz-magenta `#F472B6`, cuvinte cheie de control albastru-violet, comentarii verde subtil `#4ADE80`; marginile și fundalul sunt `#13181D`.
- Log-urile au fundal `#0F1318` și text luminos; log-ul de build afișează ieșirea GCC lizibil.
- Pagina „Start here” folosește fundal dark, text `#D4D4D4` și linkuri `#38BDF8`.

Fișiere modificate:
- `src/src/resources/env_settings.xrc` - `wxChoice` `chTheme` (Light/Dark)
- `src/src/environmentsettingsdlg.cpp` - citire/scriere `/environment/theme`
- `src/include/globals.h`, `src/sdk/globals.cpp` - paleta `cbTheme`, `cbAppTheme`, `cbGetAppTheme()`, `cbIsAppThemeDark()`, `cbApplyNativeTheme()`, `cbApplyNativeAppTheme()`, `cbApplyAppThemeToWindow()`, `cbApplyThemeToAuiNotebooks()`, subclass-urile Windows pentru header/static line/slider/controle native, bara de meniu UAH și `cbMessageBox()` dark
- `src/src/main.h`, `src/src/main.cpp` - `MainFrame::ApplyTheme()`, `DoUpdateLayoutColours()`, `DoUpdateEditorStyle()`, mesajele UAH pentru meniu, restaurarea completă a culorilor la trecerea pe Light, promptul de repornire
- `src/src/app.cpp` - filtrarea evenimentelor pentru ferestrele/dialogurile noi și aplicarea temei native la startup
- `src/src/notebookstyles.cpp`, `src/src/notebookstyles.h` - `NbStyleDark` pentru tab-uri
- `src/sdk/editorcolourset.cpp` - paleta Scintilla dark și culorile marginilor
- `src/sdk/loggers.cpp` - fundalurile și culorile log-urilor
- `src/src/startherepage.cpp` - culorile paginii „Start here”, inclusiv linkurile `#38BDF8`

## 7) Observații generale
- Toate modificările de iconițe au fost efectuate prin desenare programatică (fără scalare brutală), păstrând dimensiunile exacte și numele fișierelor originale.
- Modificările de temă afectează fișiere sursă (`.cpp`, `.h`, `.xrc`); restul modificărilor au fost doar iconițe.
- Există seturi multiple de iconițe pe dimensiuni (`16x16` până la `64x64`, `80x80`) pentru suport HiDPI/scaling - aceștia sunt redați separat pentru lizibilitate la fiecare dimensiune, nu simple copii scalate.

## 8) Portare la wxWidgets 3.2.x: API-uri moderne, High-DPI și dark mode nativ

### 8.1 Iconițe prin `wxBitmapBundle`, SVG și suport High-DPI

- `src/sdk/globals.cpp`
  - `cbLoadBitmapBundle()`: parametrul `wxFileSystem*` nu mai este ignorat; `bitmapType` și filesystem-ul sunt transmise corect către `cbLoadBitmap()`. Se încearcă toate variantele `16x16 ... 64x64`, iar dacă imaginea nu are directoare pe dimensiuni se returnează un bundle cu un singur bitmap (în loc de un bundle invalid).
  - `cbLoadBitmapBundleFromSVG()`: folosește `wxBitmapBundle::FromSVG()` și oferă scalare vectorială la orice factor DPI. Este păstrat ca punct unic de încărcare pentru SVG.
- `src/include/xtra_res.h` și `src/sdk/xtra_res.cpp`
  - Adăugat `wxToolBarAddOnXmlHandler::GetCenteredBitmapBundle()`, care construiește un `wxBitmapBundle` pentru resursele XRC de toolbar:
    - preferă SVG-ul din directorul `svg/` când există;
    - altfel încarcă toate variantele PNG `16x16 ... 64x64`;
    - pentru imagini fără directoare pe dimensiuni creează un bundle dintr-un singur bitmap;
    - pentru `stock_id` folosește `wxArtProvider::GetBitmapBundle()`, astfel încât și iconițele de art provider să fie scalabile.
  - `wxToolBarAddOnXmlHandler::DoCreateResource()` apelează supraîncărcarea modernă `wxToolBar::AddTool(..., const wxBitmapBundle&, ...)`.
  - `GetCenteredBitmap()` este păstrat pentru compatibilitate cu wxWidgets < 3.1.6 și ca fallback.
- `src/src/main.cpp`
  - `MainFrame::AddToolbarItem()`: nu mai apelează direct `cbLoadBitmap()` + `AddTool(wxBitmap)`. Construiește un bundle SVG (preferat) sau un bundle multi-size PNG și îl transmite lui `wxToolBar::AddTool()`. Astfel bara de instrumente principală urmează automat scara DPI.
- `wxImageList` nu acceptă `wxBitmapBundle` în wxWidgets 3.2; pentru listele de imagini se păstrează `wxBitmap` împreună cu `cbMakeScaledImageList()` și `cbAddBitmapToImageList()`, care țin cont de factorul de scalare. Bundle-urile sunt folosite acolo unde API-ul le suportă nativ (toolbar, meniuri, butoane, art provider).
- Call-site-uri deja modernizate în codul existent (verificate în această etapă): `src/plugins/compilergcc/compilergcc.cpp`, `src/plugins/codecompletion/codecompletion.cpp`, `src/plugins/abbreviations/abbreviations.cpp`, `src/src/breakpointsdlg.cpp` etc. Acestea folosesc deja `cbLoadBitmapBundleFromSVG()`/`wxBitmapBundle` pentru iconițele de log, listă și notebook.

### 8.2 Dark mode nativ pe Windows (UCRT64/MinGW)

- `src/include/globals.h`
  - Adăugat `cbApplyNativeTheme(wxWindow* window, bool dark)`.
- `src/sdk/globals.cpp`
  - Implementare Windows care încarcă dinamic `dwmapi.dll` și `uxtheme.dll` (fără dependență suplimentară de linkare):
    - `DwmSetWindowAttribute()` cu atributul `DWMWA_USE_IMMERSIVE_DARK_MODE` (20 pe Windows 10 2004+, fallback 19) pentru title bar dark pe ferestrele top-level;
    - `SetWindowTheme(hwnd, L"DarkMode_Explorer", nullptr)` pentru scrollbar-ele native ale controalelor care le folosesc (`TextCtrl`, `ListCtrl`, `TreeCtrl`, `Scintilla`, `RichEdit`, `ListBox`, `ComboBox`, `PropertyGrid`, `DataView`, `Grid`, `ScrollBar`);
    - la revenirea pe tema light se aplică `L"Explorer"`, revenind la aspectul normal.
  - Pe platformele non-Windows funcția este un no-op, deci codul rămâne portabil.
- `src/src/main.cpp`
  - `ApplyThemeToWindowTree()` apelează `cbApplyNativeTheme()` pentru fiecare fereastră din arbore.
  - `MainFrame::ApplyTheme()` parcurge `wxTopLevelWindows` și aplică decorarea nativă și dialogurilor și panourilor flotante AUI (acestea nu sunt copii ai ferestrei principale).
  - `MainFrame` re-aplică decorarea nativă la `wxEVT_SHOW`: pe Windows, `DWMWA_USE_IMMERSIVE_DARK_MODE` poate fi ignorat cât timp fereastra este încă ascunsă, astfel încât title bar-ul putea rămâne deschis la prima afișare.
  - `src/sdk/cbeditor.cpp`: `cbEditor::CreateEditor()` aplică tema nativă fiecărui `cbStyledTextCtrl` nou creat, astfel încât scrollbar-ele editorelor deschise după pornire să fie și ele întunecate.
  - Meniurile și bara de instrumente sunt tematizate complet în această implementare: meniul prin mesajele UAH, iar toolbar-ul prin fundal `#0B0E12`. Pe wx 3.3+ `wxApp::SetAppearance()` poate acoperi suplimentar elementele native, dar implementarea de aici funcționează pe wx 3.2.
- Randare editor: integrarea Direct2D este păstrată. `src/sdk/cbeditor.cpp` citește `/technology` (implicit `1`) și apelează `SetTechnology(wxSCI_TECHNOLOGY_DIRECTWRITE)`; `src/sdk/wxscintilla/src/ScintillaWX.cpp` tratează `SCI_SETTECHNOLOGY` neschimbat. Modernizarea nu dezactivează și nu ocolește `wxGraphicsContext`/Direct2D.

### 8.3 Trecerea de la `Connect()`/macro-uri la `.Bind()`

- `src/src/main.cpp`: cele patru conexiuni pentru `wxEVT_COMMAND_TOOL_RCLICKED` au fost convertite la:
  - `m_pToolbar->Bind(wxEVT_TOOL_RCLICKED, &MainFrame::OnToolBarRightClick, this);`
  - `m_debuggerToolbarHandler->GetToolbar()->Bind(...)`
  - cele două apeluri din `CreateToolbars()` și `DoAddPlugin()` pentru toolbar-urile pluginurilor.
- S-a folosit noul nume de eveniment `wxEVT_TOOL_RCLICKED` în locul aliasului vechi.
- Tabelele de evenimente `wxBEGIN_EVENT_TABLE`/`EVT_*` rămase **nu sunt depreciate în wxWidgets 3.2** și conversia lor integrală ar introduce riscuri mari în sute de fișiere fără beneficiu funcțional. În codul atins de această modernizare s-a folosit `.Bind()`; restul tabelelor au fost lăsate intenționat, conform principiului „unde este necesar pentru claritate și siguranță”.

### 8.4 Include-uri și tipuri C++ moderne

- `src/include/prep.h`: `<stdint.h>` → `<cstdint>`.
- `src/tools/cbp2make/lib/stringhash.h`: `<stdint.h>` → `<cstdint>`.
- `src/plugins/contrib/ThreadSearch/ThreadSearchViewManagerBase.h`: `<stdint.h>` → `<cstdint>`.
- `src/include/globals.h` folosește deja `<cstdint>` pentru `int64_t`/`uint64_t` (typedef-ul `GlobId` și interfețele debuggerului).
- Nu au fost introduse tipuri proprietare sau typedef-uri redundante; codul folosește în continuare `int64_t`/`uint64_t` din `<cstdint>`.

### 8.5 API-uri depreciate în wxWidgets 3.2

- `src/sdk/scripting/bindings/sc_consts.cpp`: `wxPATH_NORM_ALL` este marcat depreciat în wx 3.2. A fost înlocuit cu combinația explicită `wxPATH_NORM_ENV_VARS | wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT`, păstrând aceleași valori și același nume de constantă expus scripturilor Squirrel (`wxPATH_NORM_ALL`) prin `BIND_INT_CONSTANT_NAMED`.
- Încărcarea iconițelor de toolbar nu mai folosește `wxBitmap` acolo unde există supraîncărcare `wxBitmapBundle`.
- În urma build-ului cu wxWidgets 3.2.11 nu au mai apărut avertismente de API depreciat în modulele recompilate.

### 8.6 Verificare build și instalare

- Build complet, de la zero, cu MSYS2 UCRT64, g++ 16.1.0 și wxWidgets 3.2.11 (`wx-config --version` = 3.2.11):
  - `make clean` la nivel de arbore → `CLEAN_EXIT=0`
  - `make -j12` pentru întregul arbore (SDK, aplicație, pluginuri, unelte) → `BUILD_EXIT=0`
  - relink final al aplicației cu `-Wl,--no-undefined -mwindows` → `RELINK_EXIT=0`; `objdump -p` raportează `Subsystem 00000002 (Windows GUI)`
  - `make install` → `INSTALL_EXIT=0`
- Kitul funcțional este instalat în `C:\Users\tudor\Desktop\CodeBlocks wx 3.2` (executabil: `bin\codeblocks.exe`, DLL-uri runtime wxWidgets/MinGW în `bin`, pluginuri în `lib\codeblocks\plugins`, resurse în `share\codeblocks`).
- Verificare pe Windows 11 build 26200: tema **Dark** aplică panourile întunecate, meniul/toolbar-ul dark, tab-urile dark, title bar-ul nativ dark și dialogurile dark; tema **Light** revine complet la aspectul normal (title bar deschis, panouri albe, tree/list albe, captions AUI implicite). Comutarea live Dark -> Light -> Dark a fost verificată, iar mesajele Information/Question sunt dark în tema Dark.

## Fișiere create pentru generare
- `scripts_gen_icons.js` - generator pentru iconițele din toolbar (16x16, 32x32)
- `scripts_gen_start_here.js` - generator pentru iconițele din start_here (47x42, 47x47)
- `scripts_gen_big_buttons.js` - generator pentru closebutton, compile, compilerun
- `scripts_gen_folder.js` - generator pentru `folder.png`
- `scripts_gen_stop.js` - generator pentru `stop.png`

## 9) Splash screen modern si iconite Start Here SVG

- `src/src/resources/images/splash_2603.png` - noul splash screen (1671x941). Fereastra de pornire il incarca si il scaleaza proportional la maximum 800x450, apoi il afiseaza centrat pe ecran, cu aceleasi optiuni ca inainte (`wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT`). Imaginea veche `splash_1312.png` a fost pastrata.
- `src/src/resources/start_here/title_2603.png` - bannerul modern pentru pagina Start Here, redimensionat la 440x248 pentru afisare.
- `src/src/resources/start_here/new.svg`, `open.svg`, `reopen.svg`, `tip.svg`, `trash.svg` - iconite vectoriale noi, cu aceleasi nume ca vechile PNG-uri; vechile fisiere PNG au fost pastrate ca fallback.
- `src/src/resources/start_here/start_here.html` - referinte actualizate catre `title_2603.png` si iconitele SVG (40x40 pentru actiuni, 16x16 pentru cosul de gunoi).
- `src/src/app.cpp` - foloseste `splash_2603.png`; nu mai deseneaza textul vechi de release peste imaginea noua.
- `src/src/dlgabout.cpp` - dialogul About foloseste aceeasi imagine, scalata la maximum 480x270.
- `src/include/globals.h`, `src/sdk/globals.cpp` - adaugata functia `cbScaleBitmapToFit()` pentru scalare proportionala de calitate.
- `src/src/startherepage.cpp` - `wxHtmlWindow` nu afiseaza SVG nativ in wxWidgets 3.2. `MyHtmlWin::OnOpeningURL()` intercepteaza cererile `.svg`, randeaza iconita prin `wxBitmapBundle::FromSVG()` si redirectioneaza catre un PNG temporar. `currentColor` este inlocuit cu `#E6E6E6` pe tema Dark si `#333333` pe Light; randarea se face la 4x dimensiunea de afisare pentru claritate High-DPI, iar fisierele temporare sunt sterse la distrugerea ferestrei.
- `src/src/resources/Makefile.am`, `Makefile.in`, `Makefile` - `start_here.zip` include acum si fisierele `*.svg`.
- `src/src/Makefile.am`, `Makefile.in`, `Makefile` - adaugate caile catre `src/exchndl/win64/include` si `src/exchndl/win64/lib`, astfel incat `exchndl.h` si `libexchndl.a` sa fie gasite la compilare si link fara variabile de mediu suplimentare.

Rezultat: splash-ul nou este centrat si scalat corect, bannerul Start Here foloseste imaginea moderna, iar iconitele Start Here sunt vectoriale si se adapteaza temei Light/Dark.

## 10) Verificare finala Full Dark Theme (27.10.2026)

- Build final cu `make -C src/sdk -j8` si `make -C src/src -j8` (MSYS2 UCRT64, wxWidgets 3.2.11, GCC 16.1.0), fara erori.
- `codeblocks.exe` si `libcodeblocks-0.dll` actualizate in kit-ul `C:\Users\tudor\Desktop\CodeBlocks wx 3.2\bin` si in prefixul `C:\Users\tudor\Desktop\CodeBlocks_modern_2026\cb-install\bin`.
- Verificari live pe Windows 11 build 26200:
  - meniu dark: bara `#0B0E12`, text `#E2E8F0`, element activ `#1E293B`; popup-uri native dark;
  - toolbar `#0B0E12`, iconite lizibile;
  - tab-uri: activ `#13181D`, inactiv `#0B0E12`, text `#94A3B8`; fara borduri albe;
  - editor `#13181D`; sintaxa Scintilla cu `#38BDF8`, `#A78BFA`, `#4ADE80`, `#FBBF24`, `#C084FC`, `#F472B6`;
  - Management/tree `#13181D`, Start Here linkuri `#38BDF8`;
  - "Logs & others" `#0F1318`; Build messages header `#0B0E12`, text `#E2E8F0`, filler `#0B0E12`;
  - linii statice `#1E293B`; slider canal `#1E293B`, thumb albastru `#0078D7`;
  - dialoguri Environment settings / Editor configuration dark, inclusiv campuri enabled/disabled si scrollbar-uri `#171717`;
  - mesaje Information/Question dark in tema Dark.
- Comutare live Dark -> Light -> Dark: Light readuce complet panourile, tree/list, tab-urile, editorul, log-urile si captions AUI la aspectul nativ; Dark readuce UI-ul intunecat. Promptul de repornire apare in ambele sensuri si este dark cand tema activa este Dark.
- Pornire si inchidere curata, fara procese Code::Blocks ramase; fara suprafete albe in tema Dark.

## 11) Executabil GUI fără consolă (27.10.2026)

- `src/src/Makefile.am`, `src/src/Makefile.in`, `src/src/Makefile`:
  - în blocul condiționat `CODEBLOCKS_NT` a fost adăugat `AM_LDFLAGS += -mwindows`;
  - `-mwindows` se aplică numai executabilului principal `codeblocks.exe`, nu întregului arbore;
  - `src/tools/ConsoleRunner` rămâne neschimbat și este construit în continuare ca aplicație de consolă.
- `src/CodeBlocks_wx31.cbp`, `src/CodeBlocks_wx31_64.cbp`, `src/CodeBlocks_wx32.cbp`, `src/CodeBlocks_wx32_64.cbp`:
  - opțiunea `-mwindows` a fost adăugată la Linker pentru target-ul `src`, astfel încât și rebuild-urile din workspace-ul Code::Blocks să producă executabil GUI.
- `src/src/app.cpp`:
  - `InitDebugConsole()` nu mai sugerează compilarea ca aplicație de consolă; `AllocConsole()` rămâne disponibil exclusiv în build-urile de debug (`__CBDEBUG__`);
  - în build-ul release nu se alocă și nu se atașează nicio consolă la pornire.
- Logurile de la pornire și de la build sunt afișate în panoul intern `Logs & others` (taburile `Code::Blocks`, `Build log`, `Build messages`, `Debugger`); nu se mai redirecționează către o consolă externă.
- Rularea unui proiect consolă folosește în continuare `cb_console_runner.exe`, care rămâne aplicație de consolă; fereastra programului utilizatorului se deschide separat doar în timpul execuției.
- Build-urile batch lansate din terminal (`codeblocks.exe --build`) folosesc fereastra internă de batch build; output-ul nu mai este scris în consola terminalului.
- Verificări:
  - `objdump -p src/src/.libs/codeblocks.exe` → `Subsystem 00000002 (Windows GUI)`;
  - `objdump -p src/tools/ConsoleRunner/cb_console_runner.exe` → `Subsystem 00000003 (Windows CUI)`;
  - lansare din Explorer: fereastra principală pornește fără nicio fereastră de consolă; panoul `Logs & others` este prezent și funcțional;
  - `codeblocks.exe` actualizat în kit-ul Desktop și în `cb-install`, cu hash SHA-256 identic.

## 12) Iconițe și logo noi (28.10.2026)

- Set nou de iconițe preluat din `C:\Users\tudor\Desktop\Imagini`, păstrând 1:1 numele fișierelor destinație:
  - iconițe compilator în `src/plugins/compilergcc/resources/images/svg/`:
    - `compile.svg` (288.062 B);
    - `compilerun.svg` (219.535 B, redenumit din `compilerrun.svg`);
    - `rebuild.svg` (64.931 B);
    - `run.svg` (89.401 B);
    - `stop.svg` (109.935 B).
  - iconițe generale în `src/src/resources/images/svg/`:
    - `fileprint.svg` (1.193 B), `filesave.svg` (1.079 B), `filesaveall.svg` (2.696 B), `filesaveas.svg` (2.086 B), `idea.svg` (1.381 B), `info.svg` (537 B), `redo.svg` (3.402 B), `undo.svg` (3.229 B).
- Fundalul opac / modelul de checkerboard inclus în `compile.svg`, `run.svg` și `stop.svg` a fost eliminat prin re-vectorizare cu fundal transparent; `compilerun.svg` și `rebuild.svg` au fost curate din sursă. Iconițele se afișează corect pe tema Dark, fără pătrățele gri.
- `resources.zip` și `compiler.zip` au fost regenerate cu fișierele SVG noi; vechile PNG-uri au rămas în arhive ca fallback pentru wxWidgets < 3.1.6.
- Logo:
  - `new_logo.png` a fost decupat, i s-a eliminat fundalul punctat închis și a fost convertit la RGBA cu transparență;
  - a fost generat `app.ico` cu intrări 16/24/32/48 DIB 32bpp + 64/128/256 PNG (110.252 B), păstrând stilul ICO original;
  - `src/src/resources/icons/app.ico` a fost înlocuit; `resources.o` a fost șters pentru ca `windres` să reîncarce iconița (`app.ico` nu este dependență make declarată);
  - iconița este vizibilă în bara de titlu, în Explorer și în taskbar.
- Build final:
  - `make -j$(nproc)` în `src/src` → `MAKE_EXIT=0`;
  - `src/src/.libs/codeblocks.exe`: 4.189.522 B, SHA-256 `54A5BD96A03F1A237C869777CF39DD855D00BB45B7BA1634777A05DAF72D1660`;
  - `objdump -p` → `Subsystem 00000002 (Windows GUI)`; fără consolă la pornire;
  - `cb_console_runner.exe` rămâne `Subsystem 00000003 (Windows CUI)`.
- Kit Desktop actualizat:
  - `bin/codeblocks.exe` (hash identic cu build-ul);
  - `share/codeblocks/resources.zip` (2.682.219 B, SHA-256 `6F90151288F46186DBE762ED960EFA2597ECB7193C9DCD874D45D2B5B1257C99`);
  - `share/codeblocks/compiler.zip` (374.984 B, SHA-256 `924B82158DBAE704CB21D9DAF788936B122C4781F08D650B396CE642A86DF9D5`).
- `cb-install` a fost sincronizat cu aceleași fișiere (executabil + cele două arhive), pentru ca prefixul de instalare folosit la link să rămână identic cu kit-ul.
- Verificare live pe Windows 11 build 26200, tema Dark: toolbar-ul compilatorului afișează iconițele noi (compile, run, build and run, rebuild, stop), toolbar-ul principal folosește iconițele SVG noi, bara de titlu afișează noul logo, iar aplicația pornește fără fereastră de consolă. Iconițele sunt transparente pe fundalul dark.
- Backup pentru fișierele originale: `C:\Users\tudor\AppData\Local\Temp\opencode\backup_resources_20261028_171106` (SVG-uri + `app.ico` + arhivele vechi).

## 13) Optimizări Etapa 1 - pornire, temă, loguri și cache compilator (28.10.2026)

### 13.1 P0.1 - încărcarea workspace-ului după afișarea ferestrei

- `src/src/app.cpp`:
  - când există un workspace de încărcat, `LoadWorkspace()` este amânat cu `CallAfter()` după `frame->Show()`, `StartupDone()` și `frame->Update()`;
  - finalizarea pornirii (`LoadDelayedFiles()`, `AttachDebugger()`, `WorkspaceChanged()`, `SetAppStartedUp(true)`, evenimentul `cbEVT_APP_STARTUP_DONE` și mesajul crash report) a fost mutată în lambda-ul `finishStartup`, apelat imediat după încărcarea workspace-ului;
  - `s_Loading` este curățat în `finishStartup`, nu înainte de încărcarea workspace-ului.
- Efect: fereastra principală devine vizibilă în ~23-25 s într-un mediu cu pornire lentă, iar un workspace de stress cu 150 de proiecte se încarcă în continuare după afișare; arborele de proiecte și titlul se completează ulterior. Fără înghețarea interfeței în timpul parsării workspace-ului.

### 13.2 P0.4 - o singură trecere de temă la pornire

- `src/src/app.cpp`: filtrul de evenimente `cbAppThemeFilter` nu mai aplică tema pentru `MainFrame`; fereastra principală este tematizată o singură dată, în constructorul ei.
- `src/src/main.cpp`:
  - `MainFrame::ApplyTheme()` rulează o singură dată la finalul constructorului `MainFrame` și folosește `Freeze()`/`Thaw()` pe fereastră și pe panourile AUI;
  - nu mai există `Refresh()` per control, per meniu sau per status bar; la final se face o singură reîmprospătare globală;
  - panourile flotante AUI sunt tematizate explicit;
  - `DoUpdateLayoutColours()` apelează `ApplyTheme()` doar după `m_StartupDone`, pentru a evita o a doua trecere în timpul pornirii;
  - `StartupDone()` setează doar flag-ul, apelează `DoUpdateLayout()` și `m_LayoutManager.Update()`.
- `src/sdk/globals.cpp`:
  - `cbApplyAppThemeToWindow()` îngheață fereastra top-level pe durata parcursului recursiv și nu mai reîmprospătează fiecare control;
  - `cbApplyThemeToAuiNotebooks()` nu mai apelează `Refresh()` pe notebook-uri;
  - `ApplyNativeThemeToTopLevelWindows()` sare peste fereastra principală.
- `src/src/startherepage.cpp`: în tema Dark, pagina Start Here și `wxHtmlWindow` primesc fundalul `cbTheme::Panel()` încă din constructor, înainte de prima pictare.
- Abatere deliberată de la cerință: trecerea unică de temă rulează la finalul constructorului `MainFrame`, înainte de `frame->Show()`, nu după `Show()`. Varianta cu tema după `Show()` producea un flash alb vizibil; varianta cu `Freeze()` exterior în jurul lui `Show()` producea o zonă client albă/goală. `frame->Update()` este apelat în `app.cpp` imediat după `StartupDone()` pentru a forța prima pictare completă. În continuare, pagina Start Here poate apărea albă sub 200 ms la prima pictare (măsurat); bara de titlu, meniurile, toolbar-ul și panourile sunt deja dark. `StartupDone()` înainte de `Show()` și înghețarea exterioară au fost încercate și revertite pentru că înrăutățeau primul cadru.

### 13.3 P1.1 - scrierea logurilor în loturi

- `src/sdk/loggers.cpp`, `src/include/loggers.h`:
  - fiecare logger păstrează o coadă de mesaje în așteptare și un timestamp;
  - un `wxTimer` partajat de 50 ms golește cozile;
  - un singur `Freeze()`/`Thaw()` și o singură auto-derulare la finalul fiecărui lot, în loc de una per mesaj;
  - `Clear()` aruncă mesajele în așteptare;
  - operațiile de copiere și de numărare forțează golirea cozii, ca să nu lipsească mesaje;
  - `UpdateSettings()` golește coada și aplică noile stiluri;
  - `CompilerMessages::FocusError()` golește coada înainte de selectarea rândului cu eroare.
- Layout-ul claselor de logger a fost păstrat pentru compatibilitatea ABI cu pluginurile.
- Verificare funcțională cu un proiect care generează 300 de warning-uri:
  - Build log: 916 linii / 51.602 caractere, toate cele 600 de apariții „batch line” (300 warning-uri × 2) în ordine, fără pierderi sau reordonări;
  - Build messages: 302 rânduri (300 warning-uri + sumar);
  - „Copy contents to clipboard”: 300 warning-uri în clipboard;
  - „Clear contents”: lista ajunge la 0 rânduri;
  - Build log auto-derulat la final: `firstVisible=904` din 910 linii, cu liniile „Process terminated” și „0 error(s), 300 warning(s)” vizibile.

### 13.4 P1.3 - cache pentru `/ignore_output` și `/max_reported_errors`

- `src/plugins/compilergcc/compilergcc.h`, `compilergcc.cpp`:
  - cache `m_CachedIgnoreOutput` și `m_CachedMaxReportedErrors`, protejat de `wxCriticalSection`;
  - `EnsureCompilerConfigCache()` încarcă setările o singură dată, la prima ieșire de compilator;
  - `RefreshCompilerConfigCache()` este apelat la începutul build-ului (`DoPrepareQueue()`) și în `CompileFileDefault()`;
  - `InvalidateCompilerConfigCache()` este apelat din `CompilerOptionsDlg::OnApply()`.
- Verificare live în personalitatea izolată `cachetest`:
  - setări: max=5, ignore=`IGNORE_THIS_LINE` → Build messages afișează exact 5 erori, linia ignorată lipsește;
  - setări schimbate fără repornire: max=4, ignore=`undeclared_2` → Build messages afișează 4 erori, `IGNORE_THIS_LINE` reapare, `undeclared_2` este filtrat;
  - deci invalidarea la salvare și reîncărcarea la începutul build-ului funcționează în același proces.

### 13.5 Fișiere modificate

- `src/src/app.cpp` - încărcare workspace amânată, `frame->Update()` după `StartupDone()`, excluderea `MainFrame` din filtrul de temă.
- `src/src/main.cpp` - trecere unică de temă în constructor, `StartupDone()`, `ApplyTheme()`, `DoUpdateLayoutColours()`.
- `src/src/startherepage.cpp` - fundal dark în constructor pentru pagina Start Here.
- `src/sdk/globals.cpp` - aplicare recursivă de temă cu `Freeze()`/`Thaw()` și o singură reîmprospătare.
- `src/sdk/loggers.cpp`, `src/include/loggers.h` - loguri în loturi.
- `src/plugins/compilergcc/compilergcc.cpp`, `compilergcc.h` - cache pentru setările compilatorului.
- `src/plugins/compilergcc/compileroptionsdlg.cpp` - invalidarea cache-ului la salvare.
- `src/plugins/compilergcc/compilermessages.cpp` - golirea cozii înainte de `FocusError()`.

### 13.6 Build, instalare și verificări finale

- Build curat cu MSYS2 UCRT64, wxWidgets 3.2.11, GCC 16.1.0: SDK, `codeblocks.exe` și pluginul compilator, fără erori.
- Binare finale (SHA-256):
  - `codeblocks.exe`: `C3C6D2325FF20162ED2E98257ECB087DCA4E1A5BB71C4EE7BB0E9A569C56B8FC` (4.192.893 B);
  - `libcodeblocks-0.dll`: `77DCAFA6DB408DEAFE887CE2906E2A2B435BDE56ACD363EB5C9F36E40701763D`;
  - `libcompiler.dll`: `C80E139C3B3B9F166B09FDC0A6660579D586556258903663B1439FCD9BFE16C6`.
- Kitul `C:\Users\tudor\Desktop\CodeBlocks wx 3.2` și prefixul `C:\Users\tudor\Desktop\CodeBlocks_modern_2026\cb-install` conțin aceleași trei binare, cu hash-uri identice.
- `codeblocks.exe` are subsistem `0x0002 (Windows GUI)`; `cb_console_runner.exe` rămâne `0x0003 (Windows CUI)`.
- Închiderea cu `WM_CLOSE` raportează în acest mediu codul `0xC000041D` (`STATUS_FATAL_USER_CALLBACK_EXCEPTION`); build-ul vechi, testat identic, produce același cod, deci este o problemă pre-existentă de mediu, nu o regresie a optimizărilor.
- Un raport de crash din timpul testelor a fost cauzat de o sondă de test cross-process `LVM_GETITEMTEXT` cu pointer străin, nu de aplicație; sondele ulterioare au folosit doar mesaje marshallate de sistem (`WM_GETTEXT`) sau meniul propriu al logger-ului.

## 14) Pagina Start Here - linkuri utile și etichetă personalizată (29.10.2026)

### 14.1 Linkuri eliminate

- „Visit the Code::Blocks forums" și „Report a bug or request a new feature" au fost eliminate complet din `start_here.html`, împreună cu rândul și href-urile aferente.
- Nu au rămas referințe către forumuri sau bug tracker în pagina Start Here.

### 14.2 Acțiuni noi

- „Open the Code::Blocks folder" - deschide rădăcina kitului în Explorer. Handler: `CB_CMD_OPEN_CB_FOLDER` în `MainFrame::OnStartHereLink` (`src/src/main.cpp`). Calea este derivată din `wxFileName(ConfigManager::ReadAppPath()).GetPath()` (părintele folderului `bin`), apoi lansată cu `wxLaunchDefaultApplication`.
- „Open the modifications file" - deschide `MODIFICATIONS.md` (fallback `CITESTE-Desktop.txt`) direct în editorul intern Code::Blocks: `Manager::Get()->GetEditorManager()->Open(doc)` urmat de `editor->Activate()`, cu fallback `wxLaunchDefaultApplication`. Motiv: asocierea `.md` din sistem (`ASC.Md`) are un hash `UserChoice` invalid și declanșa dialogul „Open With".
- Iconițe SVG noi, în stilul Feather/Lucide existent și cu `currentColor`: `folder.svg` și `document.svg`. Sunt randate prin același mecanism `wxBitmapBundle` → PNG temporar, deci rămân clare la High-DPI și se adaptează temei Light/Dark.

### 14.3 Eticheta de versiune

- Textul vechi cu „Release 25.03 rev 13644 ..." a fost înlocuit cu „Code::Blocks Custom Edition (Dark Optimized) • by tudor62".
- Eticheta este text simplu, colorat cu accentul `#38BDF8`; wrapper-ul `<a href="rev">` a fost eliminat.
- `revInfo` a rămas intact pentru comportamentul de copiere la click pe logo.

### 14.4 Fișiere modificate

- `src/src/resources/start_here/start_here.html` - rândul forumuri/bug report eliminat, cele două acțiuni noi, eticheta personalizată.
- `src/src/resources/start_here/folder.svg` - nou.
- `src/src/resources/start_here/document.svg` - nou.
- `src/src/startherepage.cpp` - eticheta personalizată și substituțiile pentru noile acțiuni.
- `src/src/main.cpp` - handlerele `CB_CMD_OPEN_CB_FOLDER` și `CB_CMD_OPEN_MODIFICATIONS`.

### 14.5 Build, instalare și verificări

- `start_here.zip` reconstruit din `src/src/resources` cu `make start_here.zip`: 595.378 B, SHA-256 `59236C2C17AAF07CC6D78883BB0CCEC289DD06F625F11AD01FAE2751FD374AFA`.
- `codeblocks.exe` reconstruit: 4.194.465 B, SHA-256 `F35F8194144452E0256AB4C05A615D1863C319ECA93254D344E9915AC4FCCF03`, subsistem `0x0002` (Windows GUI). `cb_console_runner.exe` rămâne `0x0003` (Windows CUI).
- Binarele SDK/plugin nu s-au schimbat: `libcodeblocks-0.dll` `77DCAFA6DB408DEAFE887CE2906E2A2B435BDE56ACD363EB5C9F36E40701763D`; `libcompiler.dll` `C80E139C3B3B9F166B09FDC0A6660579D586556258903663B1439FCD9BFE16C6`.
- Executabilul și arhiva au fost copiate în `C:\Users\tudor\Desktop\CodeBlocks wx 3.2` și `C:\Users\tudor\Desktop\CodeBlocks_modern_2026\cb-install`; hash-urile sunt identice în ambele kituri.
- Teste live pe build-ul final:
  - click pe „Open the Code::Blocks folder" → Explorer s-a deschis pe `C:\Users\tudor\Desktop\CodeBlocks wx 3.2`;
  - click pe „Open the modifications file" → titlul ferestrei a devenit `MODIFICATIONS.md - Code::Blocks 25.03`, deci fișierul s-a deschis în editorul intern;
  - eticheta personalizată și cele două acțiuni sunt afișate corect, aliniate, în tema Dark.

### 14.6 Observații

- Asocierea `.md` la nivel de sistem rămâne invalidă (`ASC.Md`); aplicația nu mai depinde de ea, dar alte programe pot afișa în continuare dialogul „Open With".
- `www.png` rămâne în arhiva de resurse, nefolosit.
- Un crash raportat în timpul testelor (17:30) a fost cauzat de sonda cross-process `SB_GETTEXT` a harness-ului de test, nu de aplicație; nu au fost folosite astfel de sonde în verificările finale.
