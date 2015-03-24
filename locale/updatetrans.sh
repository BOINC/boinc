#!/bin/sh

# Look for .po files modified later than .mo, and regenerate .mo file
# Then commit and push changes.

# This is run in the Pootle copy of the source tree (~/pootle/repos/boinctrunk)
# It's run from pootle/update.sh, which is run from cron every 12 hours.
#
projname=boinc
projdir=/home/boincadm/rwalton/$projname/locale

cd $projdir


for file in `find -name 'BOINC-Manager.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Manager
 
  if test ${template_name}.po -nt ${template_name}.mo
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo

	# Add the updated file to git
	git add ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo 

  fi  
done


for file in `find -name 'BOINC-Client.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Client
 
  if test ${template_name}.po -nt ${template_name}.mo
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo

	# Add the updated file to git
	git add ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo 

  fi  
done


for file in `find -name 'BOINC-Web.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Web
 
  if test ${template_name}.po -nt ${template_name}.mo
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo

	# Add the updated file to git
	git add ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo 

  fi  
done


for file in `find -name 'BOINC-Setup.po'` ; do
  dir=`dirname $file`
  locale=`basename $dir`
  template_name=${projdir}/${locale}/BOINC-Setup
 
  if test ${template_name}.po -nt ${template_name}.mo
  then

    # Compile the PO file into an MO file.
    pocompile ${template_name}.po ${template_name}.mo

	# Add the updated file to git
	git add ${template_name}.mo
    
    # Touch each file to adjust timestamps
    touch ${template_name}.po
    touch ${template_name}.mo 

  fi  
done

git commit -a -m "locale: Update compiled localization files"
git push origin

exit 0
