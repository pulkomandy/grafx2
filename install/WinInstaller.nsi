;NSIS Modern User Interface
;Based on the Example Script written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "Grafx2"
  OutFile "grafx2-2.00b99.0-svn724-win32.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\Grafx2"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\Grafx2" ""

  ;Request application privileges for Windows Vista
  RequestExecutionLevel user

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "..\doc\gpl-2.0.txt"
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

Function .onInstSuccess
  MessageBox MB_YESNO "Run GrafX2 now ?" IDNO norun
    Exec $INSTDIR\GrafX2.exe
  norun:
FunctionEnd

;--------------------------------
;Installer Sections

Section "Grafx2" SecProgram
  SectionIn RO
  SetOutPath "$INSTDIR"
  ;ADD YOUR OWN FILES HERE...
  File ..\grafx2.exe
  File ..\src-svn724.tgz
  File ..\gfx2.gif
  File ..\gfx2def.ini
  File ..\SDL_image.dll
  File ..\SDL.dll
  File ..\libfreetype-6.dll
  File ..\SDL_ttf.dll
  File ..\zlib1.dll
  File ..\libpng13.dll
  SetOutPath "$INSTDIR\skins"
  File ..\skins\base.gif
  SetOutPath "$INSTDIR\doc"
  File ..\doc\README.txt
  File ..\doc\COMPILING.txt
  File ..\doc\README-SDL_ttf.txt
  File ..\doc\README-SDL.txt
  File ..\doc\README-SDL_image.txt
  File ..\doc\README-zlib1.txt
  File ..\doc\gpl-2.0.txt
  SetOutPath "$INSTDIR\fonts"
  File ..\fonts\8pxfont.png
  File ..\fonts\Tuffy.ttf

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
                 "URLInfoAbout" "http://grafx2.googlecode.com"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Grafx2-SDL" \
                 "DisplayVersion" "99%"
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
  CreateShortCut "$DESKTOP\Grafx2.lnk" "$INSTDIR\Grafx2.exe" "" "" "" SW_SHOWNORMAL

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
  Delete "$INSTDIR\grafx2.exe"
  Delete "$INSTDIR\src-svn724.tgz"
  Delete "$INSTDIR\gfx2.gif"
  Delete "$INSTDIR\gfx2def.ini"
  Delete "$INSTDIR\SDL_image.dll"
  Delete "$INSTDIR\SDL.dll"
  Delete "$INSTDIR\libfreetype-6.dll"
  Delete "$INSTDIR\SDL_ttf.dll"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\libpng13.dll"
  Delete "$INSTDIR\doc\README.txt"
  Delete "$INSTDIR\doc\COMPILING.txt"
  Delete "$INSTDIR\doc\README-SDL_ttf.txt"
  Delete "$INSTDIR\doc\README-SDL.txt"
  Delete "$INSTDIR\doc\README-SDL_image.txt"
  Delete "$INSTDIR\doc\README-zlib1.txt"
  Delete "$INSTDIR\doc\gpl-2.0.txt"
  RMDir  "$INSTDIR\doc"
  Delete "$INSTDIR\fonts\8pxfont.png"
  Delete "$INSTDIR\fonts\Tuffy.ttf"
  RMDir  "$INSTDIR\fonts"
  Delete "$INSTDIR\skins\base.gif"
  RMDir  "$INSTDIR\skins"
  
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
  
