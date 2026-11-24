@echo off
setlocal EnableExtensions
cd /d "%~dp0"

rem OSimX Windows build helper: matches the Qt / MinGW installation in the supplied build log.
if not defined OSIMX_QT_ROOT set "OSIMX_QT_ROOT=C:\Qt\6.11.2\mingw_64"
if not defined OSIMX_MINGW_ROOT set "OSIMX_MINGW_ROOT=C:\Qt\Tools\mingw1310_64"

if not exist "%OSIMX_QT_ROOT%\bin\windeployqt.exe" (
    echo [ERROR] Qt tools not found: %OSIMX_QT_ROOT%
    echo Set OSIMX_QT_ROOT to your Qt MinGW kit folder and run this script again.
    pause
    exit /b 1
)
if not exist "%OSIMX_MINGW_ROOT%\bin\g++.exe" (
    echo [ERROR] MinGW not found: %OSIMX_MINGW_ROOT%
    echo Set OSIMX_MINGW_ROOT to your Qt MinGW toolchain folder.
    pause
    exit /b 1
)

set "PATH=%OSIMX_QT_ROOT%\bin;%OSIMX_MINGW_ROOT%\bin;%PATH%"
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake is not on PATH.
    pause
    exit /b 1
)
where ninja >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Ninja is not on PATH.
    pause
    exit /b 1
)

rem Windows does not let the linker overwrite a running .exe.
taskkill /F /IM osimx.exe >nul 2>&1

echo [1/4] Configuring OSimX...
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="%OSIMX_QT_ROOT%" -DCMAKE_CXX_COMPILER="%OSIMX_MINGW_ROOT%\bin\g++.exe" -DCMAKE_BUILD_TYPE=Release -DOSIMX_BUILD_APP=ON -DOSIMX_BUILD_TESTS=ON
if errorlevel 1 goto failed

echo [2/4] Building OSimX...
cmake --build build --parallel 4
if errorlevel 1 goto failed

echo [3/4] Running backend regression tests...
ctest --test-dir build --output-on-failure
if errorlevel 1 goto failed

echo [4/4] Deploying Qt runtime and launching...
"%OSIMX_QT_ROOT%\bin\windeployqt.exe" --release --compiler-runtime "build\osimx.exe"
if errorlevel 1 goto failed
start "" "build\osimx.exe"
echo OSimX build and launch completed.
exit /b 0

:failed
echo.
echo [ERROR] A build/test/deployment step failed. Read the error above.
pause
exit /b 1
