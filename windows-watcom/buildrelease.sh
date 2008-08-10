#!/bin/sh
rm grafx2-beta-svn*.zip
wine ide grafx2.wpj
zip -j grafx2-beta-svn`svnversion`.zip grafx2.exe ../gfx2.dat ../gfx2.cfg ../gfx2.ini ~/.wine/drive_c/WATCOM/binnt/mt7r17.dll ~/.wine/drive_c/WATCOM/binnt/clbr17.dll ~/.wine/drive_c/windows/system32/SDL.dll
