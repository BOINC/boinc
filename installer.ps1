param
(
    [Parameter(ParameterSetName='build')][ValidateSet('x64','x64_vbox','arm')][string]$Arch = "x64",
    [Parameter(Mandatory=$true,ParameterSetName='build')][ValidateNotNullOrEmpty()][string]$Certificate,
    [Parameter(Mandatory=$true,ParameterSetName='build')][ValidateNotNullOrEmpty()][string]$CertificatePass,

    [Parameter(Mandatory=$true,ParameterSetName='clean')][switch]$Clean
)

$global:step = 0
$ErrorActionPreference = 'Stop'

$VboxInstaller = 'VirtualBox-7.0.10-158379-Win.exe'
$VC2010RedistInstaller = 'vcredist_x64.exe'

function WriteStep {
    param($msg)
    $global:step++
    Write-Output "[$global:step][$msg]"
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

function CheckPrerequisites {
    WriteStep "Requirements check: Powershell >= 7"
    if ( $PSVersionTable.PSVersion.Major -lt 7 ) {
        Write-Error "Found powershell version $PSVersionTable.PSVersion, required >= 7"
        Report $false
    }

    WriteStep "Requirements check: MSBuild"
    try {
        msbuild --version | Out-Null
    }
    Catch {
        Report $false
    }

    WriteStep "Requirements check: Wix Toolkit"
    try {
        heat -? | Out-Null
    }
    Catch {
        Report $false
    }
}

function CleanBuildDir {
    try {
        WriteStep "Cleanup"
        Remove-Item -Recurse -Path build | Out-Null

        WriteStep "Preparing build dir"
        New-Item -Path build -ItemType "directory" | Out-Null
    }
    Catch {
        Report $false
    }
}

function CheckPath {
    param
    (
        [string]$Path,
        [Parameter(Mandatory=$false)][switch]$IsDir,
        [Parameter(Mandatory=$false)][switch]$ExpectNotPresent
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

function CheckBuildDir {
    try {
        WriteStep "Check directories"
        CheckPath -Path "build" -IsDir
        CheckPath -Path "build\prerequisites" -IsDir
        CheckPath -Path "build\locale" -IsDir
        CheckPath -Path "build\res" -IsDir
        CheckPath -Path "build\Skins" -IsDir

        WriteStep "Check binary files"
        CheckPath -Path "build\boinc.exe"
        CheckPath -Path "build\boinc.scr"
        CheckPath -Path "build\boinccmd.exe"
        CheckPath -Path "build\boincsvcctrl.exe"
        CheckPath -Path "build\ca-bundle.crt"
        CheckPath -Path "build\boincmgr.exe"
        CheckPath -Path "build\boinctray.exe"

        WriteStep "Check prerequisites"
        CheckPath -Path "build\prerequisites\$VC2010RedistInstaller"

        if ( !($Arch -eq "x64_vbox") ) {
            CheckPath -Path "build\prerequisites\$VboxInstaller"
        } else {
            CheckPath -Path "build\prerequisites\$VboxInstaller" -ExpectNotPresent
        }
    }
    Catch {
        Report $false
    }
}

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

function BuildInstaller {
    try {
        switch -Exact ( $Arch ) {
            'x64' {
                WriteStep "Build: MSI installer"
                Push-Location .\win_build\installer_wix
                msbuild installer.sln | Out-Null
                Pop-Location
    
                Push-Location .\win_build\installer_wix
                WriteStep "Build: Bundle only MSI"
                msbuild bundle.sln | Out-Null
                Pop-Location
            }
            'x64_vbox' {
                WriteStep "Build: MSI installer"
                Push-Location .\win_build\installer_wix
                msbuild installer.sln | Out-Null
                Pop-Location
    
                Push-Location .\win_build\installer_wix
                WriteStep "Build: Bundle with VirtualBox"
                msbuild bundle.sln | Out-Null
                Pop-Location
            }
            'arm' {
                WriteStep "Build: MSI installer"
                Push-Location .\win_build\installer_wix
                msbuild installer.sln | Out-Null
                Pop-Location
    
                Push-Location .\win_build\installer_wix
                WriteStep "Build: Bundle only MSI"
                msbuild bundle.sln | Out-Null
                Pop-Location
            }
        }
    }
    Catch {
        Report $false
    }    
}

function SignInstaller {
    $pass = ConvertTo-SecureString -String "$CertificatePass" -Force -AsPlainText

    # for testing purposes
    # New-SelfSignedCertificate -DnsName "BOINC@berkeley.edu" -Type Codesigning -CertStoreLocation cert:\CurrentUser\My
    # Export-PfxCertificate -Cert (Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert)[0] -Password $pass -FilePath "$Certificate"

    WriteStep "Import certificate in TrustedPublisher"
    Import-PfxCertificate -FilePath "$Certificate" -Password $pass -Cert Cert:\CurrentUser\TrustedPublisher | Out-Null
    WriteStep "Import certificate as CA Root Authority"
    Import-PfxCertificate -FilePath "$Certificate" -Password $pass -Cert Cert:\CurrentUser\Root | Out-Null

    WriteStep "Sign bundle with certificate"
    $resp = Set-AuthenticodeSignature "build\boinc_bundle.exe" -Certificate (Get-PfxCertificate -FilePath "$Certificate" -Password $pass) `
        -TimestampServer "http://timestamp.digicert.com" -Force

    if( !($resp.Status -eq [System.Management.Automation.SignatureStatus]::Valid) ) {
        Report $false "Timestamp signature validation failed"
    }
}

#############################

Write-Output "arch: $Arch"

if( $Clean ) {
    CleanBuildDir
    Report $true
}

Header

WriteStep "Check Prerequisites"
CheckPrerequisites

try {
    WriteStep "Check certificate"
    CheckPath -Path "$Certificate"

    $extn = [IO.Path]::GetExtension($Certificate)
    if ( !($extn -eq ".pfx") )
    {
        Report $false "Certificate does not have supported extension .pfx, please provide a suitable certificate"
    }
}
Catch {
    Report $false "Could not find pfx certificate at path $Certificate"
}

WriteStep "Check Build directory"
CheckBuildDir

WriteStep "Copy additional source files"
CopyAdditionalSourceFiles

WriteStep "Build installers"
BuildInstaller

WriteStep "Sign installers"
SignInstaller

Report $true
