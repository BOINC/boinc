param
(
    [Parameter(ParameterSetName='build')][ValidateSet('x64','x64_vbox','arm')][string]$Type = "x64",
    [Parameter(Mandatory=$true,ParameterSetName='build')][ValidateNotNullOrEmpty()][string]$Certificate,
    [Parameter(Mandatory=$true,ParameterSetName='build')][ValidateNotNullOrEmpty()][string]$CertificatePass,

    [Parameter(Mandatory=$true,ParameterSetName='clean')][switch]$CleanOnly
)

$global:step = 0
$ErrorActionPreference = 'Stop'

$VboxInstaller = 'VirtualBox-7.0.10-158379-Win.exe'
$VC2010RedistInstaller = 'vcredist_x64.exe'
$Configuration = 'Debug'

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
    WriteStep "Cleanup"
    try { Remove-Item -Recurse -Path build\src* } # Previous bundles sources
    Catch {
        # ignore
    }

    try { Remove-Item -Path build\*bundle.exe } # Previous bundles
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
        if ( $Type -eq "arm" ) {
            CheckPath -Path "build\prerequisites\$VC2010RedistInstaller" -ExpectNotPresent
        } else {
            CheckPath -Path "build\prerequisites\$VC2010RedistInstaller"
        }

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
        switch -Exact ( $Type ) {
            'x64' {
                WriteStep "Build: MSI installer"
                Push-Location .\win_build\installer_wix
                msbuild installer.sln /p:Configuration=$Configuration /p:Platform=x64
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
    
                Push-Location .\win_build\installer_wix
                WriteStep "Build: Bundle only MSI"
                msbuild bundle.sln /target:bundle /p:Configuration=$Configuration /p:Platform=x64
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
            }
            'x64_vbox' {
                WriteStep "Build: MSI installer"
                Push-Location .\win_build\installer_wix
                msbuild installer.sln /p:Configuration=$Configuration /p:Platform=x64
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
    
                Push-Location .\win_build\installer_wix
                WriteStep "Build: Bundle with VirtualBox"
                msbuild bundle.sln /target:bundle_vbox /p:Configuration=$Configuration /p:Platform=x64
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
            }
            'arm' {
                WriteStep "Build: MSI installer"
                Push-Location .\win_build\installer_wix
                msbuild installer.sln /p:Configuration=$Configuration /p:Platform=ARM64
                Pop-Location
                if( !($LastExitCode -eq 0) ) {
                    Report $false
                }
    
                Push-Location .\win_build\installer_wix
                WriteStep "Build: Bundle only MSI"
                msbuild bundle.sln /target:bundle_arm /p:Configuration=$Configuration /p:Platform=ARM64
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

function SignInstaller {
    $pass = ConvertTo-SecureString -String "$CertificatePass" -Force -AsPlainText

    $target = "boinc_bundle.exe"
    if( $Type -eq 'x64_vbox' ) {
        $target = "boinc_vbox_bundle.exe"
    }

    # for testing purposes
    # New-SelfSignedCertificate -DnsName "BOINC@berkeley.edu" -Type Codesigning -CertStoreLocation cert:\CurrentUser\My
    # Export-PfxCertificate -Cert (Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert)[0] -Password $pass -FilePath "$Certificate"

    WriteStep "Import certificate in TrustedPublisher"
    Import-PfxCertificate -FilePath "$Certificate" -Password $pass -Cert Cert:\CurrentUser\TrustedPublisher | Out-Null
    WriteStep "Import certificate as CA Root Authority"
    Import-PfxCertificate -FilePath "$Certificate" -Password $pass -Cert Cert:\CurrentUser\Root | Out-Null

    WriteStep "Sign bundle with certificate"
    $resp = Set-AuthenticodeSignature "build\$target" -Certificate (Get-PfxCertificate -FilePath "$Certificate" -Password $pass) `
        -TimestampServer "http://timestamp.digicert.com" -Force

    if( !($resp.Status -eq [System.Management.Automation.SignatureStatus]::Valid) ) {
        Report $false "Timestamp signature validation failed"
    }
}

#############################

function Main {
    Write-Output "arch: $Type"

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

    WriteStep "Build installers"
    BuildInstaller

    WriteStep "Sign installers"
    SignInstaller

    Report $true
}

Main
