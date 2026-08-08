#!/bin/bash

set -e # abort if a command exits non-zero

# update localization files from transifex and compile .po to .mo
# .po files are updated according to minimum_perc in .tx/config, outdated files are not removed!
# Then commit and push changes manually.

# find source root upward from CWD
while ! test -r .tx/config; do
  cd ..
  test "`pwd`" = "/" && echo "no source directory found" >&2 && exit
done

command -v pocompile >/dev/null 2>&1 || { echo >&2 "pocompile (translate-toolkit) is needed but not installed.  Aborting."; exit 1; }
command -v tx >/dev/null 2>&1 || { echo >&2 "tx (transifex-client) is needed but not installed.  Aborting."; exit 1; }

# check if working directory is clean to ensure we only commit localization changes
if test 0 -ne `git status -s -uno |wc -l`; then
  echo "Warning: You have pending changes! Please make sure to only commit localization changes!"
fi

echo "pulling translations from transifex"
# this updates existing languages and adds new languages
tx pull -a -f

echo "compiling localization files for Manager and Client"
srcdir=`pwd`
cd ${srcdir}/locale

templates=("BOINC-Manager" "BOINC-Client" "BOINC-Setup" "BOINC-Web")

for template_name in "${templates[@]}"; do
  for file in `find -name "${template_name}.po"`; do
    dir=`dirname $file`
    locale=`basename $dir`

    cd ${srcdir}/locale/${locale}
    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo
    # Touch each file to adjust timestamps
    touch ${template_name}.po

  done
  cd ${srcdir}/locale
done

echo "running pofilter for BOINC-Manager.po"
echo "Please check output in BOINC-Manager-pofilter.txt!"
echo "" > BOINC-Manager-pofilter.txt
for file in `find -name "BOINC-Manager.po"`; do
  dir=`dirname $file`
  locale=`basename $dir`
  echo $file >> BOINC-Manager-pofilter.txt
  pofilter --language ${locale} -t printf -t escapes -t numbers -t tabs --nofuzzy ${srcdir}/locale/${locale}/BOINC-Manager.po >> BOINC-Manager-pofilter.txt
done

cd ${srcdir}

echo "Translations compiled successfully. Now some manual steps:"
echo " 1. less BOINC-Manager-pofilter.txt # check output from pofilter and adjust translations then start this script again"
echo " 2. git add -u # only update already tracked files (add new files when needed too)"
echo " 3. git commit -m \"Locale: Update localization files [skip ci]\""
echo " 4. git push"

exit 0
