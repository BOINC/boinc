#!/bin/sh

# Automate the compilation of the various locale PO files by automatically
# generating them at night.
#
projname=boinctrunk
projdir=/home/boincadm/pootle/repos/$projname

cd $projdir


# Iterrate through the various PO files looking for those that need to be compiled.
#
for file in `find -name 'BOINC-Manager.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Manager
 
  if test ${template_name}.po -nt ${template_name}.mo
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo 
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo 

  fi  
done


# Iterrate through the various PO files looking for those that need to be compiled.
#
for file in `find -name 'BOINC-Client.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Client
 
  if test ${template_name}.po -nt ${template_name}.mo
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo 

  fi  
done


# Iterrate through the various PO files looking for those that need to be compiled.
#
for file in `find -name 'BOINC-Web.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Web
 
  if test ${template_name}.po -nt ${template_name}.mo
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo 

  fi  
done


# Iterrate through the various PO files looking for those that need to be compiled.
#
for file in `find -name 'BOINC-Setup.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Setup
 
  if test ${template_name}.po -nt ${template_name}.mo
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo 

  fi  
done

git commit -a -m "locale: Update compiled localization files"
git push origin

exit 0
