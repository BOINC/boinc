#!/bin/bash

# This daemon (endless loop) constructs user-readable scheduler
# logs once per minute.  It runs with a 3-minute latency.

# To run this script as part of the standard BOINC backend
# components, just include:
#    <daemon>
#      <cmd> makelog.sh </cmd>
#    </daemon>
# in your config.xml file

export lastmin=dontmatch

while [ true ] ; do

# we extract the logs from 3 minutes ago, because the cgi
# process can run for a couple of minutes.
  export currmin=`date --date="-3 minute" '+%Y-%m-%d %H:%M'`

# create correct filename, directory name, path, etc.
  export oneword=`echo $currmin | sed 's/ /_/'`
  export filename=${oneword}.txt
  export dirname=`echo $filename | cut -b 1-13`
  export dirpath=../html/user/sched_logs/${dirname}
  if [ ! -d $dirpath ] ; then
    mkdir $dirpath
  fi
  export filepath=${dirpath}/${filename}

# if the minute has changed, or the one-minute log file does not exist,
# then create it.  One must consider both cases otherwise log rotation
# can screw things up.
  if [ "${currmin}" != "${lastmin}" -o ! -f "${filepath}" ] ; then

# put some text into the start of the 'human readable' log file
    echo "Note that all times in this file are in UTC.  To compare with your BOINC logs"           > $filepath
    echo "you will need to convert your local time into UTC.  To make comparison easier you"      >> $filepath
    echo "may also want to consider using a high-precision time protocol such as NTP to set your" >> $filepath
    echo "computers clock.  This will allow comparisons of the time stamps to fractions of"       >> $filepath
    echo "a second."                                                                              >> $filepath
    echo " "                                                                                      >> $filepath
    echo "These files are posted approximately three minutes after the events are logged."        >> $filepath
    echo " "                                                                                      >> $filepath
    echo " "                                                                                      >> $filepath

# determine which files to search.  Normally we just search scheduler.log, but if the timestamp of scheduler.log
# and scheduler.log.0 are within 600 seconds of each other, then we search both. The 10,000,000 case will
# handle what happens if either $cgilogtime or $cgilog0time is not properly defined.

    export cgilog0time=`ls --time-style=+%s -l ../log_*/scheduler.log.0 | awk '{print $6}'`
    export cgilogtime=$cgilog0time
    export cgilogtime=`ls --time-style=+%s -l ../log_*/scheduler.log | awk '{print $6}'`
    export deltatime=$(($cgilogtime-$cgilog0time))
    export filelist="../log_*/scheduler.log"
    if [ $deltatime -lt 600 ] || [ $deltatime -gt 10000000 ] ; then
        export filelist="../log_*/scheduler.log ../log_*/scheduler.log.0"
    fi

# now grep for all log entries from 3 minutes ago.  Use sed to hide any sensitive info
# such as authenticator and IP address.  Must
    grep --no-filename "${currmin}" $filelist | \
    sed 's/authenticator .*//g; s/\[auth [^]]*\]//g; s/from [0-9.]*//g; s/auth [0-9a-f]*\,//g; s/\[IP [0-9.]*\]//g; s/\[USER#[0-9]*\]//g; s/IP [0-9.]*\,//g' >> $filepath
    export lastmin=$currmin
  else

# if the minute has not changed and log file exists, sleep a bit and
# try again
    sleep 15;
  fi
done

# this is an infinite loop, so we should never get here!!
