# This file is part of BOINC.
# http://boinc.berkeley.edu
# Copyright (C) 2023 University of California
#
# BOINC is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License
# as published by the Free Software Foundation,
# either version 3 of the License, or (at your option) any later version.
#
# BOINC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

param
(
    [Parameter(ParameterSetName='build')][ValidateSet('x64','x64_vbox','arm64')][string]$Type = "x64", # type of the build
    [Parameter(ParameterSetName='build')][switch]$CI, # will set appropriate env overrides in CI environment
    [Parameter(Mandatory=$true,ParameterSetName='build')][ValidateNotNullOrEmpty()][string]$Version, # version string to use 
    [Parameter(Mandatory=$true,ParameterSetName='build')][ValidateNotNullOrEmpty()][string]$Certificate, # certificate file to use for signing
    [Parameter(Mandatory=$true,ParameterSetName='build')][ValidateNotNullOrEmpty()][string]$CertificatePass, # certificate password

    [Parameter(Mandatory=$true,ParameterSetName='clean')][switch]$CleanOnly # skips all steps and only cleans the build dir
)

$ErrorActionPreference = 'Stop' # makes invoked commands throw error instead of warning

$VboxInstaller = 'VirtualBox-7.0.10-158379-Win.exe'
$Configuration = 'Release'

function WriteStep {
    param($msg)
    Write-Output "Step [$msg]"
}

function Header {
    Write-Output "*********************************"
    Write-Output "**** BOINC INSTALLER BUILDER ****"
    Write-Output "*********************************"
    Write-Output ""
}

function Report {
    param($result,$msg)
    Write-Output ""
    Write-Output "*********************************"
    if ( $result ) {
        Write-Output "**** RESULT: SUCCESS         ****"
    } else {
        Write-Output "**** ERROR: $msg"
        Write-Output "**** RESULT: FAILURE         ****"
    }
    Write-Output "*********************************"

    if ( $result ) {
        exit 0
    }
    exit 1
}

# Checks the 3 prerequisites to execute the script: powershell version 7 at least, msbuild and WiX toolkit installed
function CheckPrerequisites {
    WriteStep "Requirements check: Powershell >= 7"
    if ( $PSVersionTable.PSVersion.Major -lt 7 ) {
        Write-Error "Found powershell version $PSVersionTable.PSVersion, required >= 7"
        Report $false
    }

    WriteStep "Requirements check: MSBuild"
    try {
        msbuild --version
    }
    Catch {
        Report $false
    }

    WriteStep "Requirements check: Wix Toolkit"
    try {
        heat -?
    }
    Catch {
        Report $false
    }
}

function CleanBuildDir {
    WriteStep "Cleanup"
    try { Remove-Item -Recurse -Path build\src* } # Previous bundles sources
    Catch {
        # ignore
    }

    try { Remove-Item -Path build\*bundle.exe } # Previous built bundles
    Catch {
        # ignore
    }

    try { Remove-Item -Path build\*.winpdb } # debug symbols
    Catch {
        # ignore
    }

    try { Remove-Item -Recurse -Path build\en-us } # Previous MSI
    Catch {
        # ignore
    }
}

# Method that checks of a given path, be it a directory or file, for presence or its absence
function CheckPath {
    param
    (
        [string]$Path,
        [Parameter(Mandatory=$false)][switch]$IsDir, # path is actually a directory
        [Parameter(Mandatory=$false)][switch]$ExpectNotPresent # when true, fails with path present, fails with path absent otherwise
    )
    
    $realpath = ""
    if( [System.IO.Path]::IsPathRooted("$Path") ) {
        $realpath = $Path
    } else {
        $realpath = (Join-Path $PSScriptRoot "$Path")
    }

    Write-Output "[Checking: $path]"

    $result = $false
    if( $IsDir ) {
        $result = Test-Path -Path $realpath
    } else {
        $result = Test-Path -Path $realpath -PathType Leaf
    }

    if( $ExpectNotPresent -and $result ) {
        Report $false "Found a path not expected: $realpath"
    }
    if( !$ExpectNotPresent -and !$result ) {
        Report $false "Could not find expected path: $realpath"
    }
}

# Checks the build directory setup, which must be already present before launching
function CheckBuildDir {
    try {
        WriteStep "Check directories"
        CheckPath -Path "build\prerequisites" -IsDir
        CheckPath -Path "build\locale" -IsDir
        CheckPath -Path "build\res" -IsDir
        CheckPath -Path "build\Skins" -IsDir

        WriteStep "Check binary files"
        CheckPath -Path "build\boinc.exe"
        CheckPath -Path "build\boinccmd.exe"
        CheckPath -Path "build\boincsvcctrl.exe"
        CheckPath -Path "build\ca-bundle.crt"
        CheckPath -Path "build\boincmgr.exe"
        CheckPath -Path "build\boinctray.exe"

        if ( !($Type -eq "arm64") ) {
            CheckPath -Path "build\boinc.scr"
        }

        WriteStep "Check prerequisites"
        if ( $Type -eq "x64_vbox" ) {
            CheckPath -Path "build\prerequisites\$VboxInstaller"
        } else {
            CheckPath -Path "build\prerequisites\$VboxInstaller" -ExpectNotPresent
        }
    }
    Catch {
        Report $false
    }
}

# Adds to the build directory some files from source which are not bundled in artifacts
function CopyAdditionalSourceFiles {
    try {
        WriteStep "LiberationMono-Regular copy"
        Copy-Item -Force -Path "api\ttf\liberation-fonts-ttf-2.00.0\LiberationMono-Regular.ttf" -Destination "build\LiberationMono-Regular.ttf"
        
        WriteStep "COPYING copy"
        Copy-Item -Force -Path "COPYING" -Destination "build\COPYING"
        
        WriteStep "COPYRIGHT copy"
        Copy-Item -Force -Path "COPYRIGHT" -Destination "build\COPYRIGHT"
        
        WriteStep "boinc_logo_black.jpg copy"
        Copy-Item -Force -Path "doc\logo\boinc_logo_black.jpg" -Destination "build\boinc_logo_black.jpg"
    }
    Catch {
        Report $false
    }
}

# Executes the installer build with the appropriate parameters for every platform + variant
function BuildInstaller {
    try {
        switch -Exact ( $Type ) {
            'x64' {
                WriteStep "Build: MSI installer"
                Push-Location .\win_build\installer_wix
                if( ! $CI ) {
                    msbuild installer.sln /p:Configuration=$Configuration /p:Platform=x64 /p:BoincVersion=$Version
                } else {
                    msbuild installer.sln /p:Configuration=$Configuration /p:Platform=x64 /p:BoincVersion=$Version `
                        /p:WixToolPath=$env:WIX /p:WixTargetsPath=$env:WIX\wix.targets  /p:WixInstallPath=$env:WIX\
                }
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
            }
            'x64_vbox' {
                WriteStep "Build: MSI installer"
                Push-Location .\win_build\installer_wix
                if( ! $CI ) {
                    msbuild installer.sln /p:Configuration=$Configuration /p:Platform=x64 /p:BoincVersion=$Version
                } else {
                    msbuild installer.sln /p:Configuration=$Configuration /p:Platform=x64 /p:BoincVersion=$Version `
                        /p:WixToolPath=$env:WIX /p:WixTargetsPath=$env:WIX\wix.targets  /p:WixInstallPath=$env:WIX\
                }
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
            }
            'arm64' {
                Push-Location .\win_build\installer_wix
                WriteStep "Build: MSI installer"
                if( ! $CI ) {
                    msbuild installer.sln /p:Configuration=$Configuration /p:Platform=arm64 /p:BoincVersion=$Version
                } else {
                    msbuild installer.sln /p:Configuration=$Configuration /p:Platform=arm64 /p:BoincVersion=$Version `
                        /p:InstallerPlatform=arm64 /p:WixToolPath=$env:WIX /p:WixTargetsPath=$env:WIX\wix.targets /p:WixInstallPath=$env:WIX\
                }
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
            }
        }
    }
    Catch {
        Report $false
    }    
}

# Executes the bundle build with the appropriate parameters for every platform + variant
function BuildBundle {
    try {
        switch -Exact ( $Type ) {
            'x64' {
                Push-Location .\win_build\installer_wix
                WriteStep "Build: Bundle only MSI"
                if( ! $CI ) {
                    msbuild bundle.sln /target:bundle /p:Configuration=$Configuration /p:Platform=x86 /p:BoincVersion=$Version
                } else {
                    msbuild bundle.sln /target:bundle /p:Configuration=$Configuration /p:Platform=x86  /p:BoincVersion=$Version `
                        /p:WixToolPath=$env:WIX /p:WixTargetsPath=$env:WIX\wix.targets  /p:WixInstallPath=$env:WIX\
                }
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
            }
            'x64_vbox' {
                Push-Location .\win_build\installer_wix
                WriteStep "Build: Bundle with VirtualBox"
                if( ! $CI ) {
                    msbuild bundle.sln /target:bundle_vbox /p:Configuration=$Configuration /p:Platform=x86 /p:BoincVersion=$Version
                } else {
                    msbuild bundle.sln /target:bundle_vbox /p:Configuration=$Configuration /p:Platform=x86 /p:BoincVersion=$Version `
                        /p:WixToolPath=$env:WIX /p:WixTargetsPath=$env:WIX\wix.targets  /p:WixInstallPath=$env:WIX\
                }
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
            }
            'arm64' {
                Push-Location .\win_build\installer_wix
                WriteStep "Build: Bundle only MSI"
                if( ! $CI ) {
                    msbuild bundle.sln /target:bundle_arm /p:Configuration=$Configuration /p:Platform=arm64 /p:BoincVersion=$Version
                } else {
                    msbuild bundle.sln /target:bundle_arm /p:Configuration=$Configuration /p:Platform=arm64 /p:BoincVersion=$Version `
                        /p:InstallerPlatform=arm64 /p:WixToolPath=$env:WIX /p:WixTargetsPath=$env:WIX\wix.targets /p:WixInstallPath=$env:WIX\
                }
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
            }
        }
    }
    Catch {
        Report $false
    }    
}

# Executes the steps to sign the installer with a pfx certificate 
function SignInstaller {
    $pass = ConvertTo-SecureString -String "$CertificatePass" -Force -AsPlainText

    WriteStep "Import certificate in TrustedPublisher"
    Import-PfxCertificate -FilePath "$Certificate" -Password $pass -Cert Cert:\LocalMachine\TrustedPublisher | Out-Null
    WriteStep "Import certificate as CA Root Authority"
    Import-PfxCertificate -FilePath "$Certificate" -Password $pass -Cert Cert:\LocalMachine\Root | Out-Null

    WriteStep "Sign bundle with certificate"
    $resp = Set-AuthenticodeSignature "build\en-us\installer.msi" -Certificate (Get-PfxCertificate -FilePath "$Certificate" -Password $pass) `
        -TimestampServer "http://timestamp.digicert.com" -HashAlgorithm sha256

    if( !($resp.Status -eq [System.Management.Automation.SignatureStatus]::Valid) ) {
        Report $false "Timestamp signature validation failed"
    }

    Start-Sleep -Seconds 5   # recommended to wait between sign requests
}

# Executes the steps to sign the bundle and the burn engine with a pfx certificate 
function SignBundle {
    $pass = ConvertTo-SecureString -String "$CertificatePass" -Force -AsPlainText  # required by Set-AuthenticodeSignature

    $target = "boinc_bundle.exe"

    WriteStep "Import certificate in TrustedPublisher"
    Import-PfxCertificate -FilePath "$Certificate" -Password $pass -Cert Cert:\LocalMachine\TrustedPublisher | Out-Null
    WriteStep "Import certificate as CA Root Authority"
    Import-PfxCertificate -FilePath "$Certificate" -Password $pass -Cert Cert:\LocalMachine\Root | Out-Null

    # step required by wix to sign the internal 'burn' engine
    insignia -ib build\$target -o build\engine.exe

    $resp = Set-AuthenticodeSignature "build\engine.exe" -Certificate (Get-PfxCertificate -FilePath "$Certificate" -Password $pass) `
        -TimestampServer "http://timestamp.digicert.com" -HashAlgorithm sha256

    Start-Sleep -Seconds 5   # recommended to wait between sign requests

    # reattaches the engine to the bundle
    insignia -ab build\engine.exe build\$target -o build\$target

    # signs the complete bundle
    WriteStep "Sign bundle with certificate"
    $resp = Set-AuthenticodeSignature "build\$target" -Certificate (Get-PfxCertificate -FilePath "$Certificate" -Password $pass) `
        -TimestampServer "http://timestamp.digicert.com" -HashAlgorithm sha256

    if( !($resp.Status -eq [System.Management.Automation.SignatureStatus]::Valid) ) {
        Report $false "Timestamp signature validation failed"
    }
}

# Method that renames the fixed output bundle name to the release name
function RenameToOfficialName {
    try {
        $targetName = ""
        $suffix = ""
        switch -Exact ( $Type ) {
            'x64' {
                $suffix = "x86_64"
            }
            'x64_vbox' {
                $suffix = "x86_64_vbox"
            }
            'arm64' {
                $suffix = "arm64"
            }
            default {
                Report $false "Unknown architecture for rename"
            }
        }
        $targetName = "boinc_{0}_windows_{1}.exe" -f $Version,$suffix   # use only the new name not full path
        Rename-Item -Path "build\boinc_bundle.exe" $targetName
    }
    catch {
        Report $false
    }
}

#############################

function Main {
    Write-Output "[Build Info] arch: $Type version: $Version CI: $CI"

    CheckPath -Path "build" -IsDir

    CleanBuildDir

    if( $CleanOnly ) {
        Report $true
    }

    Header

    WriteStep "Check Prerequisites"
    CheckPrerequisites

    try {
        WriteStep "Check certificate"
        CheckPath -Path "$Certificate"
    }
    Catch {
        Report $false "Could not find pfx certificate at path $Certificate"
    }

    $extn = [IO.Path]::GetExtension($Certificate)
    if ( !($extn -eq ".pfx") )
    {
        Report $false "Certificate does not have supported extension .pfx, please provide a suitable certificate"
    }

    WriteStep "Check Build directory"
    CheckBuildDir

    WriteStep "Copy additional source files"
    CopyAdditionalSourceFiles

    WriteStep "Build installer"
    BuildInstaller

    WriteStep "Sign installer"
    SignInstaller

    WriteStep "Build installer"
    BuildBundle

    WriteStep "Sign bundle"
    SignBundle

    WriteStep "Rename bundle to official name"
    RenameToOfficialName

    Report $true
}

Main
