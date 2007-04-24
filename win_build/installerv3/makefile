# Berkeley Open Infrastructure for Network Computing
# http://boinc.berkeley.edu
# Copyright (C) 2005 University of California
# 
# This is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation;
# either version 2.1 of the License, or (at your option) any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Lesser General Public License for more details.
# 
# To view the GNU Lesser General Public License visit
# http://www.gnu.org/copyleft/lesser.html
# or write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# BOINC installer makefile

.SUFFIXES : .wxs .wixobj

candle=candle -nologo
light=light -nologo
lit=lit -nologo

platform=Win32
config=Deploy
root=installerv3^\
loc=$(root)Localization^\
out=installerv3^\$(platform)^\$(config)^\
int=$(out)obj^\
source=$(out)SourceDir^\


# Default target
all: $(out)BOINC.msi

#setup.exe: BOINC.msi
#  $

$(out)BOINC.msi: SourceDir $(int)BOINC.wixlib $(loc)en-us.wxl
	$(light) -sice:ICE30 -b $(out) -out $@ $(int)BOINC.wixlib -loc $(loc)en-us.wxl

$(int)BOINC.wixlib: $(int)BOINC.wixobj $(int)Strings.wixobj $(int)Branding.wixobj
	$(lit) -out $@ $**

{$(root)}.wxs.wixobj:
	$(candle) -out $(int)%|fF.wixobj %s


# Installation files
# ...
SourceDir: Folders $(source)COPYING.txt $(source)COPYRIGHT.txt $(source)LICENSE.rtf
# Shared binary streams						
	-copy /y $(out)..^\..^\Shared^\$(config)^\Binary^\*.* $(out)Binary
# Platform specific files			
	-copy /y $(out)Redist^\*.* $(out)SourceDir
# Shared files					
	-copy /y $(out)..^\..^\Shared^\$(config)^\Redist^\*.* $(out)SourceDir
	
Folders:
	-mkdir $(out)Binary
	-mkdir $(out)SourceDir

# General files
$(source)COPYING.txt: ..^\COPYING
	copy /y ..^\COPYING $(source)COPYING.txt

$(source)COPYRIGHT.txt: ..^\COPYRIGHT
	copy /y ..^\COPYRIGHT $(source)COPYRIGHT.txt

$(source)LICENSE.rtf: ..^\LICENSE.rtf
	copy /y ..^\LICENSE.rtf $(source)LICENSE.rtf


# Clean target
clean:
	del /f /q $(out)
	-rmdir /s /q $(out)SourceDir
	del /f /q $(int)
	-rmdir /s /q $(int)
