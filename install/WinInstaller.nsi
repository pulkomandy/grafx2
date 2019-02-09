;NSIS Modern User Interface
;Based on the Example Script written by Joost Verburg

!define VERSION 2.7wip2567
!define API sdl

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

Section "Desktop shortcut" SecShortcut

  SetOutPath "$INSTDIR"
  CreateShortCut "$DESKTOP\Grafx2-${API}.lnk" "$INSTDIR\bin\grafx2-${API}.exe" "" "" "" SW_SHOWNORMAL

SectionEnd

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
  !endif
  !if ${API} == "sdl2"
    File ..\bin\SDL2_image.dll
    File ..\bin\SDL2.dll
    File ..\bin\SDL2_ttf.dll
    File ..\bin\${FREETYPEDLL}
  !endif
  File ..\bin\zlib1.dll
  File ..\bin\${PNGDLL}
  File ..\bin\${JPEGDLL}
  File ..\bin\${TIFFDLL}
  SetOutPath "$INSTDIR\share\grafx2"
  File ..\share\grafx2\gfx2.gif
  File ..\share\grafx2\gfx2def.ini
  SetOutPath "$INSTDIR\share\grafx2\skins"
  File ..\share\grafx2\skins\*.png
  # SetOverwrite cannot be skipped by control flow instructions
  # see https://nsis.sourceforge.io/Docs/Chapter4.html#flags
  IfFileExists "$INSTDIR\share\grafx2\scripts\samples\*.*" 0 overwritesamples
  MessageBox MB_YESNO "Do you want to overwrite the scripts\samples directory ?" /SD IDYES IDYES overwritesamples
    SetOverwrite off
    SetOutPath "$INSTDIR\share\grafx2\scripts\samples"
    File /r ..\share\grafx2\scripts\samples\*.*
    SetOverwrite on
    Goto +3
  overwritesamples:
  SetOutPath "$INSTDIR\share\grafx2\scripts\samples"
  File /r ..\share\grafx2\scripts\samples\*.*
  SetOutPath "$INSTDIR\doc"
  File ..\doc\*.txt
  File ..\doc\quickstart.rtf
  SetOutPath "$INSTDIR\share\grafx2\fonts"
  File ..\share\grafx2\fonts\*.png
  File ..\share\grafx2\fonts\*.ttf
  File ..\share\grafx2\fonts\*.pcx

  ; Register in Add/Remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "DisplayName" "GrafX2-${API} (GNU GPL)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "Publisher" "GrafX2 Project Team"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "UninstallString" "$INSTDIR\uninstall-${API}.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "InstalledProductName" "GrafX2-${API}"
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
  # Set the INSTALLSIZE constant (!defined at the top of this script) so Add/Remove Programs can accurately report the size
  SectionGetSize ${SecProgram} $0
  ${If} ${SectionIsSelected} ${SecShortcut}
    SectionGetSize ${SecShortcut} $1
    IntOp $0 $0 + $1
  ${EndIf}
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-${API}" \
                 "EstimatedSize" $0

  ;Store installation folder
  WriteRegStr HKLM "Software\Grafx2" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall-${API}.exe"

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
  RMDir /r "$INSTDIR\share\grafx2\scripts\samples"
  RMDir  "$INSTDIR\share\grafx2\scripts"
  RMDir  "$INSTDIR\share\grafx2"
  RMDir  "$INSTDIR\share"
  Delete "$INSTDIR\Uninstall-${API}.exe"
  
  MessageBox MB_YESNO|MB_DEFBUTTON2|MB_ICONQUESTION "Do you wish to keep your configuration settings ?" IDYES keepconfig IDNO deleteconfig
  deleteconfig:
  !if ${API} == "win32"
    Delete "$INSTDIR\gfx2-win32.cfg"
    Delete "$APPDATA\Grafx2\gfx2-win32.cfg"
  !else if ${API} == "sdl2"
    Delete "$INSTDIR\gfx2-sdl2.cfg"
    Delete "$APPDATA\Grafx2\gfx2-sdl2.cfg"
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
  Delete "$DESKTOP\Grafx2-${API}.lnk"
SectionEnd
  
