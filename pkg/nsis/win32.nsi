!ifndef CONFIGURATION
!define /ifndef CONFIGURATION "release"
!echo "CONFIGURATION defaulted to '${CONFIGURATION}'"
!endif

!ifndef VERSION
!error "VERSION not defined"
!endif

!ifndef SPECIAL_BUILD
!define /ifndef SPECIAL_BUILD "Internal"
!echo "SPECIAL_BUILD defaulted to '${SPECIAL_BUILD}'"
!endif

SetCompressor lzma
!include "MUI2.nsh"
!include "DumpLog.nsh"

# Installer Attributes: General Attributes.
InstallDir "$PROGRAMFILES\Bipolar"
Name "Bipolar"
OutFile Bipolar-${VERSION}.exe
RequestExecutionLevel highest # Required for Windows Vista+
XPStyle on

# Variables.
Var StartMenuFolder

# Modern UI2 Interface Configuration.
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${NSISDIR}\Contrib\Graphics\Header\win.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp"
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_ABORTWARNING

# Modern UI2 Install Pages.
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "gpl-3.0.rtf"
!insertmacro MUI_PAGE_DIRECTORY
#!insertmacro MUI_PAGE_COMPONENTS
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Paul Colby\Bipolar"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "startMenuFolder"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES

# Modern UI2 Uninstall Pages.
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

# Modern UI2 Languages.
!insertmacro MUI_LANGUAGE "English"

# Custom install pages.

# Sections to install.

Section "application"
    # Files to install.
    SetOutPath $INSTDIR
    File "..\..\src\${CONFIGURATION}\Bipolar.exe"
    File /r "qtlibs\*"
    WriteRegStr HKCU "Software\Software\Paul Colby\Bipolar" "" $INSTDIR
    WriteUninstaller $INSTDIR\Uninstall.exe
    # The various shortcuts.
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Bipolar.lnk" "$INSTDIR\Bipolar.exe"
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
        CreateShortCut "$DESKTOP\Bipolar.lnk" "$INSTDIR\Bipolar.exe"
    !insertmacro MUI_STARTMENU_WRITE_END
    # Windows' add/remove programs information.
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar" "DisplayName" "Bipolar"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar" "DisplayIcon" "$\"$INSTDIR\Bipolar.exe$\",0"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar" "Publisher" "Paul Colby"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar" "URLInfoAbout" "https://github.com/pcolby/bipolar"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar" "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
    WriteRegDword HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar" "NoModify" 1
    WriteRegDword HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar" "NoRepair" 1
SectionEnd

Function installMicrosoftRuntime
    exch $R0
    push $0
    retry:
    ClearErrors
    DetailPrint "Installing Microsoft Visual C++ Redistributable [$R0]"
    ExecWait '"$R0" /norestart /q' $0
    ${If} ${Errors}
        MessageBox MB_ABORTRETRYIGNORE \
            "The Microsoft Visual C++ Redistributable package failed to execute." \
            /SD IDIGNORE IDRETRY retry IDIGNORE ignore
            Abort
    ${Else}
        DetailPrint "$R0 returned exit code $0"
        ${If} $0 == 0
            DetailPrint "Microsoft Visual C++ Redistributable installed successfully"
        ${Else}
            MessageBox MB_ABORTRETRYIGNORE \
                "The Microsoft Visual C++ Redistributable failed to install." \
                /SD IDIGNORE IDRETRY retry IDIGNORE ignore
                Abort
        ${EndIf}
    ${EndIf}
    ignore:
    pop $0
    pop $R0
FunctionEnd

Section "-vcredist"
    ${If} ${FileExists} "$INSTDIR\vcredist_x86.exe"
        push "$INSTDIR\vcredist_x86.exe"
        call installMicrosoftRuntime
    ${EndIf}
    ${If} ${FileExists} "$INSTDIR\vcredist_x64.exe"
        push "$INSTDIR\vcredist_x64.exe"
        call installMicrosoftRuntime
    ${EndIf}
SectionEnd

Section "hook"
    SetOutPath $INSTDIR\hook
    File "Qt5Network.dll"
    push $0
    retry:
    ClearErrors
    ExecWait '"$INSTDIR\Bipolar.exe" -install-hook' $0
    ${If} ${Errors}
        MessageBox MB_ABORTRETRYIGNORE \
            "The Polar FlowSync hook could not be installed.$\n$\nPlease ensure Polar FlowSync is not running before trying again." \
            /SD IDIGNORE IDRETRY retry IDIGNORE ignore
            Abort
    ${Else}
        DetailPrint "Bipolar returned exit code $0 while installing hook"
        ${If} $0 == 0
            DetailPrint "Hook installed successfully."
        ${Else}
            MessageBox MB_ABORTRETRYIGNORE \
                "The Polar FlowSync hook could not be installed.$\n$\nPlease ensure Polar FlowSync is not running before trying again." \
                /SD IDIGNORE IDRETRY retry IDIGNORE ignore
                Abort
        ${EndIf}
    ${EndIf}
    ignore:
    pop $0
SectionEnd

Function .onInstFailed
  Push "$INSTDIR\Install.log"
  Call DumpLog
FunctionEnd

Function .onInstSuccess
  Push "$INSTDIR\Install.log"
  Call DumpLog
FunctionEnd

# Sections to uninstall.

Section "un.hook"
    RMDir /r $INSTDIR\hook
    # @todo Reinstate the original DLL too.
SectionEnd

Section "un.application"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Bipolar"
    Delete "$DESKTOP\Bipolar.lnk"
    !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\Bipolar.lnk"
    RMDir "$SMPROGRAMS\$StartMenuFolder"
    Delete $INSTDIR\Bipolar.exe
    Delete $INSTDIR\D3Dcompiler_*.dll
    Delete $INSTDIR\icu*.dll
    Delete $INSTDIR\libEGL.dll
    Delete $INSTDIR\libgcc*.dll
    Delete $INSTDIR\libGLESv2.dll
    Delete $INSTDIR\libstdc++*.dll
    Delete $INSTDIR\libwinpthread*.dll
    Delete $INSTDIR\Install.log
    Delete $INSTDIR\msvc*.dll
    Delete $INSTDIR\msvc*.exe
    Delete $INSTDIR\opengl*.dll
    Delete $INSTDIR\Qt5*.dll
    Delete $INSTDIR\qt_*.qm
    Delete $INSTDIR\Uninstall.exe
    RMDir /r $INSTDIR\accessible
    RMDir /r $INSTDIR\iconengines
    RMDir /r $INSTDIR\imageformats
    RMDir /r $INSTDIR\platforms
    RMDir /r $INSTDIR\translations
    RMDir $INSTDIR
    DeleteRegKey /ifempty HKCU "Software\Paul Colby\Bipolar"
    DeleteRegKey /ifempty HKCU "Software\Paul Colby"
SectionEnd

# Installer Attributes: Version Information.
VIProductVersion "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "https://github.com/pcolby/bipolar"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Paul Colby"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Bipolar installer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" "Bipolar-${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "2014-2019 Paul Colby"
#VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "Bipolar-${VERSION}.exe"
#VIAddVersionKey /LANG=${LANG_ENGLISH} "PrivateBuild" ""
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "Bipolar"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "SpecialBuild" "${SPECIAL_BUILD}"
