#!/bin/bash

# This script ensures that only one week of the scheduler logs are
# archived for users to study.

# To run this script as part of the standard BOINC backend services
# (once every 24 hours) just include this:
#      <task>
#        <cmd> cleanlogs.sh </cmd>
#        <output>cleanlogs.sh.log</output>
#        <period>24 hr</period>
#      </task>
# in your project's config.xml file

cd ../html/user/sched_logs/ || exit 1
find . -type d -name "20*" -mtime +14 | xargs rm -rf || exit 2
echo "`date '+%Y-%m-%d %H:%M:'`" "cleaned scheduler logs"
exit 0
