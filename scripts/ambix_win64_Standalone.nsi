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
!define UNINST_KEY  "Software\Microsoft\Windows\CurrentVersion\Uninstall\ambix_Standalone"
!define UNINST_NAME "ambix Standalone applications"

; Sign the embedded uninstaller at compile time (enabled by passing /DSIGN_UNINST=1 to makensis)
!ifdef SIGN_UNINST
    !uninstfinalize 'cmd /c sign_file.bat "%1"'
!endif

; The name of the installer
Name "ambix_v${VERSION}_win64"

; The file to write
!system 'mkdir "../_WIN_RELEASE" 2> NUL'
OutFile "../_WIN_RELEASE/ambix_v${VERSION}_Standalone_win64.exe"

; The default installation directory
InstallDir "$PROGRAMFILES64\ambix"

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
; Standalone apps are built as universal binaries supporting ambisonic orders 1-7.
Section "ambix Standalone applications (universal, orders 1-7)"
    SetOutPath "$INSTDIR"
    File "..\build\standalone\*.exe"
    ${DisableX64FSRedirection}
SectionEnd

Section "Start Menu shortcut (ambix Visualizer)" SecStartMenu
    SectionIn 1
    CreateDirectory "$SMPROGRAMS\ambix"
    CreateShortCut "$SMPROGRAMS\ambix\ambix Visualizer.lnk" "$INSTDIR\ambix Visualizer.exe"
SectionEnd

Section /o "Desktop shortcut (ambix Visualizer)" SecDesktop
    CreateShortCut "$DESKTOP\ambix Visualizer.lnk" "$INSTDIR\ambix Visualizer.exe"
SectionEnd

; libfftw3f-3.dll
Section "FFTW3f library"
    SetOutPath "$SYSDIR"
    File "..\win-libs\x64\libfftw3f-3.dll"
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
    ; Remove all standalone executables and the uninstaller itself
    Delete "$INSTDIR\*.exe"
    RMDir  "$INSTDIR"  ; only if empty

    ; Remove shortcuts
    Delete "$SMPROGRAMS\ambix\ambix Visualizer.lnk"
    RMDir  "$SMPROGRAMS\ambix"
    Delete "$DESKTOP\ambix Visualizer.lnk"

    ; Note: libfftw3f-3.dll in $SYSDIR is intentionally left in place (shared system DLL)

    DeleteRegKey HKLM "${UNINST_KEY}"
SectionEnd
