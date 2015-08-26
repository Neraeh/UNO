@echo off
setlocal
git submodule init
git submodule update
cd %~dp0\src
qmake "CONFIG+=release"
if %errorlevel% neq 0 goto :ERROR
echo La configuration d'UNO n'a eu aucune erreur
echo Vous pouvez utiliser mingw32-make
goto :END


:ERROR
echo Une erreur est survenue, la configuration ne peut pas continuer

:END
endlocal
cd src
