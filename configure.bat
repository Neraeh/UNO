@echo off
setlocal
echo Assurez vous que git, qmake et mingw32-make sont dans votre PATH avant de lancer ce script
set /P usurebro=Voulez-vous lancer la configuration ? (o/n) 
if /I "%usurebro%" neq "o" goto :END

:BEGIN
git submodule init
git submodule update
cd %~dp0\tools\version\src
qmake "CONFIG+=release"
if %errorlevel% neq 0 goto :ERROR
mingw32-make
if %errorlevel% neq 0 goto :ERROR
cd %~dp0\tools\version\src
call release\version.exe
if %errorlevel% neq 0 goto :ERROR
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
