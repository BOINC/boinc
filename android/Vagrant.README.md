## Goals

Provide a turn-key VM for Android development

## Requirements

* [Vagrant](https://www.vagrantup.com/downloads.html)
* [VirtualBox](https://www.virtualbox.org/wiki/Downloads)
* Host:
  * 4 CPU cores (2 used by VM)
  * ~18 GB disk space
  * 4 GB RAM (2 used by VM)
* Download volume (once): ~3.5 GB

## HOWTO

1. On your host: open a terminal
   1. `cd <BOINC_REPO>/android`
   1. `vagrant up`
   1. Wait until the final reboot finished
   1. **From this point on you don't need Vagrant anymore**
      1. Don't run `vagrant up` again!
      1. Just use VirtualBox to stop/start your new shiny VM
1. In the VM:
   1. Log in with `vagrant/vagrant`
   1. Open a terminal
      1. `cd BOINC/android`
      1. `./build_all.sh`
   1. Start Android Studio
   1. No need to change anything in the setup assitant (just complete it)
      * OK / Next / Next / Finish / Finish
   1. Import the BOINC App as *Gradle* project from: `~/BOINC/android/BOINC`
   1. Ignore potential Gradle Plugin warning: *Don't remind me again*
1. Hook up your Android device via USB (and remember to attach it to VirtualBox)
1. Happy hacking :-)

## Known limitations

* The Android Virtual Device Manager might not work properly as it needs virtualization
  which isn't possible within a virtual machine (at least not using VirtualBox).
