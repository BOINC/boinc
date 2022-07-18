## Goals

Provide a turn-key VM for Android development

## Requirements

### On Linux, MacOS or Windows with VirtualBox
* [Vagrant](https://www.vagrantup.com/downloads.html)
* [VirtualBox](https://www.virtualbox.org/wiki/Downloads)
* [VirtualBox Extension Pack](https://www.virtualbox.org/wiki/Downloads) for USB 3.0 support.
* Host:
  * 4 CPU cores (2 used by VM)
  * ~18 GB disk space
  * 4 GB RAM (2 used by VM)
* Download volume (once): ~3.5 GB

### On Windows with Hyper-V
* [Hyper-V](https://docs.microsoft.com/en-us/virtualization/hyper-v-on-windows/quick-start/enable-hyper-v)
* Host:
  * 4 CPU cores (2 used by VM)
  * ~18 GB disk space
  * 8 GB RAM (4 used by VM)
* Download volume (once): ~3.5 GB

## HOWTO

1. On your host: open a terminal
   1. Clone the [BOINC repo](https://github.com/BOINC/boinc) and `cd <BOINC_REPO>/android` or just download the [Vagrantfile from GitHub](https://github.com/BOINC/boinc/blob/master/android/Vagrantfile)
   1. `vagrant up`
   1. Wait until the final reboot finished
   1. **From this point on you don't need Vagrant anymore**
      1. Don't run `vagrant up` again!
      1. Just use VirtualBox/Hyper-V to stop/start your new shiny VM
1. In the VM:
   1. Log in with `vagrant/vagrant`
   1. Open a terminal
      1. `cd ~/BOINC/android`
      1. `./build_client.sh`
   1. Start Android Studio
   1. No need to change anything in the setup assistant (just complete it)
      * OK / Next / Next / Finish / Finish
   1. Import the BOINC App as *Gradle* project from: `~/BOINC/android/BOINC`
   1. Ignore potential Gradle Plugin warning: *Don't remind me again*
1. Hook up your Android device via USB (and remember to attach it to VirtualBox)
1. Happy hacking :-)

### Android Virtual Device Manager
Hyper-V can run Android Virtual Device Manager, but you have to install KVM and add vagrant user to KVM Group.
1. Open a terminal
   1. `apt install qemu-kvm`
   1. `adduser vagrant kvm`
1. Reboot the VM.

### Known limitations

* The Android Virtual Device Manager might not work properly as it needs virtualization
  which isn't possible within a virtual machine (at least not using VirtualBox). Although Hyper-V can run Android Virtual Device Manager.
* On Windows it seems the VirtualBox manage GPU acceleration a little better on Ubuntu 18.04, than Hyper-V, despite the fact that Windows added [Enhanced Session Mode to Ubuntu 18.04](https://blogs.technet.microsoft.com/virtualization/2018/02/28/sneak-peek-taking-a-spin-with-enhanced-linux-vms/).

## Push to GitHub without password using ssh-key
If you want to do a `push` without username and password prompt, you have to do the following:
1. Open terminal.
1. Go to the BOINC repository: `cd ~/BOINC`
1. Switch the remote URL from HTTPS to SSH: `git remote set-url origin git@github.com:BOINC/boinc.git`
1. Verify that the remote URL has changed: `git remote -v`
1. Generate a new SSH key: `ssh-keygen -t rsa -b 4096 -C "your_email@example.com"`
   1. When you're prompted to "Enter a file in which to save the key," press Enter. This accepts the default file location.
   1. When you're prompted to "Enter passphrase (empty for no passphrase)" type a secure passphrase.
1. Add your SSH key to the ssh-agent.
   1. Start the ssh-agent in the background: `eval "$(ssh-agent -s)"`
   1. Add your SSH private key to the ssh-agent: `ssh-add ~/.ssh/id_rsa`
1. Add the new SSH key to your GitHub account
   1. Copy the SSH key to your clipboard: `cat ~/.ssh/id_rsa.pub`
   1. In the upper-right corner of GitHub, click your profile photo, then click **Settings**.
   1. In the user settings sidebar, click **SSH and GPG keys**.
   1. Click **New SSH key**.
   1. In the **Title** field, add a descriptive label for the new key. For example "BOINC Android Development".
   1. Paste your key into the **Key** field.

For more information check out the [Connecting to GitHub with SSH](https://help.github.com/en/articles/connecting-to-github-with-ssh) article.
