; Compile script using Nullsoft Scriptable Install System (NSIS) on windows

;--------------------------------
!include x64.nsh
!include "MUI2.nsh"
!include "installer_common.nsh"

; load the version from file
!define /file VERSION "../VERSION"

; ORDER must be passed on the command line: /DORDER=x
!ifndef ORDER
    !error "ORDER must be defined on the command line (e.g. /DORDER=5)"
!endif

; Uninstaller identity (per-order, so multiple orders can coexist in Programs and Features)
!define UNINST_KEY  "Software\Microsoft\Windows\CurrentVersion\Uninstall\ambix_VST2_o${ORDER}"
!define UNINST_NAME "ambix VST2 plugin suite (order ${ORDER})"
!define UNINST_EXE  "uninstall_o${ORDER}.exe"

; Sign the embedded uninstaller at compile time (enabled by passing /DSIGN_UNINST=1 to makensis)
!ifdef SIGN_UNINST
    !uninstfinalize 'cmd /c sign_file.bat "%1"'
!endif

; The name of the installer
!define NAME "ambix_v${VERSION}_VST2_win64_o${ORDER}"
Name ${NAME}

; The file to write
!system 'mkdir "../_WIN_RELEASE" 2> NUL'
OutFile "../_WIN_RELEASE/${NAME}.exe"

; Build Unicode installer
Unicode True

; The default installation directory
InstallDir "$PROGRAMFILES64\Steinberg\VSTPlugins\ambix"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
; Pages
!insertmacro MUI_PAGE_WELCOME

!define MUI_TEXT_WELCOME_INFO_TITLE "ambix v${VERSION} order ${ORDER}"

!insertmacro MUI_PAGE_LICENSE "../README.md"
!insertmacro MUI_PAGE_COMPONENTS
Page directory
Page instfiles

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install
Section "ambix order ${ORDER} VST2 plugins"
    SetOutPath "$INSTDIR"
    File /r "..\build\vst_o${ORDER}\*.dll"
    ${DisableX64FSRedirection}
SectionEnd

; ambix Visualizer standalone companion app
Section "ambix Visualizer" SecVisualizer
    SetOutPath "$PROGRAMFILES64\ambix"
    File "..\build\standalone\ambix Visualizer.exe"
SectionEnd

Section "Start Menu shortcut (ambix Visualizer)" SecStartMenu
    SectionIn 1
    CreateDirectory "$SMPROGRAMS\ambix"
    CreateShortCut "$SMPROGRAMS\ambix\ambix Visualizer.lnk" "$PROGRAMFILES64\ambix\ambix Visualizer.exe"
SectionEnd

Section /o "Desktop shortcut (ambix Visualizer)" SecDesktop
    CreateShortCut "$DESKTOP\ambix Visualizer.lnk" "$PROGRAMFILES64\ambix\ambix Visualizer.exe"
SectionEnd

; libfftw3f-3.dll
Section "FFTW3f library"
    SetOutPath "$SYSDIR"
    File "..\win-libs\x64\libfftw3f-3.dll"
SectionEnd

; Write uninstaller + Programs-and-Features registration (hidden, always runs)
Section "-Uninstaller"
    SetOutPath "$INSTDIR"
    WriteUninstaller "$INSTDIR\${UNINST_EXE}"
    WriteRegStr   HKLM "${UNINST_KEY}" "DisplayName"     "${UNINST_NAME}"
    WriteRegStr   HKLM "${UNINST_KEY}" "DisplayVersion"  "${VERSION}"
    WriteRegStr   HKLM "${UNINST_KEY}" "UninstallString" '"$INSTDIR\${UNINST_EXE}"'
    WriteRegStr   HKLM "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr   HKLM "${UNINST_KEY}" "Publisher"       "Matthias Kronlachner"
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify"        1
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair"        1
SectionEnd

;--------------------------------
; Uninstaller
Section "Uninstall"
    ; Remove only this order's DLLs so other installed orders are untouched
    Delete "$INSTDIR\*_o${ORDER}.dll"
    Delete "$INSTDIR\${UNINST_EXE}"
    RMDir  "$INSTDIR"  ; removes only if empty (other orders may remain)

    ; Remove ambix Visualizer and folder (only if empty — other installers may share it)
    Delete "$PROGRAMFILES64\ambix\ambix Visualizer.exe"
    RMDir  "$PROGRAMFILES64\ambix"

    ; Remove shortcuts
    Delete "$SMPROGRAMS\ambix\ambix Visualizer.lnk"
    RMDir  "$SMPROGRAMS\ambix"
    Delete "$DESKTOP\ambix Visualizer.lnk"

    ; Note: libfftw3f-3.dll in $SYSDIR is intentionally left in place (shared system DLL)

    DeleteRegKey HKLM "${UNINST_KEY}"
SectionEnd
