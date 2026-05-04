@echo off
setlocal
cd /d "%~dp0"

set BUILD_DIR=..\build
if not defined MSBUILD set MSBUILD="C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe"
if not defined MAKENSIS set MAKENSIS="C:\Program Files (x86)\NSIS\makensis.exe"
set MSBUILD_FLAGS=/t:Build /p:Configuration=Release /p:PreBuildEvent= /p:PostBuildEvent=
set CMAKE_COMMON=-DNUM_OUTPUTS_DECODER=64 -DFFTW3_INCLUDE_DIR="../win-libs/" -DFFTW3F_LIBRARY="../win-libs/x64/libfftw3f-3.lib"

REM Allow CI (or anyone with Eigen3 outside the standard search paths) to point
REM cmake at it via env. Local Windows builds with Eigen3 installed system-wide
REM (or on cmake's default search path) don't need this.
if defined EIGEN3_INCLUDE_DIR set CMAKE_COMMON=%CMAKE_COMMON% -DEIGEN3_INCLUDE_DIR="%EIGEN3_INCLUDE_DIR%"

REM ── Code signing configuration ───────────────────────────────────────────────
REM   Set SIGN_CERT to the .p12 certificate file and SIGN_PASS to its password.
REM   Pass "sign" as a build argument to enable signing.
REM   All of SIGNTOOL / SIGN_CERT / SIGN_PASS / INSTALLER_CERT / INSTALLER_PASS
REM   can be pre-set in the environment (e.g. by CI) — defaults below only apply
REM   if the variable is not already defined.
if not defined SIGNTOOL set SIGNTOOL="C:\Program Files (x86)\Windows Kits\10\bin\10.0.19041.0\x64\signtool.exe"
if not defined SIGN_CERT set SIGN_CERT=DevIDApplication.p12
if not defined SIGN_PASS set SIGN_PASS=
if not defined INSTALLER_CERT set INSTALLER_CERT=DevIDInstaller.p12
if not defined INSTALLER_PASS set INSTALLER_PASS=

REM ── Parse arguments ──────────────────────────────────────────────────────────
REM Usage: build_all_win64.bat [vst2] [vst3] [standalone] [sign]
REM Default (no args): build all three formats without signing

set BUILD_VST2=0
set BUILD_VST3=0
set BUILD_SA=0
set SIGN=0

if "%~1"=="" (
    set BUILD_VST2=1
    set BUILD_VST3=1
    set BUILD_SA=1
    goto :start_build
)
:parse_loop
if /i "%~1"=="vst2"       set BUILD_VST2=1
if /i "%~1"=="vst3"       set BUILD_VST3=1
if /i "%~1"=="standalone" set BUILD_SA=1
if /i "%~1"=="sign"       set SIGN=1
shift
if not "%~1"=="" goto :parse_loop

:start_build
echo.
echo Build targets:  VST2=%BUILD_VST2%  VST3=%BUILD_VST3%  Standalone=%BUILD_SA%  Sign=%SIGN%
echo.

REM ── VST3 + Standalone universal builds (orders 1-7, single cmake pass) ───────
if "%BUILD_VST3%%BUILD_SA%"=="11" (
    echo ================================================================
    echo  BUILDING VST3 + Standalone  ^(universal, orders 1-7^)
    echo ================================================================
    pushd "%BUILD_DIR%"
    cmake .. %CMAKE_COMMON% -DBUILD_VST3=TRUE -DBUILD_VST=FALSE -DBUILD_STANDALONE=TRUE -DMAX_AMBI_ORDER=7
    %MSBUILD% ambix_plugin_suite.sln %MSBUILD_FLAGS%
    popd
) else if "%BUILD_VST3%"=="1" (
    echo ================================================================
    echo  BUILDING VST3  ^(universal, orders 1-7^)
    echo ================================================================
    pushd "%BUILD_DIR%"
    cmake .. %CMAKE_COMMON% -DBUILD_VST3=TRUE -DBUILD_VST=FALSE -DBUILD_STANDALONE=FALSE -DMAX_AMBI_ORDER=7
    %MSBUILD% ambix_plugin_suite.sln %MSBUILD_FLAGS%
    popd
) else if "%BUILD_SA%"=="1" (
    echo ================================================================
    echo  BUILDING Standalone  ^(universal, orders 1-7^)
    echo ================================================================
    pushd "%BUILD_DIR%"
    cmake .. %CMAKE_COMMON% -DBUILD_VST3=FALSE -DBUILD_VST=FALSE -DBUILD_STANDALONE=TRUE -DMAX_AMBI_ORDER=7
    %MSBUILD% ambix_plugin_suite.sln %MSBUILD_FLAGS%
    popd
)

REM Sign VST3 bundles (inner PE files inside bundle directories)
if "%BUILD_VST3%"=="1" if "%SIGN%"=="1" (
    echo Signing VST3 plugins...
    for /r "..\build\vst3" %%f in (*.vst3) do call :do_sign "%%f"
)

REM Sign Standalone executables
if "%BUILD_SA%"=="1" if "%SIGN%"=="1" (
    echo Signing Standalone executables...
    call :do_sign "..\build\standalone\*.exe"
)

REM Sign ambix_visualizer (bundled with VST2/VST3 installers even without standalone target)
if "%BUILD_SA%"=="0" if "%SIGN%"=="1" (
    if exist "..\build\standalone\ambix_visualizer.exe" (
        echo Signing ambix_visualizer...
        call :do_sign "..\build\standalone\ambix_visualizer.exe"
    )
)

REM ── VST2 per-order builds (orders 1, 3, 5, 7) ────────────────────────────────
REM   Each order outputs to its own vst_oX directory; sign immediately after build.
if "%BUILD_VST2%"=="1" (
    for %%x in (1 3 5 7) do (
        echo.
        echo ================================================================
        echo  BUILDING VST2  order %%x
        echo ================================================================
        pushd "%BUILD_DIR%"
        cmake .. %CMAKE_COMMON% -DBUILD_VST3=FALSE -DBUILD_VST=TRUE -DBUILD_STANDALONE=FALSE -DAMBI_ORDER:STRING=%%x
        %MSBUILD% ambix_plugin_suite.sln %MSBUILD_FLAGS%
        popd
        if "%SIGN%"=="1" (
            echo Signing order %%x DLLs...
            call :do_sign "..\build\vst_o%%x\*.dll"
        )
    )
)

REM ── Package installers ────────────────────────────────────────────────────────
REM   Download Nullsoft's Scriptable Install System (NSIS) from:
REM   https://sourceforge.net/projects/nsis/
REM   When SIGN=1, pass /DSIGN_UNINST=1 so each .nsi signs its embedded uninstaller
REM   via !uninstfinalize (signtool + cert are inherited from this bat's env).
set NSIS_SIGN_FLAG=
if "%SIGN%"=="1" set NSIS_SIGN_FLAG=/DSIGN_UNINST=1

echo.
if "%BUILD_VST3%"=="1" (
    echo Packaging VST3 installer...
    %MAKENSIS% /V4 %NSIS_SIGN_FLAG% ambix_win64_VST3.nsi
)
if "%BUILD_SA%"=="1" (
    echo Packaging Standalone installer...
    %MAKENSIS% /V4 %NSIS_SIGN_FLAG% ambix_win64_Standalone.nsi
)
if "%BUILD_VST2%"=="1" (
    for %%x in (1 3 5 7) do (
        echo Packaging VST2 order %%x installer...
        %MAKENSIS% /V4 %NSIS_SIGN_FLAG% /DORDER=%%x ambix_win64_VST2.nsi
    )
)

REM Sign the packaged installers
if "%SIGN%"=="1" (
    echo Signing installers...
    call :do_sign_installer "..\_WIN_RELEASE\*_win64.exe"
)

goto :eof

REM ── Subroutine: two-pass signing (SHA1 legacy + SHA256 dual-sign) ─────────────
REM   Usage: call :do_sign <path-or-glob>
:do_sign
%SIGNTOOL% sign /f "%SIGN_CERT%" /p "%SIGN_PASS%" /t http://timestamp.comodoca.com %*
%SIGNTOOL% sign /f "%SIGN_CERT%" /p "%SIGN_PASS%" /fd sha256 /tr http://timestamp.comodoca.com/?td=sha256 /td sha256 /as /v %*
goto :eof

:do_sign_installer
%SIGNTOOL% sign /f "%INSTALLER_CERT%" /p "%INSTALLER_PASS%" /t http://timestamp.comodoca.com %*
%SIGNTOOL% sign /f "%INSTALLER_CERT%" /p "%INSTALLER_PASS%" /fd sha256 /tr http://timestamp.comodoca.com/?td=sha256 /td sha256 /as /v %*
goto :eof
