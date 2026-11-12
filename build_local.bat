@echo off
REM Build the normal production DLL and the named local delivery checks.
setlocal
cd /d "%~dp0"
python -B tools\build_local.py %*
exit /b %errorlevel%
