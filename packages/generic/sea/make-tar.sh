#!/bin/sh
destdir=`pwd`
cd $1
srcdir=`pwd`/locale
cd $destdir
rm -rf `find BOINC -name ".CVS" -o -name ".svn"`
mkdir BOINC
mkdir BOINC/locale
for file in `find ${srcdir} -name 'BOINC-Manager.mo'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  mkdir BOINC/locale/${locale}
  cp -f $file BOINC/locale/${locale}
done  
tar cvf sea.tar BOINC
exit 0
