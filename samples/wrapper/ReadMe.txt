To build wrapper for Macintosh:

First build the BOINC libraries:
cd [path]/mac_build
source BuildMacBOINC.sh -lib

For more details, see the instructions in that script's comments or at:
[path]/mac_build/HowToBuildBOINC_XCode.rtf

After building the libraries, run the BuildMacWrapper.sh script in this wrapper directory (which will invoke the makefile Makefile_mac):
cd [path]/samples/wrapper/
sh BuildMacWrapper.sh
