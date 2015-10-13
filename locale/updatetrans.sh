#!/bin/sh

# Look for .po files modified later than .mo, and regenerate .mo file
# Then commit and push changes.

projname=boinc
projdir=/home/boincadm/rwalton/$projname/locale

cd $projdir

for file in `find -name 'BOINC-Manager.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=BOINC-Manager

  cd $projdir/${locale}
 
  if test ${template_name}.po -nt ${template_name}.mo.flag
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo

	# Add the updated file to git
	git add ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo.flag 

  fi  
done


cd $projdir

for file in `find -name 'BOINC-Client.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=BOINC-Client

  cd $projdir/${locale}
 
  if test ${template_name}.po -nt ${template_name}.mo.flag
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo

	# Add the updated file to git
	git add ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo.flag

  fi  
done


cd $projdir

for file in `find -name 'BOINC-Web.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=BOINC-Web

  cd $projdir/${locale}
 
  if test ${template_name}.po -nt ${template_name}.mo.flag
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo

	# Add the updated file to git
	git add ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo.flag

  fi  
done


cd $projdir

for file in `find -name 'BOINC-Setup.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=BOINC-Setup

  cd $projdir/${locale}
 
  if test ${template_name}.po -nt ${template_name}.mo.flag
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo

	# Add the updated file to git
	git add ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo.flag

  fi  
done

git commit -a -m "locale: Update compiled localization files"
git push origin

exit 0
