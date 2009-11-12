#!/bin/sh

# Automate the compilation of the various locale PO files by automatically
# generating them at night.
#
projname=boinctrunk
projdir=$HOME/pootle/po/$projname

cd $projdir

# Iterrate through the various PO files looking for those that need to be compiled.
#
for file in `find -name 'BOINC-Manager.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
 
  timestampPO=`date -r ${projdir}/${locale}/BOINC-Manager.po`
  timestampMO=`date -r ${projdir}/${locale}/BOINC-Manager.mo`
 
  if [ "${timestampPO}" != "${timestampMO}" ]; then

    # Remove old MO from previous compilation
    #
    rm ${projdir}/BOINC-Manager.mo > /dev/null 2> /dev/null

    # Use wget to cause the Pottle system to compile the PO file into an MO file.
    #
    # poEdit has a hard time with the Pootle markup in the PO files.
    #
    # Example: http://boinc.berkeley.edu/translate/ar/boinctrunk/BOINC-Manager.mo
    #
    wget http://boinc.berkeley.edu/translate/${locale}/${projname}/BOINC-Manager.mo > /dev/null 2> /dev/null
    
    # Add any new MO files to SVN
    svn add ${projdir}/${locale}/BOINC-Manager.mo > /dev/null 2> /dev/null

    # Touch each file to adjust timestamps
    touch ${projdir}/${locale}/BOINC-Manager.po
    touch ${projdir}/${locale}/BOINC-Manager.mo 

  fi  

done

# Remove old MO from previous compilation
#
rm ${projdir}/BOINC-Manager.mo > /dev/null 2> /dev/null

# Commit any changes to SVN
svn commit -m 'Update Translations'

exit 0
