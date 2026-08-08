# Building BOINC Android client on Windows (WSL)

## Goal

Allow Windows users to use built-in WSL for setting up for BOINC Android development

## Known Issues

* WSL1: Slow I/O performance - [Microsoft/WSL#873](https://github.com/Microsoft/WSL/issues/873)
* WSL1: fork/exec error - [Microsoft/WSL#2469](https://github.com/microsoft/WSL/issues/2469)
* WSL1: wget HSTS error - <https://superuser.com/questions/1539972/wget-error-bash-windows-subsystem-for-linux>
* WSL2: High vmmem memory usage - [Microsoft/WSL#4166](https://github.com/microsoft/WSL/issues/4166) - Note: Users can check out [this workaround](https://github.com/microsoft/WSL/issues/4166#issuecomment-526725261). Based on our testing, setting `memory=3GB` should be sufficient if users don't have more than that.
* WSL2: Unable to use hardware acceleration in Android Emulator or VirtualBox - [WSL2 FAQ](https://docs.microsoft.com/en-us/windows/wsl/wsl2-faq#will-i-be-able-to-run-wsl-2-and-other-3rd-party-virtualization-tools-such-as-vmware-or-virtualbox)

## Prerequisites

* Windows 10 x86_64 version 1607 or later (for WSL1)
* Windows 10 x86_64 version 2004 or later (for WSL2)
* [Android Studio (Windows)](https://developer.android.com/studio#downloads)
* `git` client (for Android Studio)
* [Windows Subsystem for Linux (WSL)](https://docs.microsoft.com/en-us/windows/wsl/install-win10) # IMPORTANT #
* [Ubuntu from Microsoft Store](https://wiki.ubuntu.com/WSL#Installing_Ubuntu_on_WSL_via_the_Microsoft_Store_.28Recommended.29)

## Set up Ubuntu

We are going to use Ubuntu 20.04 as an example, other versions should be similar. Refer [Prerequisites](#Prerequisites) before proceeding.

1. Update and install build tools

        $ sudo apt update && sudo apt upgrade

        $ sudo apt install git unzip make m4 pkg-config autoconf automake libtool python

1. Clone BOINC repo

        $ git clone https://github.com/boinc/boinc.git ~/boinc

1. Build BOINC Android client

        $ cd ~/boinc/android

        $ ./build_client.sh

    You should see this once you are done compiling:

        ===== BOINC for all platforms build done =====

    If you ever had problem at this point, try `git clean -fxd ..` and run again

    While waiting please do another repository cloning at Android Studio.

## Android Studio

Open Android Studio and choose `Get from Version Control`

Set URL as `https://github.com/boinc/boinc` and clone it. When asked about creating Android Studio project, select No.

Once the native client build is complete, copy the native client binaries to cloned repo at Android Studio side. Replace `<pathtofolder>` with appropriate location.

        $ cp -frv ~/boinc/android/BOINC/app/src/main/assets/* /mnt/<drive>/<pathtofolder>/boinc/android/BOINC/app/src/main/assets/

Then import project from `<drive>:\<pathtofolder>\boinc\android\BOINC`

After Gradle sync completed, you can now start development or compile the APK.

## Workarounds

While WSL1 makes building the native client easily, it is prone to errors and speed issues. You may want to check out workarounds enlisted in the [Known Issues](#known-issues) section. Note that [Microsoft does not recommend this](https://github.com/Microsoft/WSL/issues/873#issuecomment-463442051) and users are expected do it at their own discretion. You can read more about WSL1 and its issues at [Microsoft/WSL#873 (comment)](https://github.com/Microsoft/WSL/issues/873#issuecomment-425272829). This should be fixed with the release of WSL2.

## Tips

1. Speed up compiling by adding:

        $ MAKEFLAGS="-j$(nproc --all)" ./build_all.sh

1. You can access the current directory via File Explorer by running:

        $ explorer.exe .

1. If you use Visual Studio Code, you can open up the current directory by running:

        $ code .

1. If you need to reset or uninstall Ubuntu, simply:

        Start Menu > right click Ubuntu > More > App Settings
