#define Version GetEnv('VERSION')
#define ProjectName GetEnv('PROJECT_NAME')
#define ProductName GetEnv('PRODUCT_NAME')
#define Publisher GetEnv('COMPANY_NAME')
#define Year GetDateTimeString("yyyy","","")

; 'Types': What get displayed during the setup
[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

; Components are used inside the script and can be composed of a set of 'Types'
[Components]
Name: "standalone"; Description: "Standalone application"; Types: full custom
Name: "vst3"; Description: "VST3 plugin"; Types: full custom
Name: "clap"; Description: "CLAP plugin"; Types: full custom
Name: "lv2"; Description: "LV2 plugin"; Types: full custom

[Setup]
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
AppName={#ProductName}
OutputBaseFilename={#ProductName}-{#Version}-Windows
AppCopyright=Copyright (C) {#Year} {#Publisher}
AppPublisher={#Publisher}
AppVersion={#Version}
DefaultDirName="{commoncf64}\VST3\{#ProductName}.vst3"
DisableDirPage=yes

LicenseFile="..\..\LICENSE.txt"
UninstallFilesDir="{commonappdata}\{#ProductName}\uninstall"

; MSVC adds a .ilk when building the plugin. Let's not include that.
[Files]
Source: "..\..\Builds\{#ProjectName}_artefacts\Release\VST3\{#ProductName}.vst3\*"; DestDir: "{commoncf64}\VST3\{#ProductName}.vst3\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs; Components: vst3
Source: "..\..\Builds\{#ProjectName}_artefacts\Release\CLAP\{#ProductName}.clap"; DestDir: "{commoncf64}\CLAP\"; Flags: ignoreversion recursesubdirs; Components: clap
Source: "..\..\Builds\{#ProjectName}_artefacts\Release\LV2\{#ProductName}.lv2\*"; DestDir: "{commoncf64}\LV2\{#ProductName}.lv2\"; Excludes: *.ilk; Flags: ignoreversion recursesubdirs; Components: lv2
Source: "..\..\Builds\{#ProjectName}_artefacts\Release\Standalone\{#ProductName}.exe"; DestDir: "{commonpf64}\{#Publisher}\{#ProductName}"; Flags: ignoreversion; Components: standalone

[Icons]
Name: "{autoprograms}\{#ProductName}"; Filename: "{commonpf64}\{#Publisher}\{#ProductName}\{#ProductName}.exe"; Components: standalone
Name: "{autoprograms}\Uninstall {#ProductName}"; Filename: "{uninstallexe}"

[Code]
function InitializeSetup(): Boolean;
var
  ResultCode: Integer;
begin
  Result := True;

  if MsgBox('IMPORTANT: Please close all DAWs and audio applications before continuing.' + #13#10#13#10 +
            'This includes:' + #13#10 +
            '  • Reaper, Ableton Live, FL Studio, etc.' + #13#10 +
            '  • Plugin scanners and host applications' + #13#10 +
            '  • Any audio software using {#ProductName}' + #13#10#13#10 +
            'Click OK to continue with installation, or Cancel to exit.',
            mbInformation, MB_OKCANCEL) = IDCANCEL then
  begin
    Result := False;
  end;
end;