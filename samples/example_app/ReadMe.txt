To build the example_app sample (UpperCase2) for Macintosh:

First build the BOINC libraries:
cd [path]/mac_build
source BuildMacBOINC.sh -lib

For more details, see the instructions in that script's comments or at:
[path]/mac_build/HowToBuildBOINC_XCode.rtf

After building the libraries, there are three ways to build the example_app sample for Macintosh:

[1] Run the MakeMacExample.sh script in this example_app directory (which will invoke the makefile Makefile_mac):
cd [path]/samples/example_app/
sh MakeMacExample.sh

[2] Invoke the stand-alone makefile Makefile_mac2 directly:
cd [path]/samples/example_app/
make -f Makefile_mac2

[3] Run the Xcode project at:
[path]/samples/mac_build/UpperCase2.xcodeproj
