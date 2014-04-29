set echo
setenv TARGET_HOST i686-w64-mingw32
setenv XCOMPILE_ROOT "/usr/${TARGET_HOST}/sys-root/mingw"
set guess=`../config.guess`
setenv BUILD_HOST `../config.sub ${guess}`
setenv OLDPATH ${PATH}
setenv PATH "/usr/${TARGET_HOST}/bin:${XCOMPILE_ROOT}/bin:${PATH}"

setenv CC `which ${TARGET_HOST}-gcc`
setenv CXX `which ${TARGET_HOST}-g++`
setenv AR `which ${TARGET_HOST}-ar`
senenv LD `which ${TARGET_HOST}-ld`
setenv RANLIB `which ${TARGET_HOST}-ranlib`
setenv WINDRES `which ${TARGET_HOST}-windres`
setenv CPPFLAGS "-D_WIN32_WINDOWS=0x0410 -DMINGW_WIN32 -I${XCOMPILE_ROOT}/include"
setenv CFLAGS "${CPPFLAGS}"
setenv CXXFLAGS "-gstabs -g3"
setenv LDFLAGS "-L/usr/${TARGET_HOST}/lib -L${XCOMPILE_ROOT}/lib"
setenv CURL_CONFIG "${XCOMPILE_ROOT}/bin/curl-config"
setenv WX_CONFIG_PATH "${XCOMPILE_ROOT}/bin/wx-config"
set pkgsearchpath="dummy"
foreach dir ( `find /usr/${TARGET_HOST} -name pkgconfig` ) 
  set pkgsearchpath="${pkgsearchpath}:${dir}"
end
foreach dir ( `find /usr/lib -name pkgconfig` ) 
  set pkgsearchpath="${pkgsearchpath}:${dir}"
end
foreach dir ( `find /usr/share -name pkgconfig` ) 
  set pkgsearchpath="${pkgsearchpath}:${dir}"
end
setenv PKG_CONFIG_PATH `echo ${pkgsearchpath} | sed 's/dummy://'`
test -e ../configure && find .. -name configure -mtime -1
if ($status != 0) then
  cd ..
  ./_autosetup
  cd xcompile
endif
test -f ${XCOMPILE_ROOT}/lib/libssl.a
if ($status != 0) then
  set opensslver=1.0.1g
  wget http://www.openssl.org/source/openssl-${opensslver}.tar.gz
  tar zxf openssl-${opensslver}.tar.gz
  /bin/rm openssl-${opensslver}.tar.gz
  cd openssl-${opensslver}
  ./Configure --prefix=${XCOMPILE_ROOT}/ no-shared zlib mingw
  make all
  make install
  cd ..
  rm -rf openssl-${opensslver}
endif
test -f ${XCOMPILE_ROOT}/lib/libcurl.a
if ($status != 0) then
  set curlver=7.36.0
  wget http://curl.haxx.se/download/curl-${curlver}.tar.bz2
  tar jxf curl-${curlver}.tar.bz2
  /bin/rm curl-${curlver}.tar.bz2
  cd curl-${curlver}
  ./configure --prefix=${XCOMPILE_ROOT} --enable-static --disable-shared --host=$TARGET_HOST --build=$BUILD_HOST --with-zlib=${XCOMPILE_ROOT}
  make all
  make install
  cd ..
  rm -rf curl-${curlver}
endif
#test -f ${XCOMPILE_ROOT}/lib/libwxbase30u.a
#if ($status != 0) then
#  set z7=`which 7z`
#  if ( "x$z7" == x ) then
#     echo you must install 7z in /usr in order to install wxwidgets
#     exit 1
#  endif
#  set wxver=3.0.0
#  set gccver=481
#  set filename=wxMSW-${wxver}_gcc${gccver}TDM_Dev.7z
#  wget http://sourceforge.net/projects/wxwindows/files/3.0.0/binaries/$filename/download
#  mv download $filename
#  7z x $filename
#  rm $filename
#  rsync lib/gcc${gccver}TDM_dll/*.a ${XCOMPILE_ROOT}/lib
#  rsync lib/gcc${gccver}TDM_dll/*.dll ${XCOMPILE_ROOT}/bin
#  set filename=wxWidgets-${wxver}_headers.7z
#  wget http://sourceforge.net/projects/wxwindows/files/3.0.0/$filename/download
#  mv download $filename
#  7z x $filename
#  rm $filename
#  rsync -va include/wx ${XCOMPILE_ROOT}/include
#  rsync -va lib/gcc${gccver}TDM_dll/mswu/wx ${XCOMPILE_ROOT}/include
#  rm -rf lib/gcc${gccver}TDM_dll
#  rm -rf include/wx
#endif
test -f ${XCOMPILE_ROOT}/lib/libfreeglut_static.a
if ($status != 0) then
  svn co http://svn.code.sf.net/p/freeglut/code/trunk/freeglut/freeglut freeglut
  cd freeglut
  mkdir build
  cd build
  /usr/bin/cmake -D GNU_HOST=${TARGET_HOST} \
    -D CMAKE_TOOLCHAIN_FILE=mingw_cross_toolchain.cmake \
    -D CMAKE_INSTALL_PREFIX=${XCOMPILE_ROOT}\
    -D FREEGLUT_BUILD_STATIC_LIBS=ON \
    -D FREEGLUT_BUILD_SHARED_LIBS=OFF \
    -D FREEGLUT_BUILD_DEMOS=OFF  \
    ..
  make -j4 
  make install
  cd ../..
  /bin/rm -rf freeglut
endif

../configure -C --host=$TARGET_HOST --build=$BUILD_HOST --enable-static --disable-shared --enable-libraries --enable-client --disable-server --with-libcurl=${XCOMPILE_ROOT} --with-ssl=${XCOMPILE_ROOT} --with-winsock
make all
unset echo
#setenv PATH ${OLDPATH}
