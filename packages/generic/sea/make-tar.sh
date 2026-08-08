#!/bin/sh
destdir=`pwd`
destskindir=`pwd`/BOINC/skins/Default
cd $1
srcdir=`pwd`/locale
srcskindir=`pwd`/clientgui/skins/Default
cd $destdir
mkdir BOINC
mkdir BOINC/locale
mkdir BOINC/skins
mkdir BOINC/skins/Default
cp -R ${srcskindir}/* ${destskindir}
rm -rf `find BOINC -name ".CVS" -o -name ".svn"`
for file in `find ${srcdir} -name 'BOINC-Manager.mo'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  mkdir BOINC/locale/${locale}
  cp -f $file BOINC/locale/${locale}
done
for file in `find ${srcdir} -name 'BOINC-Client.mo'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  mkdir BOINC/locale/${locale}
  cp -f $file BOINC/locale/${locale}
done
tar cvf sea.tar BOINC
exit 0
