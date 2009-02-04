#!/bin/sh

rm -fr BOINC/.svn
mkdir BOINC
mkdir BOINC/locale
find ../../../locale -name 'BOINC-Manager.mo' | cut -d '/' -f 5 | awk '{print "BOINC/locale/"$0}' | xargs mkdir -p 
find ../../../locale -name 'BOINC-Manager.mo' | cut -d '/' -f 5,6 | awk '{print "cp \"../../../locale/"$0"\" \"BOINC/locale/"$0"\""}' | sh
tar cvf sea.tar BOINC

