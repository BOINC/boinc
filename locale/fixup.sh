#!/bin/sh

# Automate the compilation of the various locale PO files by automatically
# generating them at night.
#
projname=boinctrunk
projdir=$HOME/pootle/po/$projname

cd $projdir


# Iterrate through the various PO files looking for those that need to be added to SVN.
#
for file in `find -name 'BOINC-Client.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Client
 
  svn propset svn:mime-type 'text/plain;charset=UTF-8' ${template_name}.po > /dev/null 2> /dev/null
done


# Commit any changes to SVN
svn commit -m 'Update Translations'

exit 0
