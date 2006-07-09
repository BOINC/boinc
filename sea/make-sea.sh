#!/bin/sh
if [ $# -lt 1 ] || [ $# -gt 3 ]; then
  echo 'usage: make-sea.sh <archive.tar> [<filename>.sh] [<install-script>.sh]'
  exit
fi

# parse optional arguments
if [ -z "$2" ]; then
  filename="`uname`_sea.sh"
else
  filename="$2"
fi
if [ -z "$3" ]; then
  install=install.sh
else
  install="$3"
fi

# peek into archive for the install script
if tar tf "$1" | grep "$install" >/dev/null; then
  :
else
  echo "the archive \"$1\" doesn't contain the specified install script \"$install\""
  exit
fi

# find out about compression to use
# Linux usually doesn't even have compress, otherwise its standard
if [ `uname` = "Linux" ]; then
  compress=gzip
  expand=gunzip
else
  compress=compress
  expand=uncompress
fi

echo '#!/bin/sh
( read l; read l; read l; exec cat ) < "$0" | '$expand' | tar xf - && /bin/sh '"$install $*"'
exit' > "$filename" &&
$compress < "$1" >> "$filename" &&
chmod +x "$filename"
