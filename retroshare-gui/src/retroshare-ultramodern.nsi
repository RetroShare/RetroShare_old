; Script generated with the Venis Install Wizard & modified by defnax

; Define your application name
!define APPNAME "RetroShare"
!define VERSION "0.5.1 4049"
!define APPNAMEANDVERSION "${APPNAME} ${VERSION}"
!define QTBASE "D:\qt\2010.01"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\RetroShare"
InstallDirRegKey HKLM "Software\${APPNAME}" ""
OutFile "RetroShare_${VERSION}_setup_ultramodern.exe"
BrandingText "${APPNAMEANDVERSION}"
; Use compression
SetCompressor /SOLID LZMA 

; Modern interface settings
!include Sections.nsh
!include "UMUI.nsh"

;Interface Settings
!define MUI_ABORTWARNING
;!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_BITMAP "retroshare.bmp" ; optional

# MUI defines
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\UltraModernUI\Icon.ico"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_LICENSEPAGE_RADIOBUTTONS
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_FINISHPAGE_LINK "Visit the RetroShare forum for the latest news and support"
!define MUI_FINISHPAGE_LINK_LOCATION "http://retroshare.sourceforge.net/forum/"
!define MUI_FINISHPAGE_RUN "$INSTDIR\RetroShare.exe"
!define MUI_FINISHPAGE_SHOWREADME $INSTDIR\changelog.txt
!define MUI_FINISHPAGE_SHOWREADME_TEXT changelog.txt
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\UltraModernUI\UnIcon.ico"
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!define MUI_LANGDLL_REGISTRY_ROOT HKLM
!define MUI_LANGDLL_REGISTRY_KEY ${REGKEY}
!define UMUI_LANGDLL_REGISTRY_VALUENAME InstallerLanguage

;!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of RetroShare. \r\n\r\nIt is recommended that you close all other applications before starting Setup. This will make it possible to update relevant system files without havinf to reboot your computer. \r\n\r\nIMPORTANT: Ensure that RetroShare is NOT RUNNING before continuing (you can exit from the taskbar menu), otherwise the installer cannot update the executables, and the installation will fail. \r\n\r\nClick Next to continue. "

;!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of RetroShare. \r\n\r\nIMPORTANT: Ensure that RetroShare is NOT RUNNING before continuing (you can exit from the taskbar menu), otherwise the installer cannot update the executables, and the installation will fail. \r\n\r\nClick Next to continue. "


; Defines the un-/installer logo of RetroShare
!insertmacro MUI_DEFAULT MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
!insertmacro MUI_DEFAULT MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"

; Set languages (first is default language)
!insertmacro MUI_RESERVEFILE_LANGDLL
ReserveFile "${NSISDIR}\Plugins\AdvSplash.dll"

;--------------------------------
;Configuration


  ;!insertmacro MUI_RESERVEFILE_SPECIALBITMAP
 
  LicenseLangString myLicenseData 1030 "license\license.txt"
  LicenseLangString myLicenseData 1033 "license\license.txt"
  LicenseLangString myLicenseData 1031 "license\license-GER.txt"
  LicenseLangString myLicenseData 1036 "license\license-FR.txt"
  LicenseLangString myLicenseData 1055 "license\license-TR.txt"
  LicenseLangString myLicenseData 2052 "license\license.txt"
  LicenseLangString myLicenseData 1045 "license\license.txt"
  LicenseLangString myLicenseData 1041 "license\license.txt"
  LicenseLangString myLicenseData 1042 "license\license.txt"
  LicenseLangString myLicenseData 1049 "license\license.txt"
  LicenseLangString myLicenseData 1053 "license\license.txt"

  LicenseData $(myLicenseData)

# Installer pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "$(myLicenseData)"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Installer languages
!define MUI_LANGDLL_ALLLANGUAGES

!insertmacro MUI_LANGUAGE Danish
!insertmacro MUI_LANGUAGE English
!insertmacro MUI_LANGUAGE French
!insertmacro MUI_LANGUAGE German
!insertmacro MUI_LANGUAGE Japanese
!insertmacro MUI_LANGUAGE Korean
!insertmacro MUI_LANGUAGE Polish
!insertmacro MUI_LANGUAGE Russian
!insertmacro MUI_LANGUAGE Swedish
!insertmacro MUI_LANGUAGE SimpChinese
!insertmacro MUI_LANGUAGE Turkish



  ;Component-selection page
    ;Titles
    
    LangString sec_main ${LANG_ENGLISH} "Program Files"
    LangString sec_data ${LANG_ENGLISH} "Program Skins"
    LangString sec_shortcuts ${LANG_ENGLISH} "Shortcuts"
    LangString sec_link ${LANG_ENGLISH} "File Association"
    LangString sec_autostart ${LANG_ENGLISH} "Auto Startup"
    LangString DESC_sec_main ${LANG_ENGLISH} "Installs the RetroShare program files."
    LangString DESC_sec_data ${LANG_ENGLISH} "Installs RetroShare Skins"
    LangString DESC_sec_shortcuts ${LANG_ENGLISH} "Create RetroShare shortcut icons."
    LangString DESC_sec_link ${LANG_ENGLISH} "Associate RetroShare with .rsc file extension"
    LangString LANGUAGEID ${LANG_ENGLISH} "1033"
    
    
    LangString sec_main ${LANG_FRENCH} "RetroShare"
    LangString sec_data ${LANG_FRENCH} "Programme de Skins"
    LangString sec_shortcuts ${LANG_FRENCH} "Raccourcis"
    LangString sec_link ${LANG_FRENCH} "RetroShare fichiers Association"
    LangString sec_startmenu ${LANG_FRENCH} "Raccourcis du menu Démarrer"
    LangString sec_autostart ${LANG_FRENCH} "Démarrage automatique"
    LangString DESC_sec_main ${LANG_FRENCH} "Installe les fichiers du programme."
    LangString DESC_sec_data ${LANG_FRENCH} "Installe RetroShare Skins"
    LangString DESC_sec_startmenu ${LANG_FRENCH} "Crée les raccourcis du menu Démarrer"
    LangString DESC_sec_shortcuts ${LANG_FRENCH} "Crée une icône sur le bureau."
    LangString DESC_sec_link ${LANG_FRENCH} "Associate RetroShare with .rsc file extension"
    LangString LANGUAGEID ${LANG_FRENCH} "1036"

    
    LangString sec_main ${LANG_GERMAN} "Programmdateien"
    LangString sec_data ${LANG_GERMAN} "Skins fuer das Programm"
    LangString sec_shortcuts ${LANG_GERMAN} "Shortcuts"
    LangString sec_link ${LANG_GERMAN} "Dateiverknuepfungen"
    LangString sec_autostart ${LANG_GERMAN} "Auto Startup"
	  LangString DESC_sec_main ${LANG_GERMAN} "Installiert die erforderlichen Programmdateien."
	  LangString DESC_sec_data ${LANG_GERMAN} "Installiert RetroShare Skins"
    LangString DESC_sec_shortcuts ${LANG_GERMAN} "Erstellt eine RetroShare Verkn�pfung im Startmen�, Desktop oder im Schnellstarter."
    LangString DESC_sec_link ${LANG_GERMAN} "RetroShare mit .rsc Dateien verkn�pfen"
    LangString LANGUAGEID ${LANG_GERMAN} "1031"
        
    LangString sec_main ${LANG_TURKISH} "Program Dosyalar�"
    LangString sec_data ${LANG_TURKISH} "Program Skinleri"
    LangString sec_shortcuts ${LANG_TURKISH} "Shortcut'lar"
    LangString sec_link ${LANG_TURKISH} ".rsc Dosya Kaydet"
    LangString sec_autostart ${LANG_TURKISH} "Otomatik calistir ve baglan"
	  LangString DESC_sec_main ${LANG_TURKISH} "Program dosyalar�n� kurar."
	  LangString DESC_sec_data ${LANG_TURKISH} "RetroShare Skin'leri kurar"
    LangString DESC_sec_shortcuts ${TURKISH} "Shortcut yap Start menu , Desktop veya Quicklaunchbar icin."
    LangString DESC_sec_link ${LANG_TURKISH} "RetroShare .rsc almas� i�in kaydettirir"
    LangString LANGUAGEID ${LANG_TURKISH} "1055"
    
    LangString sec_main ${LANG_SIMPCHINESE} "程序文件"
    LangString sec_data ${LANG_SIMPCHINESE} "程序皮肤"
    LangString sec_shortcuts ${LANG_SIMPCHINESE} "快捷方式"
    LangString sec_link ${LANG_SIMPCHINESE} "RetroShare文件关联"
    LangString sec_autostart ${LANG_SIMPCHINESE} "自动启动"
    LangString DESC_sec_main ${LANG_SIMPCHINESE} "安装RetroShare程序"
    LangString DESC_sec_data ${LANG_SIMPCHINESE} "安装RetroShare皮肤"
    LangString DESC_sec_shortcuts ${LANG_SIMPCHINESE} "建RetroShare快捷方式"
    LangString DESC_sec_link ${LANG_SIMPCHINESE} "关联.rsc扩"
    LangString LANGUAGEID ${LANG_SIMPCHINESE} "2052"
    
    LangString sec_main ${LANG_POLISH} "Pliki programu"
    LangString sec_data ${LANG_POLISH} "Skórki"
    LangString sec_shortcuts ${LANG_POLISH} "Skróty"
    LangString sec_link ${LANG_POLISH} "Skojarz pliki"
    LangString sec_autostart ${LANG_POLISH} "Automatyczne uruchamianie"
    LangString DESC_sec_main ${LANG_POLISH} "Instaluje pliki programu RetroShare"
    LangString DESC_sec_data ${LANG_POLISH} "Instaluje skórki programu RetroShare"
    LangString DESC_sec_shortcuts ${LANG_POLISH} "Utwórz ikony skrótów na pulpicie, w menu start oraz na pasku szybkiego uruchamiania."
    LangString DESC_sec_link ${LANG_POLISH} "Skojarz pliki o rozszerzeniu .rsc z RetroShare"
    LangString LANGUAGEID ${LANG_POLISH} "1045"
    
    LangString sec_main ${LANG_DANISH} "Program Files"
    LangString sec_data ${LANG_DANISH} "Program Skins"
    LangString sec_shortcuts ${LANG_DANISH} "Shortcuts"
    LangString sec_link ${LANG_DANISH} "File Association"
    LangString sec_autostart ${LANG_DANISH} "Auto Startup"
    LangString DESC_sec_main ${LANG_DANISH} "Installs the RetroShare program files."
    LangString DESC_sec_data ${LANG_DANISH} "Installs RetroShare Skins"
    LangString DESC_sec_shortcuts ${LANG_DANISH} "Create RetroShare shortcut icons."
    LangString DESC_sec_link ${LANG_DANISH} "Associate RetroShare with .rsc file extension"
    LangString LANGUAGEID ${LANG_DANISH} "1030"
    
    LangString sec_main ${LANG_RUSSIAN} "Program Files"
    LangString sec_data ${LANG_RUSSIAN} "Program Skins"
    LangString sec_shortcuts ${LANG_RUSSIAN} "Shortcuts"
    LangString sec_link ${LANG_RUSSIAN} "File Association"
    LangString sec_autostart ${LANG_RUSSIAN} "Auto Startup"
    LangString DESC_sec_main ${LANG_RUSSIAN} "Installs the RetroShare program files."
    LangString DESC_sec_data ${LANG_RUSSIAN} "Installs RetroShare Skins"
    LangString DESC_sec_shortcuts ${LANG_RUSSIAN} "Create RetroShare shortcut icons."
    LangString DESC_sec_link ${LANG_RUSSIAN} "Associate RetroShare with .rsc file extension"
    LangString LANGUAGEID ${LANG_RUSSIAN} "1049"

    LangString sec_main ${LANG_SWEDISH} "Program Files"
    LangString sec_data ${LANG_SWEDISH} "Program Skins"
    LangString sec_shortcuts ${LANG_SWEDISH} "Shortcuts"
    LangString sec_link ${LANG_SWEDISH} "File Association"
    LangString sec_autostart ${LANG_SWEDISH} "Auto Startup"
    LangString DESC_sec_main ${LANG_SWEDISH} "Installs the RetroShare program files."
    LangString DESC_sec_data ${LANG_SWEDISH} "Installs RetroShare Skins"
    LangString DESC_sec_shortcuts ${LANG_SWEDISH} "Create RetroShare shortcut icons."
    LangString DESC_sec_link ${LANG_SWEDISH} "Associate RetroShare with .rsc file extension"
    LangString LANGUAGEID ${LANG_SWEDISH} "1053"
    
    LangString sec_main ${LANG_JAPANESE} "Program Files"
    LangString sec_data ${LANG_JAPANESE} "Program Skins"
    LangString sec_shortcuts ${LANG_JAPANESE} "Shortcuts"
    LangString sec_link ${LANG_JAPANESE} "File Association"
    LangString sec_autostart ${LANG_JAPANESE} "Auto Startup"
    LangString DESC_sec_main ${LANG_JAPANESE} "Installs the RetroShare program files."
    LangString DESC_sec_data ${LANG_JAPANESE} "Installs RetroShare Skins"
    LangString DESC_sec_shortcuts ${LANG_JAPANESE} "Create RetroShare shortcut icons."
    LangString DESC_sec_link ${LANG_JAPANESE} "Associate RetroShare with .rsc file extension"
    LangString LANGUAGEID ${LANG_JAPANESE} "1041"
    
    LangString sec_main ${LANG_KOREAN} "Program Files"
    LangString sec_data ${LANG_KOREAN} "Program Skins"
    LangString sec_shortcuts ${LANG_KOREAN} "Shortcuts"
    LangString sec_link ${LANG_KOREAN} "File Association"
    LangString sec_autostart ${LANG_KOREAN} "Auto Startup"
    LangString DESC_sec_main ${LANG_KOREAN} "Installs the RetroShare program files."
    LangString DESC_sec_data ${LANG_KOREAN} "Installs RetroShare Skins"
    LangString DESC_sec_shortcuts ${LANG_KOREAN} "Create RetroShare shortcut icons."
    LangString DESC_sec_link ${LANG_KOREAN} "Associate RetroShare with .rsc file extension"
    LangString LANGUAGEID ${LANG_KOREAN} "1042"    
    

!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

Section $(sec_main) sec_main

  ;Set Section required
  SectionIn RO

  ; Set Section properties
  SetOverwrite on

  ; Clears previous error logs
  Delete "$INSTDIR\*.log"
	
  ; Set Section Files and Shortcuts
  SetOutPath "$INSTDIR\"
  File /r "release\RetroShare.exe"
  File /r "..\..\retroshare-nogui\src\release\retroshare-nogui.exe"
  File /r "D:\Qt\2010.01\mingw\bin\mingwm10.dll"
  File /r "D:\Qt\2010.01\qt\bin\QtCore4.dll"
  File /r "D:\Qt\2010.01\qt\bin\QtGui4.dll"
  File /r "D:\Qt\2010.01\qt\bin\QtNetwork4.dll"
  File /r "D:\Qt\2010.01\qt\bin\QtXml4.dll"
  File /r "D:\Qt\2010.01\qt\bin\QtScript4.dll"
  File /r "D:\Qt\2010.01\qt\bin\libgcc_s_dw2-1.dll"
  File /r "D:\Qt\2010.01\qt\plugins\imageformats"
  File /r "D:\Development\miniupnpc-1.3\miniupnpc.dll"
  File /r ${QTBASE}\qt\qt_*.qm
  File /r "release\pthreadGC2d.dll"
  File /r "release\libgpg-error-0.dll"
  File /r "release\libgpgme-11.dll"
  File /r "changelog.txt"
  File /r /x Data "release\bdboot.txt"
  

SectionEnd

Section  $(sec_data) sec_data

  ; Set Section properties
  SetOverwrite on

  ; Set Section Files and Shortcuts
  SetOutPath "$APPDATA\RetroShare\"
  ;File /r "data\*"
  
  ; Set Section Plugins
  SetOutPath "$APPDATA\RetroShare\plugins\"
  ;File /r "plugins\"
  
  ; Set Section qss and exclude svn
  SetOutPath "$INSTDIR\qss\"
  File /r /x .svn qss\*.*
  
  ; Set Section sounds and exclude svn
  SetOutPath "$INSTDIR\sounds\"
  File /r /x .svn sounds\*.*

  ; Set Section skin
  ; SetOutPath "$INSTDIR\skin\"
  ; File /r release\skin\*.* 

  ; Add emoticons
  ;SetOutPath "$INSTDIR\emoticons\"
  ;File /r emoticons\*.*   
	
  ; Add Chat Style
  ;SetOutPath "$INSTDIR\style\"
  ;File /r style\*.*   
	
SectionEnd

; These are the programs that are needed by RetroShare.
Section -Prerequisites
  SetOutPath $INSTDIR\Prerequisites
  MessageBox MB_YESNO "$(InstallGPG4WIN)" /SD IDYES IDNO leave
    File "Prerequisites\gpg4win-1.1.4.exe"
    ExecWait "$INSTDIR\Prerequisites\gpg4win-1.1.4.exe"
     
 leave:
SectionEnd

Section $(sec_link) sec_link
  ; Delete any existing keys


  ; Write the file association
  WriteRegStr HKCR .rsc "" retroshare
  WriteRegStr HKCR retroshare "" "RSC File"
  WriteRegBin HKCR retroshare EditFlags 00000100
  WriteRegStr HKCR "retroshare\shell" "" open
  WriteRegStr HKCR "retroshare\shell\open\command" "" `"$INSTDIR\RetroShare.exe" "%1"`

SectionEnd

SectionGroup $(sec_shortcuts) sec_shortcuts
Section  StartMenu SEC0001

  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\${APPNAME}"
  CreateShortCut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\RetroShare.exe" "" "$INSTDIR\RetroShare.exe" 0
  CreateShortCut "$SMPROGRAMS\${APPNAME}\$(^UninstallLink).lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

SectionEnd

Section  Desktop SEC0002
  

  CreateShortCut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\RetroShare.exe" "" "$INSTDIR\RetroShare.exe" 0
  
SectionEnd

Section  Quicklaunchbar SEC0003
  

  CreateShortCut "$QUICKLAUNCH\${APPNAME}.lnk" "$INSTDIR\RetroShare.exe" "" "$INSTDIR\RetroShare.exe" 0
  
SectionEnd
SectionGroupEnd        

;Section $(sec_autostart) sec_autostart

;  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "RetroRun"   "$INSTDIR\${APPNAME}.exe -a"
  
;SectionEnd

;Section $(sec_autostart) sec_autostart

;  CreateShortCut "$SMSTARTUP\${APPNAME}.lnk" "$INSTDIR\RetroShare.exe" "" "$INSTDIR\RetroShare.exe" 0
;SectionEnd


Section -FinishSection

  WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
  WriteRegStr HKLM "Software\${APPNAME}" "Version" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd



;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${sec_main} $(DESC_sec_main)
    !insertmacro MUI_DESCRIPTION_TEXT ${sec_data} $(DESC_sec_data)
    !insertmacro MUI_DESCRIPTION_TEXT ${sec_shortcuts} $(DESC_sec_shortcuts)
    !insertmacro MUI_DESCRIPTION_TEXT ${sec_link} $(DESC_sec_link)
	;!insertmacro MUI_DESCRIPTION_TEXT ${sec_autostart} $(DESC_sec_autostart)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section "Uninstall"
  
  ; Remove file association registry keys
  DeleteRegKey HKCR .rsc
  DeleteRegKey HKCR retroshare
	
  ; Remove program/uninstall regsitry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
  DeleteRegKey HKLM SOFTWARE\${APPNAME}

  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "RetroRun"

  ; Remove files and uninstaller
  Delete $INSTDIR\RetroShare.exe
  Delete $INSTDIR\*.dll
  Delete $INSTDIR\*.dat
  Delete $INSTDIR\*.txt
  Delete $INSTDIR\*.ini
  Delete $INSTDIR\*.log

  Delete $INSTDIR\uninstall.exe

  ; Remove the kadc.ini file.
  ; Don't remove the directory, otherwise
  ; we lose the XPGP keys.
  ; Should make this an option though...
  Delete "$APPDATA\${APPNAME}\kadc.ini"
  Delete "$APPDATA\${APPNAME}\*.cfg"
  Delete "$APPDATA\${APPNAME}\*.conf"
  Delete "$APPDATA\${APPNAME}\*.log-save"
  Delete "$APPDATA\${APPNAME}\*.log"
  Delete "$APPDATA\${APPNAME}\*.failed"

  RMDir /r "$APPDATA\${APPNAME}\cache"
  RMDir /r "$APPDATA\${APPNAME}\Partials"

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\${APPNAME}\*.*"

  ; Remove desktop shortcut
  Delete "$DESKTOP\${APPNAME}.lnk"
  
  ; Remove Quicklaunch shortcut
  Delete "$QUICKLAUNCH\${APPNAME}.lnk"
  
  ; Remove Autostart 
  ;Delete "$SMSTARTUP\${APPNAME}.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\${APPNAME}"
  RMDir /r "$INSTDIR"
  RMDir /r "$INSTDIR\qss"
  RMDir /r "$INSTDIR\emoticons"
  RMDir /r "$INSTDIR\style"
  RMDir /r "$INSTDIR\translations"

SectionEnd

Function .onInit

    InitPluginsDir
    Push $R1
    File /oname=$PLUGINSDIR\spltmp.bmp "gui\images\splash.bmp"
    advsplash::show 1200 1000 1000 -1 $PLUGINSDIR\spltmp
    Pop $R1
    Pop $R1
    !insertmacro MUI_LANGDLL_DISPLAY



FunctionEnd


# Installer Language Strings
# TODO Update the Language Strings with the appropriate translations.

LangString InstallGPG4WIN ${LANG_ENGLISH}     "Install Gpg4win ? Gpg4win is required for RetroShare!"
LangString InstallGPG4WIN ${LANG_GERMAN}      "Installiere Gpg4win ? Gpg4win ist erforderlich fuer RetroShare!"
LangString InstallGPG4WIN ${LANG_TURKISH}     "Gpg4win Yükle?   Gpg4win RetroShare için gerekli!"
LangString InstallGPG4WIN ${LANG_FRENCH}      "RetroShare a besoin de GPG4win pour fonctionner. Lancer l'installation de GPG4win?"
LangString InstallGPG4WIN ${LANG_SIMPCHINESE} "Install Gpg4win ? Gpg4win是需要Retroshare!" 
LangString InstallGPG4WIN ${LANG_POLISH}      "Install Gpg4win ? Gpg4win wymagane jest Retroshare!"
LangString InstallGPG4WIN ${LANG_DANISH}      "Installer Gpg4win? Gpg4win er nødvendig for RetroShare!"
LangString InstallGPG4WIN ${LANG_JAPANESE}    "Install Gpg4win ? Gpg4win is required for RetroShare!"
LangString InstallGPG4WIN ${LANG_KOREAN}      "Install Gpg4win ? Gpg4win is required for RetroShare!"
LangString InstallGPG4WIN ${LANG_RUSSIAN}     "Install Gpg4win ? Gpg4win is required for RetroShare!"
LangString InstallGPG4WIN ${LANG_SWEDISH}     "Installera Gpg4win? Gpg4win krävs för RetroShare!"

LangString FINISHPAGELINK ${LANG_ENGLISH}     "Visit the RetroShare forums for the latest news and support"
LangString FINISHPAGELINK ${LANG_GERMAN}      "Besuche RetroShare Support Forum "
LangString FINISHPAGELINK ${LANG_TURKISH}     "Destek için Retroshare foruma ziyaret"
LangString FINISHPAGELINK ${LANG_FRENCH}      "Consultez le forum RetroShare pour vous tenir au courant des dernieres modifications, et obtenir de l'aide."
LangString FINISHPAGELINK ${LANG_SIMPCHINESE} "帮助论坛"
LangString FINISHPAGELINK ${LANG_POLISH}      "Odwiedź forum RetroShare do najświeższych informacji i wsparcia"
LangString FINISHPAGELINK ${LANG_DANISH}      "Besøg RetroShare fora for de seneste nyheder og støtte"
LangString FINISHPAGELINK ${LANG_JAPANESE}    "Visit the RetroShare forums for the latest news and support"
LangString FINISHPAGELINK ${LANG_KOREAN}      "Visit the RetroShare forums for the latest news and support"
LangString FINISHPAGELINK ${LANG_RUSSIAN}     "Visit the RetroShare forums for the latest news and support"
LangString FINISHPAGELINK ${LANG_SWEDISH}     "Besök RetroShare forum för de senaste nyheterna och stöd"

LangString ^UninstallLink ${LANG_ENGLISH}     "Uninstall"
LangString ^UninstallLink ${LANG_GERMAN}      "Deinstallieren"
LangString ^UninstallLink ${LANG_TURKISH}     "Kald�r"
LangString ^UninstallLink ${LANG_FRENCH}      "Désinstaller"
LangString ^UninstallLink ${LANG_SIMPCHINESE} "卸载"
LangString ^UninstallLink ${LANG_POLISH}      "Odinstaluj"
LangString ^UninstallLink ${LANG_DANISH}      "Afinstaller"
LangString ^UninstallLink ${LANG_JAPANESE}    "Uninstall"
LangString ^UninstallLink ${LANG_KOREAN}      "Uninstall"
LangString ^UninstallLink ${LANG_RUSSIAN}     "Uninstall"
LangString ^UninstallLink ${LANG_SWEDISH}     "Avinstallera"


; eof
