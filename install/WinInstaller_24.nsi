;NSIS Modern User Interface
;Based on the Example Script written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Grafx2"
  OutFile "grafx2-2.4.2023.win32.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Grafx2"
  !define MULTIUSER_INSTALLMODE_INSTDIR "Grafx2"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Grafx2" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !define  MUI_WELCOMEFINISHPAGE_BITMAP vector.bmp
  !define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
  !insertmacro MUI_PAGE_WELCOME
  !define MULTIUSER_EXECUTIONLEVEL Highest
  !define MULTIUSER_MUI
  !define MULTIUSER_INSTALLMODE_COMMANDLINE
  ;!define MUI_HEADERIMAGE_BITMAP logo_scenish.bmp
  ;!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
  !insertmacro MUI_PAGE_LICENSE "..\doc\gpl-2.0.txt"
  !include MultiUser.nsh
  !insertmacro MULTIUSER_PAGE_INSTALLMODE
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Functions

Function .onInit
  !insertmacro MULTIUSER_INIT
FunctionEnd

Function un.onInit
  !insertmacro MULTIUSER_UNINIT
FunctionEnd

Function .onInstSuccess
  MessageBox MB_YESNO "Run GrafX2 now ?" IDNO norun
    Exec $INSTDIR\bin\grafx2.exe
  norun:
FunctionEnd

;--------------------------------
;Installer Sections

Section "Grafx2" SecProgram
  SectionIn RO
  SetOutPath "$INSTDIR"
  ;ADD YOUR OWN FILES HERE...
  File ..\src-2.4wip2023.tgz
  SetOutPath "$INSTDIR\bin"
  File ..\bin\grafx2.exe
  File ..\bin\SDL_image.dll
  File ..\bin\SDL.dll
  File ..\bin\libfreetype-6.dll
  File ..\bin\SDL_ttf.dll
  File ..\bin\zlib1.dll
  File ..\bin\libpng14-14.dll
  SetOutPath "$INSTDIR\share\grafx2"
  File ..\share\grafx2\gfx2.gif
  File ..\share\grafx2\gfx2def.ini
  SetOutPath "$INSTDIR\share\grafx2\skins"
  File ..\share\grafx2\skins\font_Classic.png
  File ..\share\grafx2\skins\font_DPaint.png
  File ..\share\grafx2\skins\font_Fairlight.png
  File ..\share\grafx2\skins\font_Fun.png
  File ..\share\grafx2\skins\font_Melon.png
  File ..\share\grafx2\skins\font_Seen.png
  File ..\share\grafx2\skins\skin_Aurora.png
  File ..\share\grafx2\skins\skin_classic.png
  File ..\share\grafx2\skins\skin_DPaint.png
  File ..\share\grafx2\skins\skin_modern.png
  File ..\share\grafx2\skins\skin_scenish.png
  SetOutPath "$INSTDIR\share\grafx2\scripts\samples_2.4\brush"
  File ..\share\grafx2\scripts\samples_2.4\brush\ApplyColor.lua
  File ..\share\grafx2\scripts\samples_2.4\brush\Fisheye.lua
  File ..\share\grafx2\scripts\samples_2.4\brush\GrayscaleAvg.lua
  File ..\share\grafx2\scripts\samples_2.4\brush\GrayscaleDesat.lua
  File ..\share\grafx2\scripts\samples_2.4\brush\Halfsmooth.lua
  File ..\share\grafx2\scripts\samples_2.4\brush\Waves.lua
  SetOutPath "$INSTDIR\share\grafx2\scripts\samples_2.4\demo"
  File ..\share\grafx2\scripts\samples_2.4\demo\3DPalette.lua
  File ..\share\grafx2\scripts\samples_2.4\demo\Ellipse.lua
  File ..\share\grafx2\scripts\samples_2.4\demo\FlipPicture.lua
  File ..\share\grafx2\scripts\samples_2.4\demo\SierpinskyCarpet.lua
  File ..\share\grafx2\scripts\samples_2.4\demo\SierpinskyTriangle.lua
  File ..\share\grafx2\scripts\samples_2.4\demo\Spritesheet.lua
  SetOutPath "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\brush"
  File ..\share\grafx2\scripts\samples_2.4\demo\brush\Amigaball.lua
  File ..\share\grafx2\scripts\samples_2.4\demo\brush\ColorSphere.lua
  File ..\share\grafx2\scripts\samples_2.4\demo\brush\FindAA.lua
  File ..\share\grafx2\scripts\samples_2.4\demo\brush\Mandelbrot.lua
  SetOutPath "$INSTDIR\share\grafx2\scripts\samples_2.4\libs"
  File ..\share\grafx2\scripts\samples_2.4\libs\dawnbringer_lib.lua
  File ..\share\grafx2\scripts\samples_2.4\libs\memory.lua
  SetOutPath "$INSTDIR\share\grafx2\scripts\samples_2.4\palette"
  File ..\share\grafx2\scripts\samples_2.4\palette\Desaturate.lua
  File ..\share\grafx2\scripts\samples_2.4\palette\ExpandColors.lua
  File ..\share\grafx2\scripts\samples_2.4\palette\FillColorCube.lua
  File ..\share\grafx2\scripts\samples_2.4\palette\InvertedRGB.lua
  File ..\share\grafx2\scripts\samples_2.4\palette\Set3bit.lua
  File ..\share\grafx2\scripts\samples_2.4\palette\Set6bit.lua
  File ..\share\grafx2\scripts\samples_2.4\palette\SetC64Palette.lua
  File ..\share\grafx2\scripts\samples_2.4\palette\ShiftHue.lua
  SetOutPath "$INSTDIR\share\grafx2\scripts\samples_2.4\picture"
  File ..\share\grafx2\scripts\samples_2.4\picture\CellColourReducer.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\DrawGridIsometric.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\DrawgridOrthogonal_Index.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\DrawGridOrthogonal_RGB.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\GlassGridFilter.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\PaletteToPicture.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\Pic2isometric.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\Rainbow-Dark2Bright.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\RemapImage2RGB.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\RemapImage2RGB_ed.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\RemapImageTo3bitPal.lua
  File ..\share\grafx2\scripts\samples_2.4\Tiler.lua
  File ..\share\grafx2\scripts\samples_2.4\picture\XBitColourXpaceFromPalette.lua
  SetOutPath "$INSTDIR\doc"
  File ..\doc\COMPILING.txt
  File ..\doc\PF_fonts.txt
  File ..\doc\README-SDL.txt
  File ..\doc\README-SDL_image.txt
  File ..\doc\README-SDL_ttf.txt
  File ..\doc\README-lua.txt
  File ..\doc\README-zlib1.txt
  File ..\doc\README.txt
  File ..\doc\gpl-2.0.txt
  SetOutPath "$INSTDIR\share\grafx2\fonts"
  File ..\share\grafx2\fonts\8pxfont.png
  File ..\share\grafx2\fonts\Tuffy.ttf
  File ..\share\grafx2\fonts\PF_Arma_5__.png
  File ..\share\grafx2\fonts\PF_Easta_7_.png
  File ..\share\grafx2\fonts\PF_Easta_7__.png
  File ..\share\grafx2\fonts\PF_Ronda_7__.png
  File ..\share\grafx2\fonts\PF_Tempesta_5.png
  File ..\share\grafx2\fonts\PF_Tempesta_5_.png
  File ..\share\grafx2\fonts\PF_Tempesta_5__.png
  File ..\share\grafx2\fonts\PF_Tempesta_5___.png
  File ..\share\grafx2\fonts\PF_Tempesta_7.png
  File ..\share\grafx2\fonts\PF_Tempesta_7_.png
  File ..\share\grafx2\fonts\PF_Tempesta_7__.png
  File ..\share\grafx2\fonts\PF_Tempesta_7___.png
  File ..\share\grafx2\fonts\PF_Westa_7_.png
  File ..\share\grafx2\fonts\PF_Westa_7__.png

  ; Register in Add/Remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "DisplayName" "GrafX2 (GNU GPL)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "InstalledProductName" "GrafX2"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "InstalledLocation" $INSTDIR
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "DisplayIcon" "$INSTDIR\gfx2.ico"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "URLInfoAbout" "http://grafx2.org"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "DisplayVersion" "2.4.wip2023"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "NoModify" 1
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "NoRepair" 1

  ;Store installation folder
  WriteRegStr HKLM "Software\Grafx2" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Desktop shortcut" SecShortcut

  SetOutPath "$INSTDIR"
  CreateShortCut "$DESKTOP\Grafx2.lnk" "$INSTDIR\bin\grafx2.exe" "" "" "" SW_SHOWNORMAL

SectionEnd


;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecProgram  ${LANG_ENGLISH} "Grafx2 application and runtime data."
  LangString DESC_SecShortcut ${LANG_ENGLISH} "Desktop shortcut."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecProgram} $(DESC_SecProgram)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcut} $(DESC_SecShortcut)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "un.SecProgram"

  ;ADD YOUR OWN FILES HERE...
  Delete "$INSTDIR\bin\grafx2.exe"
  Delete "$INSTDIR\src-2.4.wip2023.tgz"
  Delete "$INSTDIR\share\grafx2\gfx2.gif"
  Delete "$INSTDIR\share\grafx2\gfx2def.ini"
  Delete "$INSTDIR\bin\SDL_image.dll"
  Delete "$INSTDIR\bin\SDL.dll"
  Delete "$INSTDIR\bin\libfreetype-6.dll"
  Delete "$INSTDIR\bin\SDL_ttf.dll"
  Delete "$INSTDIR\bin\zlib1.dll"
  Delete "$INSTDIR\bin\libpng14-14.dll"
  Delete "$INSTDIR\bin\stdout.txt"
  Delete "$INSTDIR\bin\stderr.txt"
  RMDir  "$INSTDIR\bin"
  Delete "$INSTDIR\doc\COMPILING.txt"
  Delete "$INSTDIR\doc\PF_fonts.txt"
  Delete "$INSTDIR\doc\README-SDL.txt"
  Delete "$INSTDIR\doc\README-SDL_image.txt"
  Delete "$INSTDIR\doc\README-SDL_ttf.txt"
  Delete "$INSTDIR\doc\README-lua.txt"
  Delete "$INSTDIR\doc\README-zlib1.txt"
  Delete "$INSTDIR\doc\README.txt"
  Delete "$INSTDIR\doc\gpl-2.0.txt"
  RMDir  "$INSTDIR\doc"
  Delete "$INSTDIR\share\grafx2\fonts\8pxfont.png"
  Delete "$INSTDIR\share\grafx2\fonts\Tuffy.ttf"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Arma_5__.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Easta_7_.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Easta_7__.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Ronda_7__.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Tempesta_5.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Tempesta_5_.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Tempesta_5__.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Tempesta_5___.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Tempesta_7.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Tempesta_7_.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Tempesta_7__.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Tempesta_7___.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Westa_7_.png"
  Delete "$INSTDIR\share\grafx2\fonts\PF_Westa_7__.png"
  RMDir  "$INSTDIR\share\grafx2\fonts"
  Delete "$INSTDIR\share\grafx2\skins\font_Classic.png"
  Delete "$INSTDIR\share\grafx2\skins\font_Fun.png"
  Delete "$INSTDIR\share\grafx2\skins\font_Fairlight.png"
  Delete "$INSTDIR\share\grafx2\skins\font_Melon.png"
  Delete "$INSTDIR\share\grafx2\skins\font_DPaint.png"
  Delete "$INSTDIR\share\grafx2\skins\font_Seen.png"
  Delete "$INSTDIR\share\grafx2\skins\skin_classic.png"
  Delete "$INSTDIR\share\grafx2\skins\skin_Aurora.png"
  Delete "$INSTDIR\share\grafx2\skins\skin_modern.png"
  Delete "$INSTDIR\share\grafx2\skins\skin_DPaint.png"
  Delete "$INSTDIR\share\grafx2\skins\skin_scenish.png"
  RMDir  "$INSTDIR\share\grafx2\skins"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\brush\ApplyColor.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\brush\Fisheye.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\brush\GrayscaleAvg.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\brush\GrayscaleDesat.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\brush\Halfsmooth.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\brush\Waves.lua"
  RMDir  "$INSTDIR\share\grafx2\scripts\samples_2.4\brush"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\brush\Amigaball.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\brush\ColorSphere.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\brush\FindAA.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\brush\Mandelbrot.lua"
  RMDir  "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\brush"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\3DPalette.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\Ellipse.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\FlipPicture.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\SierpinskyCarpet.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\SierpinskyTriangle.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\demo\Spritesheet.lua"
  RMDir  "$INSTDIR\share\grafx2\scripts\samples_2.4\demo"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\libs\dawnbringer_lib.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\libs\memory.lua"
  RMDir  "$INSTDIR\share\grafx2\scripts\samples_2.4\libs"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\palette\Desaturate.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\palette\ExpandColors.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\palette\FillColorCube.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\palette\InvertedRGB.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\palette\Set3bit.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\palette\Set6bit.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\palette\SetC64Palette.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\palette\ShiftHue.lua"
  RMDir  "$INSTDIR\share\grafx2\scripts\samples_2.4\palette"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\CellColourReducer.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\DrawGridIsometric.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\DrawgridOrthogonal_Index.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\DrawGridOrthogonal_RGB.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\GlassGridFilter.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\PaletteToPicture.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\Pic2isometric.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\Rainbow-Dark2Bright.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\RemapImage2RGB.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\RemapImage2RGB_ed.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\RemapImageTo3bitPal.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\Tiler.lua"
  Delete "$INSTDIR\share\grafx2\scripts\samples_2.4\picture\XBitColourXpaceFromPalette.lua"
  RMDir  "$INSTDIR\share\grafx2\scripts\samples_2.4\picture"
  RMDir  "$INSTDIR\share\grafx2\scripts\samples_2.4"
  RMDir  "$INSTDIR\share\grafx2\scripts"
  RMDir  "$INSTDIR\share\grafx2"
  RMDir  "$INSTDIR\share"
  Delete "$INSTDIR\Uninstall.exe"

  MessageBox MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION "Do you wish to keep your configuration settings ?" IDYES keepconfig IDNO deleteconfig
  deleteconfig:
  Delete "$INSTDIR\gfx2.cfg"
  Delete "$INSTDIR\gfx2.ini"
  Delete "$APPDATA\Grafx2\gfx2.cfg"
  Delete "$APPDATA\Grafx2\gfx2.ini"
  RMDir  "$APPDATA\Grafx2"
  keepconfig:

  RMDir "$INSTDIR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL"

  DeleteRegKey /ifempty HKLM "Software\Grafx2"

SectionEnd

Section "un.SecShortcut"
  Delete "$DESKTOP\Grafx2.lnk"
SectionEnd
