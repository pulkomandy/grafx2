;NSIS Modern User Interface
;Based on the Example Script written by Joost Verburg

!define VERSION 2.5.1946
!define API sdl

!define INSTALLSIZE 10072

!define FREETYPEDLL libfreetype-6.dll
!define JPEGDLL     libjpeg-9.dll
!define PNGDLL      libpng16-16.dll
!define TIFFDLL     libtiff-5.dll

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Grafx2"
  OutFile "grafx2-${API}-${VERSION}.win32.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Grafx2"
  !define MULTIUSER_INSTALLMODE_INSTDIR "Grafx2"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Grafx2" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

  SetCompressor /SOLID LZMA
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
    Exec $INSTDIR\bin\grafx2-${API}.exe
  norun:
FunctionEnd

;--------------------------------
;Installer Sections

Section "Grafx2" SecProgram
  SectionIn RO
  SetOutPath "$INSTDIR"
  ;ADD YOUR OWN FILES HERE...
  File ..\src\gfx2.ico
  File "..\src-${VERSION}.tgz"
  SetOutPath "$INSTDIR\bin"
  File ..\bin\grafx2-${API}.exe
  !if ${API} == "sdl"
    File ..\bin\SDL_image.dll
    File ..\bin\SDL.dll
    File ..\bin\SDL_ttf.dll
    File ..\bin\${FREETYPEDLL}
    File ..\bin\${JPEGDLL}
    File ..\bin\${TIFFDLL}
  !endif
  File ..\bin\zlib1.dll
  File ..\bin\${PNGDLL}
  SetOutPath "$INSTDIR\share\grafx2"
  File ..\share\grafx2\gfx2.gif
  File ..\share\grafx2\gfx2def.ini
  SetOutPath "$INSTDIR\share\grafx2\skins"
  File ..\share\grafx2\skins\*.png
  SetOutPath "$INSTDIR\share\grafx2\scripts\samples_2.4"
  File /r ..\share\grafx2\scripts\samples_2.4\*.*
  SetOutPath "$INSTDIR\doc"
  File ..\doc\*.txt
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
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "DisplayName" "GrafX2-${API} (GNU GPL)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "Publisher" "GrafX2 Project Team"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "UninstallString" "$INSTDIR\uninstall-${API}.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "InstalledProductName" "GrafX2"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "InstalledLocation" $INSTDIR
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "DisplayIcon" "$INSTDIR\gfx2.ico"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "URLInfoAbout" "http://grafx2.tk"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "NoModify" 1
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "NoRepair" 1

  ;Store installation folder
  WriteRegStr HKLM "Software\Grafx2" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall-${API}.exe"

SectionEnd

Section "Desktop shortcut" SecShortcut

  SetOutPath "$INSTDIR"
  CreateShortCut "$DESKTOP\Grafx2.lnk" "$INSTDIR\bin\grafx2-${API}.exe" "" "" "" SW_SHOWNORMAL

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
  Delete "$INSTDIR\gfx2.ico"
  Delete "$INSTDIR\bin\grafx2-${API}.exe"
  Delete "$INSTDIR\src-${VERSION}.tgz"
  Delete "$INSTDIR\share\grafx2\gfx2.gif"
  Delete "$INSTDIR\share\grafx2\gfx2def.ini"
  Delete "$INSTDIR\bin\SDL_image.dll"
  Delete "$INSTDIR\bin\SDL.dll"
  Delete "$INSTDIR\bin\SDL_ttf.dll"
  Delete "$INSTDIR\bin\${JPEGDLL}"
  Delete "$INSTDIR\bin\${FREETYPEDLL}"
  Delete "$INSTDIR\bin\zlib1.dll"
  Delete "$INSTDIR\bin\${PNGDLL}"
  Delete "$INSTDIR\bin\${TIFFDLL}"
  Delete "$INSTDIR\bin\stdout.txt"
  Delete "$INSTDIR\bin\stderr.txt"
  RMDir  "$INSTDIR\bin"
  RMDir /r "$INSTDIR\doc"
  RMDir /r "$INSTDIR\share\grafx2\fonts"
  RMDir /r "$INSTDIR\share\grafx2\skins"
  RMDir /r "$INSTDIR\share\grafx2\scripts\samples_2.4"
  RMDir  "$INSTDIR\share\grafx2\scripts"
  RMDir  "$INSTDIR\share\grafx2"
  RMDir  "$INSTDIR\share"
  Delete "$INSTDIR\Uninstall-${API}.exe"
  
  MessageBox MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION "Do you wish to keep your configuration settings ?" IDYES keepconfig IDNO deleteconfig
  deleteconfig:
  !if ${API} == "win32"
    Delete "$INSTDIR\gfx2-win32.cfg"
    Delete "$APPDATA\Grafx2\gfx2-win32.cfg"
  !else
    Delete "$INSTDIR\gfx2.cfg"
    Delete "$APPDATA\Grafx2\gfx2.cfg"
  !endif
  Delete "$INSTDIR\gfx2.ini"
  Delete "$APPDATA\Grafx2\gfx2.ini"
  RMDir  "$APPDATA\Grafx2"
  keepconfig:
  
  RMDir "$INSTDIR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}"

  DeleteRegKey /ifempty HKLM "Software\Grafx2"

SectionEnd

Section "un.SecShortcut"
  Delete "$DESKTOP\Grafx2.lnk"
SectionEnd
  
