; Compile script using Nullsoft Scriptable Install System (NSIS) on windows

; Build Unicode installer.
; Must precede Name/OutFile and any header-changing directive — newer
; NSIS (3.x via choco on the GH-hosted runner) errors out otherwise.
Unicode True

;--------------------------------
!include x64.nsh
!include "MUI2.nsh"
!include "installer_common.nsh"

; load the version from file
!define /file VERSION "../VERSION"

; Uninstaller identity (shown in Programs and Features)
!define UNINST_KEY  "Software\Microsoft\Windows\CurrentVersion\Uninstall\ambix_VST3"
!define UNINST_NAME "ambix VST3 plugin suite"

; Sign the embedded uninstaller at compile time (enabled by passing /DSIGN_UNINST=1 to makensis)
!ifdef SIGN_UNINST
    !uninstfinalize 'cmd /c sign_file.bat "%1"'
!endif

; The name of the installer
Name "ambix_v${VERSION}_win64"

; The file to write
!system 'mkdir "../_WIN_RELEASE" 2> NUL'
OutFile "../_WIN_RELEASE/ambix_v${VERSION}_VST3_win64.exe"

; The default installation directory
InstallDir "$PROGRAMFILES64\Common Files\VST3\ambix"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
; Pages
!insertmacro MUI_PAGE_WELCOME

!define MUI_TEXT_WELCOME_INFO_TITLE "ambix v${VERSION}"

!insertmacro MUI_PAGE_LICENSE "../README.md"
!insertmacro MUI_PAGE_COMPONENTS
Page directory
Page instfiles

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install
; VST3 plugins are built as universal binaries supporting ambisonic orders 1-7.
; JUCE builds VST3 as directory bundles on Windows; /r copies the bundle correctly.
Section "ambix VST3 plugins (universal, orders 1-7)"
    SetOutPath "$INSTDIR"
    File /r "..\build\vst3\*.vst3"
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

; Write uninstaller + Programs-and-Features registration (hidden, always runs)
Section "-Uninstaller"
    SetOutPath "$INSTDIR"
    WriteUninstaller "$INSTDIR\uninstall.exe"
    WriteRegStr   HKLM "${UNINST_KEY}" "DisplayName"     "${UNINST_NAME}"
    WriteRegStr   HKLM "${UNINST_KEY}" "DisplayVersion"  "${VERSION}"
    WriteRegStr   HKLM "${UNINST_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegStr   HKLM "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr   HKLM "${UNINST_KEY}" "Publisher"       "Matthias Kronlachner"
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoModify"        1
    WriteRegDWORD HKLM "${UNINST_KEY}" "NoRepair"        1
SectionEnd

;--------------------------------
; Uninstaller
Section "Uninstall"
    ; Remove VST3 bundles (directory bundles) and the VST3\ambix folder itself
    RMDir /r "$INSTDIR"

    ; Remove ambix Visualizer and folder (only if empty — other installers may share it)
    Delete "$PROGRAMFILES64\ambix\ambix Visualizer.exe"
    RMDir  "$PROGRAMFILES64\ambix"

    ; Remove shortcuts
    Delete "$SMPROGRAMS\ambix\ambix Visualizer.lnk"
    RMDir  "$SMPROGRAMS\ambix"
    Delete "$DESKTOP\ambix Visualizer.lnk"

    DeleteRegKey HKLM "${UNINST_KEY}"
SectionEnd
