@echo off
setlocal
cd %~dp0\src
qmake "CONFIG+=release"
if %errorlevel% neq 0 goto :ERROR
echo You can now launch mingw32-make
goto :END


:ERROR
echo Error

:END
endlocal
cd src
