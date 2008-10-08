@  Grafx2 - The Ultimate 256-color bitmap paint program
@
@  Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)
@
@  Grafx2 is free software; you can redistribute it and/or
@  modify it under the terms of the GNU General Public License
@  as published by the Free Software Foundation; version 2
@  of the License.
@
@  Grafx2 is distributed in the hope that it will be useful,
@  but WITHOUT ANY WARRANTY; without even the implied warranty of
@  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
@  GNU General Public License for more details.
@
@  You should have received a copy of the GNU General Public License
@  along with Grafx2; if not, see <http://www.gnu.org/licenses/> or
@  write to the Free Software Foundation, Inc.,
@  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

@echo off

rem
rem This batch file will run GrafX2 with a disk cache of 16 megabytes.
rem Use it ONLY IF you don't have enough memory to run GrafX2.
rem
rem (Ce fichier de commandes lancera GrafX2 avec un cache disque de 16 Mo.
rem Ne l'utilisez QUE SI vous n'avez pas assez de m‚moire pour lancer GrafX2.)
rem



rem
rem Modify the following line in order to indicate the path to DOS4GW.EXE.
rem (Modifiez la ligne suivante afin d'indiquer le chemin de DOS4GW.EXE)
rem

set PATHDOS4GW=DOS4GW.EXE


rem
rem Modify the following line in order to indicate the path to GFX2.EXE.
rem (Modifiez la ligne suivante afin d'indiquer le chemin de GFX2.EXE)
rem

set PATHGFX2=C:\GFX2\GFX2.EXE


if "%1"=="/novm" goto NOVM
rem
rem The following line may not work correctly on some computers. In this case,
rem please read the DOS4GW manual which is not supplied with GrafX2.
rem
rem (La ligne suivante peut ne pas fonctionner correctement sur certains
rem ordinateurs. Dans ce cas, veuillez vous r‚f‚rer au manuel de DOS4GW qui
rem n'est pas fourni avec GrafX2)
rem

set DOS4GVM=1
:NOVM


rem
rem The following lines will execute GrafX2
rem (Les lignes suivantes ‚x‚cuteront GrafX2)
rem

if not exist %PATHDOS4GW% goto DOSNOTFOUND
if not exist %PATHGFX2% goto GFXNOTFOUND

if "%1"=="/novm" goto RUNNOVM
%PATHDOS4GW% %PATHGFX2% %1
goto END
:RUNNOVM
%PATHDOS4GW% %PATHGFX2% %2
goto END

:DOSNOTFOUND
echo DOS4GW.EXE not found: please edit the GFX2_MEM.BAT file and enter its location.
goto END
:GFXNOTFOUND
echo GFX2.EXE not found: please edit the GFX2_MEM.BAT file and enter its location.
:END


rem
rem The following lines will remove all the environnement variables.
rem (Les lignes suivantes supprimeront toutes les variables d'environnement)
rem

set DOS4GVM=
set PATHGFX2=
set PATHDOS4GW=
