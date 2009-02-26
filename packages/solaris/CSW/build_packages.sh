#! /bin/bash
/bin/rm -f */*.pkg.gz */*.pkgb
PKGDIR=`pwd`
cd ../../..
# we are now in the build dir...
TOP=`pwd`
MAKE=gmake
export MAKE
LD_LIBRARY_PATH=
export LD_LIBRARY_PATH
$MAKE -j 3 all
fakeroot $MAKE -j 3 stage
cd stage
rm *
strip opt/csw/bin/*
fakeroot pkgproto . | sed 's/korpela csw/root bin/' > prototype
for dir in boinclibs boincdevel boincclient boincmanager ; do
  echo $PKGDIR/${dir}
  cd $PKGDIR/${dir}
  $MAKE all
  grep -v NAME pkginfo | grep -v VENDOR  > /tmp/$$
  . /tmp/$$
  pkgmk -r $TOP/stage -a `uname -p` BASEDIR=/
  filename=${dir}-${VERSION}-SunOS`uname -r`-`uname -p`-CSW.pkg
  pkgtrans -s /var/spool/pkg $PWD/$filename $PKG
  rm -r /var/spool/pkg/$PKG
  gzip $PWD/$filename
  checkpkg $filename.gz
done
