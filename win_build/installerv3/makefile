# BOINC installer makefile

.SUFFIXES : .wxs .wixobj

candle=candle -nologo
light=light -nologo
lit=lit -nologo
source=installerv3^\
loc=$(source)Localization^\
out=installerv3^\Debug^\


# Default target
all: $(out)BOINC.msi

#setup.exe: BOINC.msi
#  $(link) $(ldebug) $(conflags) -out:simple.exe simple.obj $(conlibs) lsapi32.lib

$(out)BOINC.msi: $(out)BOINC.wixlib $(loc)en-us.wxl
  $(light) -sice:ICE30 -b $(source) -out $@ $(out)BOINC.wixlib -loc $(loc)en-us.wxl

$(out)BOINC.wixlib: $(out)BOINC.wixobj $(out)Strings.wixobj $(out)Branding.wixobj
  $(lit) -out $@ $**

{$(source)}.wxs.wixobj:
  $(candle) -out $(out)%|fF.wixobj %s


# Clean target
clean:
	del /f /q $(out)