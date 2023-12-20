#! /bin/sh
# script for running worker in a VM, with shared slot and project dirs

mkdir /root/project
mount -t vboxsf project /root/project
execpath=`./boinc_resolve worker`
inpath=`./boinc_resolve in`
outpath=`./boinc_resolve out`

$execpath $inpath $outpath
