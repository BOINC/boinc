#!/bin/sh

rm -fr BOINC_debug/.CVS
mkdir BOINC_debug
mkdir BOINC_debug/locale
find ../locale/client -name 'BOINC-Manager.mo' | cut -d '/' -f 4 | awk '{print "BOINC_debug/locale/"$0}' | xargs mkdir -p 
find ../locale/client -name 'BOINC-Manager.mo' | cut -d '/' -f 4,5 | awk '{print "cp \"../locale/"$0"\" \"BOINC_debug/locale/"$0"\""}' | sh
tar cvf sea_debug.tar BOINC_debug

