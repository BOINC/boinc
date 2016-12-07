#!/bin/bash

set -e # abort if a command exits non-zero

# update localization files from transifex and compile .po to .mo if needed
# .po files are only updated if they are 100% translated, outdated files are not removed!
# Then commit and push changes.

testmode=0
if test $# -gt 0; then
  if test $1 = "-t"; then
    testmode=1
  else
    echo "Usage: $0 [-t]"
    echo "     -t  testmode (don't commit or push to git repository)"
    exit 1
  fi
fi

# find source root upward from CWD
while ! test -r .tx/config; do
  cd ..
  test "`pwd`" = "/" && echo "no source directory found" >&2 && exit
done

command -v pocompile >/dev/null 2>&1 || { echo >&2 "pocompile (translate-toolkit) is needed but not installed.  Aborting."; exit 1; }
command -v tx >/dev/null 2>&1 || { echo >&2 "tx (transifex-client) is needed but not installed.  Aborting."; exit 1; }

# check if working directory is clean to ensure we only commit localization changes
if test 0 -ne `git status -s -uno |wc -l`; then
  echo "Please commit your pending changes first"
  exit 1
fi

echo "pulling translations from transifex"
# this only updates existing languages, new languages need to be added manually with 'tx pull -a' and 'git add'
tx pull

echo "compiling localization files for Manager and Client"
srcdir=`pwd`
cd ${srcdir}/locale

templates=("BOINC-Manager" "BOINC-Client" "BOINC-Setup" "BOINC-Web")

for template_name in "${templates[@]}"; do
  for file in `find -name "${template_name}.po"`; do
    dir=`dirname $file`
    locale=`basename $dir`

    cd ${srcdir}/locale/${locale}
    if test ${template_name}.po -nt ${template_name}.mo.flag || test ! -e ${template_name}.mo.flag; then
      # Compile the PO file into an MO file.
      pocompile ${template_name}.po ${template_name}.mo

      # Touch each file to adjust timestamps
      touch ${template_name}.po
      touch ${template_name}.mo.flag

    fi
  done
  cd ${srcdir}/locale
done

cd ${srcdir}

git add -u # only update already tracked files (will not track new files)
if test $testmode -eq 0; then
  git commit -m "Locale: Update localization files [skip ci]"
  git push
else
  echo "working directory prepared for commit, inspect changes with 'git diff --cached'"
fi

exit 0
