#!/bin/bash

cat <<EOT
<?xml version="1.0" encoding="windows-1252"?>
<Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
  <Product Name="Mobile Data Usage" Id="DDBA0377-9C6C-43A4-889B-2836EAE3D2E8" UpgradeCode="0164927C-F840-4A64-8035-4B145D7CB96A" Language="1033" Codepage="1252" Version="1.0.0" Manufacturer="Schlachta Software">

    <Package Id="*" Keywords="Installer" Description="Mobile Data Usage Installer" Comments="Tray Icon" Manufacturer="Schlachta Software" InstallerVersion="100" Languages="1033" SummaryCodepage="1252" Compressed="yes"/>

    <Media Id="1" Cabinet="application.cab" EmbedCab="yes"/>

    <Icon Id="icon.ico" SourceFile="..\mdu-notifier.ico"/>
    <Property Id="ARPPRODUCTICON" Value="icon.ico" />
    
    <Property Id="WIXUI_EXITDIALOGOPTIONALCHECKBOXTEXT" Value="Launch Mobile Data Usage"  />
    <Property Id="WixShellExecTarget" Value="[#mdunotifier.exe]" />
    <CustomAction Id="LaunchApplication" BinaryKey="WixCA" DllEntry="WixShellExec" Impersonate="yes" />

    <UI>
      <UIRef Id="WixUI_FeatureTree" />

      <Publish Dialog="WelcomeDlg"
            Control="Next"
            Event="NewDialog"
            Value="CustomizeDlg"
            Order="2">1</Publish>
      <Publish Dialog="CustomizeDlg"
            Control="Back"
            Event="NewDialog"
            Value="WelcomeDlg"
            Order="2">1</Publish>
      <Publish Dialog="ExitDialog"
        Control="Finish" 
        Event="DoAction" 
        Value="LaunchApplication">WIXUI_EXITDIALOGOPTIONALCHECKBOX = 1 and NOT Installed
        </Publish>
    </UI>

    <Directory Id="TARGETDIR" Name="SourceDir">
        <Directory Id="ProgramFilesFolder">
           <Directory Id="APPLICATIONROOTDIRECTORY" Name="Mobile Data Usage"/>
        </Directory>

        <Directory Id="ProgramMenuFolder" Name="Programs">
            <Directory Id="ApplicationMenuFolder" Name="Mobile Data Usage">
                 <Component Id="ApplicationShortcut" Guid="f78e1c6e-455f-4cf8-8d45-0b7e7aa918a2">
                    <Shortcut Id="ApplicationStartMenuShortcut" Advertise="no" 
                         Name="Mobile Data Usage" 
                        Target="[#mdunotifier.exe]"
                              WorkingDirectory="APPLICATIONROOTDIRECTORY"/>
                    <RemoveFolder Id="ApplicationMenuFolder" On="uninstall"/>   
                   <RegistryValue Root="HKCU" Key="Software\mdu-notifier"
                           Type="integer" Value="1" Name="installed" KeyPath="yes" />
                  
                 </Component>            
            </Directory>
                <Directory Id="StartupFolder" Name="Startup">
                 <Component Id="StartupShortcut" Guid="b5a2e82d-4435-4a14-9e79-71c23e0b1679">
                    <Shortcut Id="StartupMenuShortcut" Advertise="no" 
                         Name="Mobile Data Usage" 
                        Target="[#mdunotifier.exe]"
                              WorkingDirectory="APPLICATIONROOTDIRECTORY"/>
                    <RemoveFolder Id="StartupMenuFolder" On="uninstall"/>   
                   <RegistryValue Root="HKCU" Key="Software\mdu-notifier"
                           Type="integer" Value="1" Name="startup" KeyPath="yes" />
                  
                 </Component>            
            </Directory>
        </Directory>
    </Directory>

    <DirectoryRef Id="APPLICATIONROOTDIRECTORY">
EOT

COMPGEN=`pwd`/compgen.sh
FEATGEN=`pwd`/featgen.sh

cd "$1/$2"

find * -maxdepth 0 -exec "$COMPGEN" {} \; | sed 's,"[^"]*'"$2"',"'"$2"',g'

cat <<EOT
    </DirectoryRef>

    <Feature Id="MainApplication" Title="Mobile data usage tray icon" Level="1" Absent="disallow">
EOT

find * -maxdepth 0 -exec "$FEATGEN" {} \; 

cat <<EOT                    
    </Feature>

    <Feature Id="ApplicationShortcut" Title="Start menu folder" Level="1">
        <ComponentRef Id="ApplicationShortcut" />  
    </Feature>

    <Feature Id="StartupShortcut" Title="Add to startup programs" Level="1">
        <ComponentRef Id="StartupShortcut" />  
    </Feature>
</Product>
</Wix>
EOT
