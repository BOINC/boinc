## Goals

Provide a turn-key VM for Android development

## Requirements

* Vagrant
* VirtualBox

## HOWTO

1. On your host: open a terminal
   1. `cd <BOINC_REPO>/android`
   1. `vagrant up`
   1. Wait until the final reboot finished
   1. `vagrant ssh`
   1. `cd BOINC/android`
   1. `./build_all.sh`
1. In the VM:
   1. Log in with `vagrant/vagrant`
   1. Start Android Studio
   1. No need to change anything in the setup assitant (just complete it)
      * OK / Next / Next / Finish / Finish
   1. Import the BOINC App as *Gradle* project from: `~/BOINC/android/BOINC`
   1. Ignore potential Gradle Plugin warning: Don't remind me again
1. Hook up your Android device via USB (and remember to attach it to VirtualBox)
1. Happy hacking :-)

## Know limitations

* The Android Virtual Device Manager might not work properly as it needs virtualization
  which isn't possible within a virtual machine (at least not using VirtualBox).
