#!/bin/sh

# Automate the compilation of the various locale PO files by automatically
# generating them at night.
#
projname=boinctrunk
projdir=$HOME/pootle/po/$projname

cd $projdir


# Update anything that needs updating
svn update


# Iterrate through the various PO files looking for those that need to be compiled.
#
for file in `find -name 'BOINC-Manager.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Manager
 
  # Remove old MO from previous compilation
  #
  rm ${projdir}/BOINC-Manager.mo > /dev/null 2> /dev/null

  if test ${template_name}.po -nt ${template_name}.mo
  then

    # Use wget to cause the Pottle system to compile the PO file into an MO file.
    #
    # poEdit has a hard time with the Pootle markup in the PO files.
    #
    # Example: http://boinc.berkeley.edu/translate/ar/boinctrunk/BOINC-Manager.mo
    #
    wget "http://boinc.berkeley.edu/translate/${locale}/${projname}/BOINC-Manager.mo" > /dev/null 2> /dev/null
    
    # Add any new MO files to SVN
    svn add ${template_name}.mo > /dev/null 2> /dev/null

    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo 

  fi  
done


# Determine if we need to update the various languages using the templates.
# This will be done by the use of a tag file which should have a matching 
# timestamp as the template files. If the timestamps do not match update all
# languages.
for file in `find -name '*.pot'` ; do
  template_rootname=`basename $file .pot`
  template_name=${projdir}/templates/${template_rootname}

  # Check to see if the file exists, if not create it
  if test ! -e ${template_name}.flag
  then
    cp ${template_name}.pot ${template_name}.flag
  fi
  
  # If the modification timestamps don't match then update all the languages
  if test ${template_name}.pot -nt ${template_name}.flag
  then
    execute_update=true
  fi  
done

if test "${execute_update}" = "true"
then

  for file in `find -name '*.po'` ; do
    dir=`dirname $file`
    locale=`basename $dir`
    po_name=`basename $file .po`

    msgmerge --update ${locale}/${po_name}.po templates/${po_name}.pot
 
  done

fi

for file in `find -name '*.pot'` ; do
  template_rootname=`basename $file .pot`
  template_name=${projdir}/templates/${template_rootname}

  # Touch each file to adjust timestamps
  touch ${template_name}.pot
  touch ${template_name}.flag

done


# Commit any changes to SVN
svn commit -m 'Update Translations'

exit 0
