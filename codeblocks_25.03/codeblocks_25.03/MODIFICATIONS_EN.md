# Changes made

## 1) Toolbar icons - 16x16 and 32x32
I updated the following icons in `src/src/resources/images/` while keeping the **original names and sizes**:

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

Modernized design: clean outline, blue accent for actions, consistent appearance. Script used: `scripts_gen_icons.js`.

## 2) "Start Here" page - `src/src/resources/start_here/`
I updated the following icons while keeping the **original names and sizes**:

- `new.png` (47x42) - modern document with a blue `+` symbol, suggestive for creating a project/file
- `open.png` (47x47) - modern folder with a blue arrow indicating opening, more suggestive and visually consistent

Script used: `scripts_gen_start_here.js`.

## 3) Large buttons in images/ (root)
I updated the following icons in `src/src/resources/images/` while keeping the **original names and sizes**:

- `closebutton.png` (12x12) - modernized, cleaner X symbol
- `compile.png` (22x22) - modern blue gear/wheel, suggestive for compiling
- `compilerun.png` (22x22) - combination of gear + green arrow, suggestive for compiling and running

Script used: `scripts_gen_big_buttons.js`.

## 4) Folder - `src/src/resources/images/folder.png`
- `folder.png` (22x22) - modern flat-style folder with rounded corners, blue gradient and a subtle highlight on the top edge (drawn with anti-aliasing via supersampling)

Script used: `scripts_gen_folder.js`.

## 5) Stop - `src/src/resources/images/stop.png`
- `stop.png` (22x22) - redesign: rounded square filled red (#EF4444 → #B91C1C) with a centered white X — the modern and clear "stop" metaphor

Script used: `scripts_gen_stop.js`.

## 6) Light/Dark application theme (Full Dark Theme)
- Option in `Settings -> Environment -> View`, "Application theme" section (`wxChoice name="chTheme"`).
- The value is saved in the config under the `app` namespace, key `/environment/theme` (`light` / `dark`), default `light`.
- Exact palette used by the Dark theme:
  - menu bar, title bar, inactive tabs: `#0B0E12`;
  - side panels and Scintilla editor: `#13181D`;
  - "Logs & others" panel: `#0F1318`;
  - selection/active element: `#1E293B` / `#1F2D3D`;
  - main text/code: `#E2E8F0` / `#D4D4D4`;
  - Start Here links: `#38BDF8`.
- On Windows, native dark mode is enabled through dynamically loaded `uxtheme.dll`/`dwmapi.dll` (compatible with wxWidgets 3.2):
  - `SetPreferredAppMode(2)`, `AllowDarkModeForWindow()`, `SetWindowTheme()` and `DwmSetWindowAttribute(DWMWA_USE_IMMERSIVE_DARK_MODE)`;
  - on Windows 11 build >= 26200, `DarkMode_DarkTheme` is used; on older versions `DarkMode_CFD` for edit/combo and `DarkMode_Explorer` for the rest;
  - native scrollbars, lists, trees, comboboxes, spins and dialogs receive the dark variant.
- The menu bar is drawn by the application through the UAH messages also used by wxWidgets 3.3 (`WM_MENUBAR_DRAWMENU` 0x91, `WM_MENUBAR_DRAWMENUITEM` 0x92, `WM_UAHMEASUREMENUITEM` 0x94): background `#0B0E12`, text `#E2E8F0`, active element `#1E293B`. Popup menus remain natively dark-drawn by Windows.
- The toolbar has a `#0B0E12` background; the `wxBitmapBundle`/SVG icons remain visible.
- Tabs use a custom art provider (`NbStyleDark`, derived from `wxAuiGenericTabArt`) to avoid any white native fallback: active tab `#13181D`, inactive tabs `#0B0E12`, text `#94A3B8`, close button `#94A3B8`.
- AUI panes receive dark background/sash/border/caption in `MainFrame::ApplyTheme()`; when switching back to Light, configurable colors are re-read from the config, and the others are restored from `wxAuiDefaultDockArt`.
- Native controls that do not have a complete dark theme are partially drawn by the application:
  - `SysHeader32` via `NM_CUSTOMDRAW` (background `#0B0E12`, text `#E2E8F0`, border `#1E293B`, filler after the last column);
  - `wxStaticLine` via `WM_NCPAINT` (the 2 px line is entirely non-client);
  - `wxSlider` via `NM_CUSTOMDRAW` for the channel (`#1E293B`), keeping the native blue thumb;
  - disabled `Edit`/`ComboBox` fields via `WM_CTLCOLORSTATIC`/`WM_CTLCOLOREDIT`/`WM_CTLCOLORLISTBOX`.
- `cbMessageBox()` uses `wxGenericMessageDialog` when the Dark theme is active, then applies the theme and shows the dialog; Information/Question messages no longer appear white. In Light, the native dialog remains.
- When switching Dark -> Light, `cbApplyAppThemeToWindow()` explicitly restores the system colors for tree/list (`TVM_SETBKCOLOR`, `TVM_SETTEXTCOLOR`, `LVM_SETBKCOLOR`, `LVM_SETTEXTCOLOR`, `LVM_SETTEXTBKCOLOR`) and for panels, so that Management/tree and the other lists become white with dark text again.
- The theme change is applied immediately for widgets; the native menu, title bar and some common controls fully stabilize after a restart, which is why the prompt "Code::Blocks needs to be restarted for the changes to take effect." is shown.
- The Scintilla editor uses the dark theme: cyan keywords `#38BDF8`, light blue types, pink-magenta numbers/constants `#F472B6`, blue-violet control keywords, subtle green comments `#4ADE80`; margins and background are `#13181D`.
- Logs have a `#0F1318` background and bright text; the build log displays GCC output legibly.
- The "Start here" page uses a dark background, `#D4D4D4` text and `#38BDF8` links.

Modified files:
- `src/src/resources/env_settings.xrc` - `wxChoice` `chTheme` (Light/Dark)
- `src/src/environmentsettingsdlg.cpp` - reads/writes `/environment/theme`
- `src/include/globals.h`, `src/sdk/globals.cpp` - `cbTheme`, `cbAppTheme`, `cbGetAppTheme()`, `cbIsAppThemeDark()`, `cbApplyNativeTheme()`, `cbApplyNativeAppTheme()`, `cbApplyAppThemeToWindow()`, `cbApplyThemeToAuiNotebooks()`, the Windows subclasses for header/static line/slider/native controls, the UAH menu bar and the dark `cbMessageBox()`
- `src/src/main.h`, `src/src/main.cpp` - `MainFrame::ApplyTheme()`, `DoUpdateLayoutColours()`, `DoUpdateEditorStyle()`, the UAH menu messages, the complete color restoration when switching to Light, the restart prompt
- `src/src/app.cpp` - event filtering for new windows/dialogs and native theme application at startup
- `src/src/notebookstyles.cpp`, `src/src/notebookstyles.h` - `NbStyleDark` for tabs
- `src/sdk/editorcolourset.cpp` - Scintilla dark palette and margin colors
- `src/sdk/loggers.cpp` - log backgrounds and colors
- `src/src/startherepage.cpp` - "Start here" page colors, including the `#38BDF8` links

## 7) General observations
- All icon changes were made through programmatic drawing (no brute-force scaling), keeping the exact sizes and original file names.
- The theme changes affect source files (`.cpp`, `.h`, `.xrc`); the other changes were icons only.
- There are multiple icon sets by size (`16x16` to `64x64`, `80x80`) for HiDPI/scaling support - these are rendered separately for legibility at each size, not simple scaled copies.

## 8) Port to wxWidgets 3.2.x: modern APIs, High-DPI and native dark mode

### 8.1 Icons via `wxBitmapBundle`, SVG and High-DPI support

- `src/sdk/globals.cpp`
  - `cbLoadBitmapBundle()`: the `wxFileSystem*` parameter is no longer ignored; `bitmapType` and the filesystem are correctly passed to `cbLoadBitmap()`. All `16x16 ... 64x64` variants are tried, and if the image has no size directories, a bundle with a single bitmap is returned (instead of an invalid bundle).
  - `cbLoadBitmapBundleFromSVG()`: uses `wxBitmapBundle::FromSVG()` and provides vector scaling at any DPI factor. It is kept as the single loading point for SVG.
- `src/include/xtra_res.h` and `src/sdk/xtra_res.cpp`
  - Added `wxToolBarAddOnXmlHandler::GetCenteredBitmapBundle()`, which builds a `wxBitmapBundle` for XRC toolbar resources:
    - prefers the SVG in the `svg/` directory when present;
    - otherwise loads all PNG variants `16x16 ... 64x64`;
    - for images without size directories, creates a bundle from a single bitmap;
    - for `stock_id`, uses `wxArtProvider::GetBitmapBundle()`, so art provider icons are scalable too.
  - `wxToolBarAddOnXmlHandler::DoCreateResource()` calls the modern overload `wxToolBar::AddTool(..., const wxBitmapBundle&, ...)`.
  - `GetCenteredBitmap()` is kept for compatibility with wxWidgets < 3.1.6 and as a fallback.
- `src/src/main.cpp`
  - `MainFrame::AddToolbarItem()`: no longer calls `cbLoadBitmap()` + `AddTool(wxBitmap)` directly. It builds an SVG bundle (preferred) or a multi-size PNG bundle and passes it to `wxToolBar::AddTool()`. This way the main toolbar automatically follows the DPI scale.
- `wxImageList` does not accept `wxBitmapBundle` in wxWidgets 3.2; for image lists, `wxBitmap` is kept together with `cbMakeScaledImageList()` and `cbAddBitmapToImageList()`, which take the scaling factor into account. Bundles are used where the API supports them natively (toolbar, menus, buttons, art provider).
- Call sites already modernized in the existing code (verified at this stage): `src/plugins/compilergcc/compilergcc.cpp`, `src/plugins/codecompletion/codecompletion.cpp`, `src/plugins/abbreviations/abbreviations.cpp`, `src/src/breakpointsdlg.cpp` etc. They already use `cbLoadBitmapBundleFromSVG()`/`wxBitmapBundle` for log, list and notebook icons.

### 8.2 Native dark mode on Windows (UCRT64/MinGW)

- `src/include/globals.h`
  - Added `cbApplyNativeTheme(wxWindow* window, bool dark)`.
- `src/sdk/globals.cpp`
  - Windows implementation that dynamically loads `dwmapi.dll` and `uxtheme.dll` (no additional link dependency):
    - `DwmSetWindowAttribute()` with the `DWMWA_USE_IMMERSIVE_DARK_MODE` attribute (20 on Windows 10 2004+, fallback 19) for a dark title bar on top-level windows;
    - `SetWindowTheme(hwnd, L"DarkMode_Explorer", nullptr)` for the native scrollbars of the controls that use them (`TextCtrl`, `ListCtrl`, `TreeCtrl`, `Scintilla`, `RichEdit`, `ListBox`, `ComboBox`, `PropertyGrid`, `DataView`, `Grid`, `ScrollBar`);
    - when switching back to the light theme, `L"Explorer"` is applied, returning to the normal appearance.
  - On non-Windows platforms the function is a no-op, so the code remains portable.
- `src/src/main.cpp`
  - `ApplyThemeToWindowTree()` calls `cbApplyNativeTheme()` for every window in the tree.
  - `MainFrame::ApplyTheme()` walks `wxTopLevelWindows` and applies native decoration to dialogs and floating AUI panes as well (they are not children of the main window).
  - `MainFrame` re-applies native decoration on `wxEVT_SHOW`: on Windows, `DWMWA_USE_IMMERSIVE_DARK_MODE` can be ignored while the window is still hidden, so the title bar could remain light on first display.
  - `src/sdk/cbeditor.cpp`: `cbEditor::CreateEditor()` applies the native theme to every newly created `cbStyledTextCtrl`, so the scrollbars of editors opened after startup are dark as well.
  - Menus and the toolbar are fully themed in this implementation: the menu through the UAH messages, and the toolbar through the `#0B0E12` background. On wx 3.3+ `wxApp::SetAppearance()` can additionally cover native elements, but the implementation here works on wx 3.2.
- Editor rendering: Direct2D integration is preserved. `src/sdk/cbeditor.cpp` reads `/technology` (default `1`) and calls `SetTechnology(wxSCI_TECHNOLOGY_DIRECTWRITE)`; `src/sdk/wxscintilla/src/ScintillaWX.cpp` handles `SCI_SETTECHNOLOGY` unchanged. The modernization does not disable or bypass `wxGraphicsContext`/Direct2D.

### 8.3 Migration from `Connect()`/macros to `.Bind()`

- `src/src/main.cpp`: the four connections for `wxEVT_COMMAND_TOOL_RCLICKED` were converted to:
  - `m_pToolbar->Bind(wxEVT_TOOL_RCLICKED, &MainFrame::OnToolBarRightClick, this);`
  - `m_debuggerToolbarHandler->GetToolbar()->Bind(...)`
  - the two calls in `CreateToolbars()` and `DoAddPlugin()` for the plugin toolbars.
- The new event name `wxEVT_TOOL_RCLICKED` was used instead of the old alias.
- The remaining `wxBEGIN_EVENT_TABLE`/`EVT_*` event tables are **not deprecated in wxWidgets 3.2**, and converting them entirely would introduce high risk in hundreds of files without functional benefit. `.Bind()` was used in the code touched by this modernization; the other tables were intentionally left as they were, according to the principle "where necessary for clarity and safety".

### 8.4 Modern C++ includes and types

- `src/include/prep.h`: `<stdint.h>` → `<cstdint>`.
- `src/tools/cbp2make/lib/stringhash.h`: `<stdint.h>` → `<cstdint>`.
- `src/plugins/contrib/ThreadSearch/ThreadSearchViewManagerBase.h`: `<stdint.h>` → `<cstdint>`.
- `src/include/globals.h` already uses `<cstdint>` for `int64_t`/`uint64_t` (the `GlobId` typedef and the debugger interfaces).
- No proprietary types or redundant typedefs were introduced; the code still uses `int64_t`/`uint64_t` from `<cstdint>`.

### 8.5 APIs deprecated in wxWidgets 3.2

- `src/sdk/scripting/bindings/sc_consts.cpp`: `wxPATH_NORM_ALL` is marked deprecated in wx 3.2. It was replaced with the explicit combination `wxPATH_NORM_ENV_VARS | wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT`, keeping the same values and the same constant name exposed to Squirrel scripts (`wxPATH_NORM_ALL`) through `BIND_INT_CONSTANT_NAMED`.
- Toolbar icon loading no longer uses `wxBitmap` where a `wxBitmapBundle` overload exists.
- Following the build with wxWidgets 3.2.11, no deprecated API warnings appeared in the recompiled modules.

### 8.6 Build and installation verification

- Complete build, from scratch, with MSYS2 UCRT64, g++ 16.1.0 and wxWidgets 3.2.11 (`wx-config --version` = 3.2.11):
  - `make clean` at tree level → `CLEAN_EXIT=0`
  - `make -j12` for the entire tree (SDK, application, plugins, tools) → `BUILD_EXIT=0`
  - final relink of the application with `-Wl,--no-undefined -mwindows` → `RELINK_EXIT=0`; `objdump -p` reports `Subsystem 00000002 (Windows GUI)`
  - `make install` → `INSTALL_EXIT=0`
- The functional kit is installed in `C:\Users\tudor\Desktop\CodeBlocks wx 3.2` (executable: `bin\codeblocks.exe`, wxWidgets/MinGW runtime DLLs in `bin`, plugins in `lib\codeblocks\plugins`, resources in `share\codeblocks`).
- Verification on Windows 11 build 26200: the **Dark** theme applies dark panels, dark menu/toolbar, dark tabs, native dark title bar and dark dialogs; the **Light** theme fully returns to the normal appearance (light title bar, white panels, white tree/list, default AUI captions). Live switching Dark -> Light -> Dark was verified, and Information/Question messages are dark in the Dark theme.

## Files created for generation
- `scripts_gen_icons.js` - generator for the toolbar icons (16x16, 32x32)
- `scripts_gen_start_here.js` - generator for the start_here icons (47x42, 47x47)
- `scripts_gen_big_buttons.js` - generator for closebutton, compile, compilerun
- `scripts_gen_folder.js` - generator for `folder.png`
- `scripts_gen_stop.js` - generator for `stop.png`

## 9) Modern splash screen and Start Here SVG icons

- `src/src/resources/images/splash_2603.png` - the new splash screen (1671x941). The startup window loads it and scales it proportionally to a maximum of 800x450, then displays it centered on the screen, with the same options as before (`wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT`). The old `splash_1312.png` image was kept.
- `src/src/resources/start_here/title_2603.png` - the modern banner for the Start Here page, resized to 440x248 for display.
- `src/src/resources/start_here/new.svg`, `open.svg`, `reopen.svg`, `tip.svg`, `trash.svg` - new vector icons, with the same names as the old PNGs; the old PNG files were kept as a fallback.
- `src/src/resources/start_here/start_here.html` - updated references to `title_2603.png` and the SVG icons (40x40 for actions, 16x16 for the trash bin).
- `src/src/app.cpp` - uses `splash_2603.png`; no longer draws the old release text over the new image.
- `src/src/dlgabout.cpp` - the About dialog uses the same image, scaled to a maximum of 480x270.
- `src/include/globals.h`, `src/sdk/globals.cpp` - added the `cbScaleBitmapToFit()` function for quality proportional scaling.
- `src/src/startherepage.cpp` - `wxHtmlWindow` does not display SVG natively in wxWidgets 3.2. `MyHtmlWin::OnOpeningURL()` intercepts `.svg` requests, renders the icon through `wxBitmapBundle::FromSVG()` and redirects to a temporary PNG. `currentColor` is replaced with `#E6E6E6` in the Dark theme and `#333333` in Light; rendering is done at 4x the display size for High-DPI clarity, and the temporary files are deleted when the window is destroyed.
- `src/src/resources/Makefile.am`, `Makefile.in`, `Makefile` - `start_here.zip` now also includes the `*.svg` files.
- `src/src/Makefile.am`, `Makefile.in`, `Makefile` - added the paths to `src/exchndl/win64/include` and `src/exchndl/win64/lib`, so that `exchndl.h` and `libexchndl.a` are found at compile and link time without additional environment variables.

Result: the new splash is centered and scaled correctly, the Start Here banner uses the modern image, and the Start Here icons are vector-based and adapt to the Light/Dark theme.

## 10) Final Full Dark Theme verification (27.10.2026)

- Final build with `make -C src/sdk -j8` and `make -C src/src -j8` (MSYS2 UCRT64, wxWidgets 3.2.11, GCC 16.1.0), without errors.
- `codeblocks.exe` and `libcodeblocks-0.dll` updated in the `C:\Users\tudor\Desktop\CodeBlocks wx 3.2\bin` kit and in the `C:\Users\tudor\Desktop\CodeBlocks_modern_2026\cb-install\bin` prefix.
- Live checks on Windows 11 build 26200:
  - dark menu: bar `#0B0E12`, text `#E2E8F0`, active element `#1E293B`; native dark popups;
  - toolbar `#0B0E12`, legible icons;
  - tabs: active `#13181D`, inactive `#0B0E12`, text `#94A3B8`; no white borders;
  - editor `#13181D`; Scintilla syntax with `#38BDF8`, `#A78BFA`, `#4ADE80`, `#FBBF24`, `#C084FC`, `#F472B6`;
  - Management/tree `#13181D`, Start Here links `#38BDF8`;
  - "Logs & others" `#0F1318`; Build messages header `#0B0E12`, text `#E2E8F0`, filler `#0B0E12`;
  - static lines `#1E293B`; slider channel `#1E293B`, blue thumb `#0078D7`;
  - Environment settings / Editor configuration dialogs dark, including enabled/disabled fields and scrollbars `#171717`;
  - Information/Question messages dark in the Dark theme.
- Live switching Dark -> Light -> Dark: Light fully restores panels, tree/list, tabs, editor, logs and AUI captions to the native appearance; Dark brings back the dark UI. The restart prompt appears in both directions and is dark when the active theme is Dark.
- Clean startup and shutdown, with no Code::Blocks processes left behind; no white surfaces in the Dark theme.

## 11) GUI executable without a console (27.10.2026)

- `src/src/Makefile.am`, `src/src/Makefile.in`, `src/src/Makefile`:
  - in the `CODEBLOCKS_NT` conditional block, `AM_LDFLAGS += -mwindows` was added;
  - `-mwindows` applies only to the main `codeblocks.exe` executable, not to the entire tree;
  - `src/tools/ConsoleRunner` remains unchanged and is still built as a console application.
- `src/CodeBlocks_wx31.cbp`, `src/CodeBlocks_wx31_64.cbp`, `src/CodeBlocks_wx32.cbp`, `src/CodeBlocks_wx32_64.cbp`:
  - the `-mwindows` option was added to the Linker for the `src` target, so that rebuilds from the Code::Blocks workspace also produce a GUI executable.
- `src/src/app.cpp`:
  - `InitDebugConsole()` no longer suggests compiling as a console application; `AllocConsole()` remains available exclusively in debug builds (`__CBDEBUG__`);
  - in the release build, no console is allocated or attached at startup.
- Startup and build logs are displayed in the internal `Logs & others` panel (`Code::Blocks`, `Build log`, `Build messages`, `Debugger` tabs); they are no longer redirected to an external console.
- Running a console project still uses `cb_console_runner.exe`, which remains a console application; the user program's window opens separately only during execution.
- Batch builds launched from the terminal (`codeblocks.exe --build`) use the internal batch build window; the output is no longer written to the terminal console.
- Checks:
  - `objdump -p src/src/.libs/codeblocks.exe` → `Subsystem 00000002 (Windows GUI)`;
  - `objdump -p src/tools/ConsoleRunner/cb_console_runner.exe` → `Subsystem 00000003 (Windows CUI)`;
  - launch from Explorer: the main window starts without any console window; the `Logs & others` panel is present and functional;
  - `codeblocks.exe` updated in the Desktop kit and in `cb-install`, with identical SHA-256 hash.

## 12) New icons and logo (28.10.2026)

- New icon set taken from `C:\Users\tudor\Desktop\Imagini`, keeping the destination file names 1:1:
  - compiler icons in `src/plugins/compilergcc/resources/images/svg/`:
    - `compile.svg` (288,062 B);
    - `compilerun.svg` (219,535 B, renamed from `compilerrun.svg`);
    - `rebuild.svg` (64,931 B);
    - `run.svg` (89,401 B);
    - `stop.svg` (109,935 B).
  - general icons in `src/src/resources/images/svg/`:
    - `fileprint.svg` (1,193 B), `filesave.svg` (1,079 B), `filesaveall.svg` (2,696 B), `filesaveas.svg` (2,086 B), `idea.svg` (1,381 B), `info.svg` (537 B), `redo.svg` (3,402 B), `undo.svg` (3,229 B).
- The opaque background / checkerboard pattern included in `compile.svg`, `run.svg` and `stop.svg` was removed by re-vectorizing with a transparent background; `compilerun.svg` and `rebuild.svg` were cleaned from the source. The icons display correctly on the Dark theme, without gray squares.
- `resources.zip` and `compiler.zip` were regenerated with the new SVG files; the old PNGs remained in the archives as a fallback for wxWidgets < 3.1.6.
- Logo:
  - `new_logo.png` was cropped, its dark dotted background was removed and it was converted to RGBA with transparency;
  - `app.ico` was generated with 16/24/32/48 32bpp DIB entries + 64/128/256 PNG (110,252 B), keeping the original ICO style;
  - `src/src/resources/icons/app.ico` was replaced; `resources.o` was deleted so that `windres` reloads the icon (`app.ico` is not a declared make dependency);
  - the icon is visible in the title bar, in Explorer and in the taskbar.
- Final build:
  - `make -j$(nproc)` in `src/src` → `MAKE_EXIT=0`;
  - `src/src/.libs/codeblocks.exe`: 4,189,522 B, SHA-256 `54A5BD96A03F1A237C869777CF39DD855D00BB45B7BA1634777A05DAF72D1660`;
  - `objdump -p` → `Subsystem 00000002 (Windows GUI)`; no console at startup;
  - `cb_console_runner.exe` remains `Subsystem 00000003 (Windows CUI)`.
- Desktop kit updated:
  - `bin/codeblocks.exe` (hash identical to the build);
  - `share/codeblocks/resources.zip` (2,682,219 B, SHA-256 `6F90151288F46186DBE762ED960EFA2597ECB7193C9DCD874D45D2B5B1257C99`);
  - `share/codeblocks/compiler.zip` (374,984 B, SHA-256 `924B82158DBAE704CB21D9DAF788936B122C4781F08D650B396CE642A86DF9D5`).
- `cb-install` was synchronized with the same files (executable + the two archives), so that the installation prefix used for linking remains identical to the kit.
- Live verification on Windows 11 build 26200, Dark theme: the compiler toolbar displays the new icons (compile, run, build and run, rebuild, stop), the main toolbar uses the new SVG icons, the title bar displays the new logo, and the application starts without a console window. The icons are transparent on the dark background.
- Backup for the original files: `C:\Users\tudor\AppData\Local\Temp\opencode\backup_resources_20261028_171106` (SVGs + `app.ico` + the old archives).

## 13) Stage 1 optimizations - startup, theme, logs and compiler cache (28.10.2026)

### 13.1 P0.1 - load the workspace after the window is shown

- `src/src/app.cpp`:
  - when there is a workspace to load, `LoadWorkspace()` is deferred with `CallAfter()` after `frame->Show()`, `StartupDone()` and `frame->Update()`;
  - startup completion (`LoadDelayedFiles()`, `AttachDebugger()`, `WorkspaceChanged()`, `SetAppStartedUp(true)`, the `cbEVT_APP_STARTUP_DONE` event and the crash report message) was moved into the `finishStartup` lambda, called immediately after loading the workspace;
  - `s_Loading` is cleared in `finishStartup`, not before loading the workspace.
- Effect: the main window becomes visible in ~23-25 s in an environment with slow startup, and a stress workspace with 150 projects still loads after it is shown; the project tree and title are filled in later. No interface freeze while parsing the workspace.

### 13.2 P0.4 - a single theme pass at startup

- `src/src/app.cpp`: the `cbAppThemeFilter` event filter no longer applies the theme for `MainFrame`; the main window is themed only once, in its constructor.
- `src/src/main.cpp`:
  - `MainFrame::ApplyTheme()` runs once at the end of the `MainFrame` constructor and uses `Freeze()`/`Thaw()` on the window and on the AUI panes;
  - there is no longer a `Refresh()` per control, per menu or per status bar; at the end there is a single global refresh;
  - floating AUI panes are explicitly themed;
  - `DoUpdateLayoutColours()` calls `ApplyTheme()` only after `m_StartupDone`, to avoid a second pass during startup;
  - `StartupDone()` only sets the flag, calls `DoUpdateLayout()` and `m_LayoutManager.Update()`.
- `src/sdk/globals.cpp`:
  - `cbApplyAppThemeToWindow()` freezes the top-level window during the recursive walk and no longer refreshes each control;
  - `cbApplyThemeToAuiNotebooks()` no longer calls `Refresh()` on notebooks;
  - `ApplyNativeThemeToTopLevelWindows()` skips the main window.
- `src/src/startherepage.cpp`: in the Dark theme, the Start Here page and `wxHtmlWindow` receive the `cbTheme::Panel()` background from the constructor, before the first paint.
- Deliberate deviation from the requirement: the single theme pass runs at the end of the `MainFrame` constructor, before `frame->Show()`, not after `Show()`. The variant with the theme after `Show()` produced a visible white flash; the variant with an outer `Freeze()` around `Show()` produced a white/empty client area. `frame->Update()` is called in `app.cpp` immediately after `StartupDone()` to force the first complete paint. The Start Here page can still appear white for under 200 ms on the first paint (measured); the title bar, menus, toolbar and panels are already dark. `StartupDone()` before `Show()` and outer freezing were tried and reverted because they made the first frame worse.

### 13.3 P1.1 - batched log writes

- `src/sdk/loggers.cpp`, `src/include/loggers.h`:
  - each logger keeps a queue of pending messages and a timestamp;
  - a shared 50 ms `wxTimer` flushes the queues;
  - a single `Freeze()`/`Thaw()` and a single auto-scroll at the end of each batch, instead of one per message;
  - `Clear()` discards pending messages;
  - copy and count operations force the queue to flush, so no messages are missing;
  - `UpdateSettings()` flushes the queue and applies the new styles;
  - `CompilerMessages::FocusError()` flushes the queue before selecting the error row.
- The logger class layout was kept for ABI compatibility with the plugins.
- Functional verification with a project that generates 300 warnings:
  - Build log: 916 lines / 51,602 characters, all 600 "batch line" occurrences (300 warnings × 2) in order, without losses or reordering;
  - Build messages: 302 rows (300 warnings + summary);
  - "Copy contents to clipboard": 300 warnings in the clipboard;
  - "Clear contents": the list reaches 0 rows;
  - auto-scrolled Build log at the end: `firstVisible=904` out of 910 lines, with the "Process terminated" and "0 error(s), 300 warning(s)" lines visible.

### 13.4 P1.3 - cache for `/ignore_output` and `/max_reported_errors`

- `src/plugins/compilergcc/compilergcc.h`, `compilergcc.cpp`:
  - cache `m_CachedIgnoreOutput` and `m_CachedMaxReportedErrors`, protected by `wxCriticalSection`;
  - `EnsureCompilerConfigCache()` loads the settings once, on the first compiler output;
  - `RefreshCompilerConfigCache()` is called at the beginning of the build (`DoPrepareQueue()`) and in `CompileFileDefault()`;
  - `InvalidateCompilerConfigCache()` is called from `CompilerOptionsDlg::OnApply()`.
- Live verification in the isolated `cachetest` personality:
  - settings: max=5, ignore=`IGNORE_THIS_LINE` → Build messages shows exactly 5 errors, the ignored line is missing;
  - settings changed without restart: max=4, ignore=`undeclared_2` → Build messages shows 4 errors, `IGNORE_THIS_LINE` reappears, `undeclared_2` is filtered;
  - therefore invalidation on save and reload at the beginning of the build work within the same process.

### 13.5 Modified files

- `src/src/app.cpp` - deferred workspace loading, `frame->Update()` after `StartupDone()`, excluding `MainFrame` from the theme filter.
- `src/src/main.cpp` - single theme pass in the constructor, `StartupDone()`, `ApplyTheme()`, `DoUpdateLayoutColours()`.
- `src/src/startherepage.cpp` - dark background in the constructor for the Start Here page.
- `src/sdk/globals.cpp` - recursive theme application with `Freeze()`/`Thaw()` and a single refresh.
- `src/sdk/loggers.cpp`, `src/include/loggers.h` - batched logs.
- `src/plugins/compilergcc/compilergcc.cpp`, `compilergcc.h` - cache for the compiler settings.
- `src/plugins/compilergcc/compileroptionsdlg.cpp` - cache invalidation on save.
- `src/plugins/compilergcc/compilermessages.cpp` - flush the queue before `FocusError()`.

### 13.6 Build, installation and final checks

- Clean build with MSYS2 UCRT64, wxWidgets 3.2.11, GCC 16.1.0: SDK, `codeblocks.exe` and the compiler plugin, without errors.
- Final binaries (SHA-256):
  - `codeblocks.exe`: `C3C6D2325FF20162ED2E98257ECB087DCA4E1A5BB71C4EE7BB0E9A569C56B8FC` (4,192,893 B);
  - `libcodeblocks-0.dll`: `77DCAFA6DB408DEAFE887CE2906E2A2B435BDE56ACD363EB5C9F36E40701763D`;
  - `libcompiler.dll`: `C80E139C3B3B9F166B09FDC0A6660579D586556258903663B1439FCD9BFE16C6`.
- The `C:\Users\tudor\Desktop\CodeBlocks wx 3.2` kit and the `C:\Users\tudor\Desktop\CodeBlocks_modern_2026\cb-install` prefix contain the same three binaries, with identical hashes.
- `codeblocks.exe` has subsystem `0x0002 (Windows GUI)`; `cb_console_runner.exe` remains `0x0003 (Windows CUI)`.
- Closing with `WM_CLOSE` reports the code `0xC000041D` (`STATUS_FATAL_USER_CALLBACK_EXCEPTION`) in this environment; the old build, tested identically, produces the same code, so it is a pre-existing environment issue, not a regression of the optimizations.
- A crash report during the tests was caused by a cross-process test probe `LVM_GETITEMTEXT` with a foreign pointer, not by the application; subsequent probes used only system-marshalled messages (`WM_GETTEXT`) or the logger's own menu.

## 14) Start Here page - useful links and custom label (29.10.2026)

### 14.1 Removed links

- "Visit the Code::Blocks forums" and "Report a bug or request a new feature" were completely removed from `start_here.html`, together with their row and hrefs.
- No references to the forums or the bug tracker remained on the Start Here page.

### 14.2 New actions

- "Open the Code::Blocks folder" - opens the kit root in Explorer. Handler: `CB_CMD_OPEN_CB_FOLDER` in `MainFrame::OnStartHereLink` (`src/src/main.cpp`). The path is derived from `wxFileName(ConfigManager::ReadAppPath()).GetPath()` (the parent of the `bin` folder), then launched with `wxLaunchDefaultApplication`.
- "Open the modifications file" - opens `MODIFICATIONS.md` (fallback `CITESTE-Desktop.txt`) directly in the internal Code::Blocks editor: `Manager::Get()->GetEditorManager()->Open(doc)` followed by `editor->Activate()`, with `wxLaunchDefaultApplication` as a fallback. Reason: the system `.md` association (`ASC.Md`) has an invalid `UserChoice` hash and triggered the "Open With" dialog.
- New SVG icons, in the existing Feather/Lucide style and with `currentColor`: `folder.svg` and `document.svg`. They are rendered through the same `wxBitmapBundle` → temporary PNG mechanism, so they remain sharp at High-DPI and adapt to the Light/Dark theme.

### 14.3 Version label

- The old text with "Release 25.03 rev 13644 ..." was replaced with "Code::Blocks Custom Edition (Dark Optimized) • by tudor62".
- The label is plain text, colored with the `#38BDF8` accent; the `<a href="rev">` wrapper was removed.
- `revInfo` remained intact for the copy-on-logo-click behavior.

### 14.4 Modified files

- `src/src/resources/start_here/start_here.html` - removed the forums/bug report row, the two new actions, the custom label.
- `src/src/resources/start_here/folder.svg` - new.
- `src/src/resources/start_here/document.svg` - new.
- `src/src/startherepage.cpp` - the custom label and the substitutions for the new actions.
- `src/src/main.cpp` - the `CB_CMD_OPEN_CB_FOLDER` and `CB_CMD_OPEN_MODIFICATIONS` handlers.

### 14.5 Build, installation and checks

- `start_here.zip` rebuilt from `src/src/resources` with `make start_here.zip`: 595,378 B, SHA-256 `59236C2C17AAF07CC6D78883BB0CCEC289DD06F625F11AD01FAE2751FD374AFA`.
- `codeblocks.exe` rebuilt: 4,194,465 B, SHA-256 `F35F8194144452E0256AB4C05A615D1863C319ECA93254D344E9915AC4FCCF03`, subsystem `0x0002` (Windows GUI). `cb_console_runner.exe` remains `0x0003` (Windows CUI).
- The SDK/plugin binaries did not change: `libcodeblocks-0.dll` `77DCAFA6DB408DEAFE887CE2906E2A2B435BDE56ACD363EB5C9F36E40701763D`; `libcompiler.dll` `C80E139C3B3B9F166B09FDC0A6660579D586556258903663B1439FCD9BFE16C6`.
- The executable and the archive were copied to `C:\Users\tudor\Desktop\CodeBlocks wx 3.2` and `C:\Users\tudor\Desktop\CodeBlocks_modern_2026\cb-install`; the hashes are identical in both kits.
- Live tests on the final build:
  - clicking "Open the Code::Blocks folder" → Explorer opened at `C:\Users\tudor\Desktop\CodeBlocks wx 3.2`;
  - clicking "Open the modifications file" → the window title became `MODIFICATIONS.md - Code::Blocks 25.03`, so the file opened in the internal editor;
  - the custom label and the two actions are displayed correctly, aligned, in the Dark theme.

### 14.6 Observations

- The system-level `.md` association remains invalid (`ASC.Md`); the application no longer depends on it, but other programs may still show the "Open With" dialog.
- `www.png` remains in the resource archive, unused.
- A crash reported during the tests (17:30) was caused by the test harness's cross-process `SB_GETTEXT` probe, not by the application; no such probes were used in the final checks.
