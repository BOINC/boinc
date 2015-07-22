To build VBoxWrapper for Macintosh:

First build the BOINC libraries:
cd [path]/mac_build
source BuildMacBOINC.sh -lib

For more details, see the instructions in that script's comments or at:
[path]/mac_build/HowToBuildBOINC_XCode.rtf

After building the libraries, there are two ways to build VBoxWrapper for Macintosh:

[1] Run the Xcode project at:
[path]/samples/vboxwrapper/vboxwrapper.xcodeproj

[2] Run the BuildMacVboxWrapper.sh script in this vboxwrapper directory (which will invoke the makefile Makefile_mac):
cd [path]/samples/vboxwrapper/
sh BuildMacVboxWrapper.sh
