@echo off
REM Sign a single file using the two-pass SHA1 + SHA256 scheme.
REM Invoked by NSIS !uninstfinalize to sign the embedded uninstaller during installer compilation.
REM Expects SIGNTOOL, SIGN_CERT, SIGN_PASS to be set in the environment (inherited from build_all_win64.bat).
%SIGNTOOL% sign /f "%SIGN_CERT%" /p "%SIGN_PASS%" /t http://timestamp.comodoca.com %1
%SIGNTOOL% sign /f "%SIGN_CERT%" /p "%SIGN_PASS%" /fd sha256 /tr http://timestamp.comodoca.com/?td=sha256 /td sha256 /as /v %1
