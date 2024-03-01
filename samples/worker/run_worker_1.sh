#! /bin/sh
# script for running worker in a VM, with shared slot and project dirs

mkdir /root/project >>log 2>&1
mount -t vboxsf project /root/project >>log 2>&1
execpath=`./boinc_resolve worker` >>log 2>&1
inpath=`./boinc_resolve in` >>log 2>&1
outpath=`./boinc_resolve out` >>log 2>&1

echo $execpath >>log
echo $inpath >>log
echo $outpath >>log

$execpath $inpath $outpath >>log 2>&1
