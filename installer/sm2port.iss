; Inno Setup 6.6+ script for sm2port. Built by tools/package.ps1:
;   ISCC /DAppVersion=<x.y.z> /DSourceDir=<staged files> /DOutputDir=dist installer\sm2port.iss
; Installs for the current user (no administrator rights), so the launcher's updates run without a prompt.
; Settings, saves, log and shader cache live in %LOCALAPPDATA%\sm2port and survive uninstalling.

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef SourceDir
  #define SourceDir "..\dist\stage"
#endif
#ifndef OutputDir
  #define OutputDir "..\dist"
#endif

[Setup]
AppId={{8C3E1B2A-5D4F-4E7A-9B61-3F2D7A0C5E91}
AppName=sm2port
AppVersion={#AppVersion}
AppVerName=sm2port {#AppVersion}
AppPublisher=SickSartori
AppPublisherURL=https://github.com/SickSartori/sm2port
AppSupportURL=https://github.com/SickSartori/sm2port/issues
AppUpdatesURL=https://github.com/SickSartori/sm2port/releases
AppComments=Unofficial native Windows port of Spider-Man 2 (Xbox, PAL: Italy or Europe). No game files included.
VersionInfoVersion={#AppVersion}
VersionInfoDescription=sm2port setup
DefaultDirName={autopf}\sm2port
DefaultGroupName=sm2port
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x86compatible
DisableWelcomePage=no
SetupIconFile=..\res\sm2.ico
UninstallDisplayIcon={app}\sm2.exe
UninstallDisplayName=sm2port - Spider-Man 2
WizardStyle=modern dark
WizardImageFile=wizard-100.png,wizard-150.png,wizard-200.png,wizard-250.png
WizardSmallImageFile=wizard-small-100.png,wizard-small-150.png,wizard-small-200.png,wizard-small-250.png
ShowLanguageDialog=auto
CloseApplications=yes
RestartApplications=no
OutputDir={#OutputDir}
OutputBaseFilename=sm2port-{#AppVersion}-windows-x86-setup
Compression=lzma2/max
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"

[Messages]
english.WelcomeLabel2=This will install [name/ver], an unofficial native Windows port of Spider-Man 2.%n%nNo game files are included: on the first start you choose the disc image (ISO) of your own Xbox copy (Italy, or Europe in French, German or Spanish).%n%nYour settings and saves are kept in your user folder and stay when you update or uninstall.
italian.WelcomeLabel2=Verrà installato [name/ver], un port nativo non ufficiale per Windows di Spider-Man 2.%n%nNessun file del gioco è incluso: al primo avvio scegli l'immagine del disco (ISO) della tua copia Xbox (Italia, oppure Europa in francese, tedesco o spagnolo).%n%nImpostazioni e salvataggi restano nella tua cartella utente e non si perdono con aggiornamenti o disinstallazione.

[CustomMessages]
english.DesktopShortcut=Create a desktop shortcut
italian.DesktopShortcut=Crea un collegamento sul desktop
english.ShortcutName=Spider-Man 2 (sm2port)
italian.ShortcutName=Spider-Man 2 (sm2port)
english.StartNow=Start sm2port
italian.StartNow=Avvia sm2port

[Tasks]
Name: "desktopicon"; Description: "{cm:DesktopShortcut}"

[Files]
Source: "{#SourceDir}\sm2.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\sm2runtime.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\README.txt"; DestDir: "{app}"; Flags: ignoreversion isreadme
Source: "{#SourceDir}\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\THIRD_PARTY.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\licenses\*"; DestDir: "{app}\licenses"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{cm:ShortcutName}"; Filename: "{app}\sm2.exe"
Name: "{autodesktop}\{cm:ShortcutName}"; Filename: "{app}\sm2.exe"; Tasks: desktopicon

[Run]
; After a normal install, offered on the last page; after a silent update started by a launcher of 0.2.4 or earlier
; (/UPDATE), always. Newer launchers only download the installer and show it, and the player starts it.
Filename: "{app}\sm2.exe"; Description: "{cm:StartNow}"; Flags: nowait postinstall skipifsilent
Filename: "{app}\sm2.exe"; Flags: nowait; Check: IsLauncherUpdate

[Code]
function IsLauncherUpdate: Boolean;
var
  I: Integer;
begin
  Result := False;
  for I := 1 to ParamCount do
    if CompareText(ParamStr(I), '/UPDATE') = 0 then
      Result := True;
end;

// Windows caches the icons it has shown; after sm2.exe is replaced, the desktop and Start menu shortcuts kept the
// old one. Telling the shell that associations changed makes it reload them.
procedure SHChangeNotify(wEventId: Integer; uFlags: Cardinal; dwItem1, dwItem2: Cardinal);
  external 'SHChangeNotify@shell32.dll stdcall';

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
    SHChangeNotify($08000000, 0, 0, 0);  { SHCNE_ASSOCCHANGED, SHCNF_IDLIST }
end;
