# Building BOINC Android client on Windows (WSL)

## Goal

Allow Windows users to use built-in WSL for setting up for BOINC Android development

## Limitations

* Windows 10 only
* Windows Defender may affect compile times (see [Tips#1](#Tips))
* fork/exec error on WSL (see [Tips#1](#Tips))

## Prerequisites

* Windows 10
* [Android Studio (Windows)](https://developer.android.com/studio#downloads)
* [Windows Subsystem for Linux (WSL)](https://docs.microsoft.com/en-us/windows/wsl/install-win10) # IMPORTANT #
* [Ubuntu from Microsoft Store](https://wiki.ubuntu.com/WSL#Installing_Ubuntu_on_WSL_via_the_Microsoft_Store_.28Recommended.29)

## Set up Ubuntu

We are going to use Ubuntu 18.04 as an example, other versions should be similar. Refer [Prerequisites](#Prerequisites) before proceeding.

1. Update and install build tools

        $ sudo apt update && sudo apt upgrade

        $ sudo apt install git unzip make m4 pkg-config autoconf automake libtool python

1. Clone BOINC repo

        $ git clone https://github.com/boinc/boinc.git ~/boinc

1. Build BOINC Android client

        $ cd ~/boinc/android

        $ ./build_all.sh

    You should see this once you are done compiling:

        ===== BOINC for all platforms build done =====

    If you ever had problem at this point, try `git clean -fxd ..` and run again

1. Copy the main BOINC repo to Android Studio readable path. Replace `<pathtofolder>` with appropriate location.

        $ cp -fr ~/boinc /mnt/c/<pathtofolder>/boinc

## Android Studio

Import project from `C:\<pathtofolder>\boinc\android\BOINC`

After Gradle sync completed, you can now start development or compile the APK.

## Tips

1. For compatibility and speed reasons, you may want to temporarily disable Windows Defender Real Time Protection.

        Start Menu > search Virus & Threat Protection > Manage settings > toggle Real-time protection to Off

1. Speed up compiling by adding:

        $ MAKEFLAGS="-j$(nproc --all)" ./build_all.sh

1. You can access the current directory via File Explorer by running:

        $ explorer.exe .

1. If you use Visual Studio Code, you can open up the current directory by running:

        $ code .

1. If you need to reset or uninstall Ubuntu, simply:

        Start Menu > right click Ubuntu > More > App Settings
