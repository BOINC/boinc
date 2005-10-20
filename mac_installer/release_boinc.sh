#!/bin/csh

##
# Release Script for Macintosh BOINC Manager 10/20/05 by Charlie Fenton
##

## Usage:
## cd to the root directory of the boinc tree, for example:
##     cd [path]/boinc
##
## Invoke this script with the three parts of version number as arguments.  
## For example, if the version is 3.2.1:
##     source [path_to_this_script] 3 2 1
##
## This will create a director "BOINC_Installer" in the parent directory of 
## the current directory


sudo rm -dfR ../BOINC_Installer/Installer\ Resources/
sudo rm -dfR ../BOINC_Installer/Pkg_Root

mkdir -p ../BOINC_Installer/Installer\ Resources/

cp -fp mac_Installer/License.rtf ../BOINC_Installer/Installer\ Resources/
cp -fp mac_installer/ReadMe.rtf ../BOINC_Installer/Installer\ Resources/
cp -fp mac_installer/postinstall ../BOINC_Installer/Installer\ Resources/
cp -fp mac_installer/postupgrade ../BOINC_Installer/Installer\ Resources/

cp -fpR mac_build/build/Postinstall.app ../BOINC_Installer/Installer\ Resources/

mkdir -p ../BOINC_Installer/Pkg_Root
mkdir -p ../BOINC_Installer/Pkg_Root/Applications
mkdir -p ../BOINC_Installer/Pkg_Root/Library
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data
mkdir -p ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale

cp -fpR mac_build/build/BOINCManager.app ../BOINC_Installer/Pkg_Root/Applications/

cp -fpR mac_build/build/BOINCSaver.saver ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers/

## Copy the localization files into the installer tree
## Old way copies CVS and *.po files which are not needed
## cp -fpR locale/client/ ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale
## sudo rm -dfR ../BOINC_Installer/Pkg_Root/Library/Application\ Support/BOINC\ Data/locale/CVS
## New way copies only *.mo files (adapted from boinc/sea/make-tar.sh)
find locale/client -name '*.mo' | cut -d '/' -f 3 | awk '{print "-p \"../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/locale/"$0"\""}' | xargs mkdir
find locale/client -name '*.mo' | cut -d '/' -f 3,4 | awk '{print "cp \"locale/client/"$0"\" \"../BOINC_Installer/Pkg_Root/Library/Application Support/BOINC Data/locale/"$0"\""}' | bash

sudo chown -R root:admin ../BOINC_Installer/Pkg_Root/*
sudo chmod -R 775 ../BOINC_Installer/Pkg_Root/*

sudo chown -R 501:wheel ../BOINC_Installer/Pkg_Root/Applications/*
sudo chmod -R 755 ../BOINC_Installer/Pkg_Root/Applications/*

sudo chown -R 501:wheel ../BOINC_Installer/Pkg_Root/Library/Application\ Support/*
sudo chmod -R 755 ../BOINC_Installer/Pkg_Root/Library/Application\ Support/*

sudo chown -R 501:wheel ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers/*
sudo chmod -R 755  ../BOINC_Installer/Pkg_Root/Library/Screen\ Savers/*

sudo chown -R root:admin ../BOINC_Installer/Installer\ Resources/*
sudo chmod -R 755 ../BOINC_Installer/Installer\ Resources/*

sudo rm -dfR ../BOINC_Installer/New_Release_$1_$2_$3/

mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_powerpc-apple-darwin
mkdir -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_SymbolTables

cp -fp mac_installer/ReadMe.rtf ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX
sudo chown -R 501:wheel ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX/ReadMe.rtf
sudo chmod -R 755 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX/ReadMe.rtf

cp -fpR mac_build/build/boinc ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_powerpc-apple-darwin/
cp -fpR mac_build/build/boinc_cmd ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_powerpc-apple-darwin/
sudo chown -R root:admin ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_powerpc-apple-darwin/*
sudo chmod -R 755 ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_powerpc-apple-darwin/*

cp -fpR mac_build/build/SymbolTables ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX_SymbolTables/

/Developer/Tools/packagemaker -build -p ../BOINC_Installer/New_Release_$1_$2_$3/boinc_$1.$2.$3_macOSX/BOINC.pkg -f ../BOINC_Installer/Pkg_Root -r ../BOINC_Installer/Installer\ Resources/ -i mac_build/Pkg-Info.plist -d mac_Installer/Description.plist -ds 

cd ../BOINC_Installer/New_Release_$1_$2_$3
zip -rq boinc_$1.$2.$3_macOSX.zip boinc_$1.$2.$3_macOSX
zip -rq boinc_$1.$2.$3_powerpc-apple-darwin.zip boinc_$1.$2.$3_powerpc-apple-darwin
zip -rq boinc_$1.$2.$3_macOSX_SymbolTables.zip boinc_$1.$2.$3_macOSX_SymbolTables

