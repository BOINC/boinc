#!/bin/bash
# Provide a go between the historical way BOINC has tracked commit changes with the way
# git manages historical information.  Basically mimic the checkin_notes file with the
# output of various git commands
#

# Command line customizations here



# Get a list of commit ids to extract the log information for
for i in `git log --since="1 week ago" --pretty="%H"`; 
do
    git show --name-status --pretty=fuller $i
    echo "$ii"
done
