set QTDIR=d:\qt\2010.01
set MINGW=%QTDIR%\mingw

set PATH=%QTDIR%\qt\bin;%QTDIR%\bin;%MINGW%\bin;%PATH%

mingw32-make clean 

qmake libbitdht.pro

mingw32-make

pause

