#!/bin/sh

# Automate the compilation of the various locale PO files by automatically
# generating them at night.
projname=boinctrunk
projdir=$HOME/pootle/po/$projname

echo $projname
echo $projdir

exit 0

for file in `find -name 'BOINC-Manager.mo'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  
  cp -f $file BOINC/locale/${locale}

  # http://boinc.berkeley.edu/translate/ar/boinctrunk/BOINC-Manager.mo

done  

exit 0
