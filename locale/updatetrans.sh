#!/bin/sh

for file in `find -name 'BOINC-Manager.mo'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  cp -f $file BOINC/locale/${locale}
done  

exit 0
