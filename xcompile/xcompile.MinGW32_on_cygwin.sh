#! /bin/sh
case $1 in
     --help) echo Usage:
              echo "$0 \<target_host_triplet\>  (default i686-\*-mingw32)"
	      echo "  Expects to find build root in /usr/\<target_host_triplet\>/sys-root/mingw"
	      ;;
      i686*mingw32|x86_64*mingw32)
              export TARGET_HOST=$1
              ;;
esac
echo -n Checking for cross compiling environment ...
for target_host in ${TARGET_HOST} i686-w64-mingw32 i686-pc-mingw32 x86_64-w64-mingw32 x86_64-pc-mingw32 none ; do
   if test ${target_host} = none ; then
     echo ${target_host}
     echo Cross compiling environment not found in /usr
     exit 1
   fi
   if test -d /usr/${target_host}; then
     export TARGET_HOST=${target_host}
     echo /usr/${target_host}
     break
   fi
done

case $TARGET_HOST in
	i686*)   openssl_cross=mingw
	         ;;
	x86_64*) openssl_cross=mingw64
	         ;;
esac

build_manager=no
build_client=yes
build_libs=yes
build_server=no

export XCOMPILE_ROOT="/usr/${TARGET_HOST}/sys-root/mingw"
guess=`../config.guess`
export BUILD_HOST=`../config.sub ${guess}`
export PATH="/usr/${TARGET_HOST}/bin:${XCOMPILE_ROOT}/bin:${PATH}"

export CC=`which ${TARGET_HOST}-gcc`
export CXX=`which ${TARGET_HOST}-g++`
export AR=`which ${TARGET_HOST}-ar`
export LD=`which ${TARGET_HOST}-ld`
export RANLIB=`which ${TARGET_HOST}-ranlib`
export WINDRES=`which ${TARGET_HOST}-windres`
export CPPFLAGS="-D_WIN32_WINDOWS=0x0410 -DMINGW_WIN32 -I${XCOMPILE_ROOT}/include"
export CFLAGS="${CPPFLAGS}"
export CXXFLAGS="-gstabs -g3 -fpermissive"
export LDFLAGS="-L/usr/${TARGET_HOST}/lib -L${XCOMPILE_ROOT}/lib"
export CURL_CONFIG="${XCOMPILE_ROOT}/bin/curl-config"
export WX_CONFIG_PATH="${XCOMPILE_ROOT}/bin/wx-config"
pkgsearchpath="dummy"
for dir in `find /usr/${TARGET_HOST} -name pkgconfig` ; do
  pkgsearchpath="${pkgsearchpath}:${dir}"
done
for dir in `find /usr/lib -name pkgconfig` ; do
  pkgsearchpath="${pkgsearchpath}:${dir}"
done
for dir in `find /usr/share -name pkgconfig` ; do
  pkgsearchpath="${pkgsearchpath}:${dir}"
done
export PKG_CONFIG_PATH=`echo ${pkgsearchpath} | sed 's/dummy://'`

if ! ( test -e ../configure && find .. -name configure -mtime -1 ) ; then
  cd ..
  ./_autosetup
  cd xcompile
fi

if test $build_manager != no -o $build_libs != no ; then
  if ! test -f ${XCOMPILE_ROOT}/lib/libjpeg.a ; then
    jpegver=9a
    wget http://ijg.org/files/jpegsrc.v${jpegver}.tar.gz
    tar zxf jpegsrc.v${jpegver}.tar.gz
    /bin/rm jpegsrc.v${jpegver}.tar.gz
    cd jpeg-${jpegver}
    ./configure --prefix=${XCOMPILE_ROOT}/ --disable-shared --host=$TARGET_HOST --build=$BUILD_HOST
    make -j 4 all
    make install
    cd ..
    rm -rf jpeg-${jpegver}
  fi

#  if ! test -f ${XCOMPILE_ROOT}/lib/libbmp.a ; then
#    svn checkout svn://svn.code.sf.net/p/libbmp/svn/trunk libbmp
#    cd libbmp
#    sed -e 's/	gcc/'${TARGET_HOST}'-gcc/' -e 's/	g++/'${TARGET_HOST}'-g++/' -e 's/ar/'${TARGET_HOST}'-ar/' Makefile > Makefile.mingw
#    make -f Makefile.mingw libbmp.a bmp_go
#    cp bmp.h ${XCOMPILE_ROOT}/include
#    cp libbmp.a ${XCOMPILE_ROOT}/lib
#    cd ..
#    rm -rf libbmp
#  fi
fi

if test $build_client != no -o $build_manager != no -o $build_libs != no ; then
  if ! test -f ${XCOMPILE_ROOT}/lib/libssl.a ; then
    opensslver=1.0.1g
    wget http://www.openssl.org/source/openssl-${opensslver}.tar.gz
    tar zxf openssl-${opensslver}.tar.gz
    /bin/rm openssl-${opensslver}.tar.gz
    cd openssl-${opensslver}
    ./Configure --prefix=${XCOMPILE_ROOT}/ ${CFLAGS} ${LDFLAGS} no-shared zlib $openssl_cross
    make -j 4 all
    make install
    cd ..
    rm -rf openssl-${opensslver}
  fi
fi

if test $build_client != no -o $build_manager != no ; then
  if ! test -f ${XCOMPILE_ROOT}/lib/libcurl.a ; then
    curlver=7.36.0
    wget http://curl.haxx.se/download/curl-${curlver}.tar.bz2
    tar jxf curl-${curlver}.tar.bz2
    /bin/rm curl-${curlver}.tar.bz2
    cd curl-${curlver}
    ./configure --prefix=${XCOMPILE_ROOT} --enable-static --disable-shared --host=$TARGET_HOST --build=$BUILD_HOST --with-zlib=${XCOMPILE_ROOT}
    make -j 4 all
    make install
    cd ..
    rm -rf curl-${curlver}
  fi
fi

if test $build_client != no ; then
  thisdir=`pwd`
  if ! test -f ${XCOMPILE_ROOT}/include/nvapi.h ; then
    rsync -va ../coprocs/NVIDIA/include/* ${XCOMPILE_ROOT}/include
  fi
  nvdir=
  if ! test -f ${XCOMPILE_ROOT}/lib/libnvapi.a ; then
    mkdir NVIDIA
    case $target_host in
    	i[56]86-*)  nvdir=../coprocs/NVIDIA/mswin/Win32/Release/lib
	            ;;
        x86*|x64*)  nvdir=../coprocs/NVIDIA/mswin/x64/Release/lib
	            ;;
    esac
    if test x$nvdir != x ; then
      rsync -va $nvdir/* NVIDIA
      cd NVIDIA
      obj=`ar t nvapi.lib | head -1`
      objdir=`dirname $obj`
      mkdir -p $objdir
      ar x nvapi.lib
      cd $objdir
      ar cr libnvapi.a *.obj
      mv *.a ${XCOMPILE_ROOT}/lib
    fi
  fi
  cd $thisdir
  rm -rf NVIDIA
fi

if test $build_manager != no ; then
  if ! test -f ${XCOMPILE_ROOT}/lib/libwx_baseu_net-3.0-i686-w64-mingw32.dll.a ; then
    wxver=3.0.0
    filename=wxWidgets-${wxver}.tar.bz2
    wget http://sourceforge.net/projects/wxwindows/files/3.0.0/$filename/download
    mv download $filename
    tar jxf $filename
#    rm -f $filename
    cd wxWidgets-${wxver}
    enables="--with-msw  --with-libpng --with-libjpeg --with-libtiff --with-opengl --with-zlib  --enable-toolbar --enable-statusbar --enable-statbmp --enable-intl --enable-accessibility --enable-catch_segvs --enable-std_containers --enable-std_iostreams --enable-std_string --enable-std_string_conv_in_wxstring --enable-permissive  --enable-xlocale --enable-config --enable-protocols --enable-ftp --enable-http --enable-fileproto --enable-sockets --enable-ole --enable-dataobj --enable-ipc --enable-baseevtloop --enable-selectloop --enable-any --enable-base64 --enable-dialupman --enable-uxtheme "
    ./configure -C --host=$TARGET_HOST --build=$BUILD_HOST ${enables} --with-libcurl=${XCOMPILE_ROOT} --with-ssl=${XCOMPILE_ROOT} --with-winsock --prefix=${XCOMPILE_ROOT}
    make all
    make install
    /bin/cp lib/*.a ${XCOMPILE_ROOT}/lib
    /bin/cp lib/*.dll ${XCOMPILE_ROOT}/bin
    /bin/mv -f ${XCOMPILE_ROOT}/lib/wx*.dll ${XCOMPILE_ROOT}/bin
    cd ..
    rm -rf wxWidgets-${wxver}
  fi
fi

if test $build_manager != no -o $build_libs != no ; then
  if ! test -f ${XCOMPILE_ROOT}/include/GL/glut.h -a -f ${XCOMPILE_ROOT}/lib/libfreeglut_static.a  ; then
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
    make -j 4 all
    make install
    cd ../..
    /bin/rm -rf freeglut
  fi
fi

enables="--enable-static --disable-shared"
if test $build_client != no ; then
  enables="${enables} --enable-client"
else
  enables="${enables} --disable-client"
fi
if test $build_server != no ; then
  enables="${enables} --enable-server"
else
  enables="${enables} --disable-server"
fi
if test $build_libs != no ; then
  enables="${enables} --enable-libraries"
else
  enables="${enables} --disable-libraries"
fi
if test $build_manager != no ; then
  enables="${enables} --enable-manager"
else
  enables="${enables} --disable-manager"
fi

../configure -C --host=$TARGET_HOST --build=$BUILD_HOST ${enables} --with-libcurl=${XCOMPILE_ROOT} --with-ssl=${XCOMPILE_ROOT} --with-winsock --prefix=${XCOMPILE_ROOT}
make -j 4 all
#exit 0
