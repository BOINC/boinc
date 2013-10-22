#!/bin/bash
# Provide a go between the historical way BOINC has tracked commit changes with the way
# git manages historical information.  Basically mimic the checkin_notes file with the
# output of various git commands
#

# Command line customizations here

# Get a list of commit ids to extract the log information for
git log --name-status --pretty=fuller --since="$1 day ago"