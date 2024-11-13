#! /usr/bin/env python3

# Sample script for the script-based assimilator (sched/script_assimilator.cpp)
# Moves output files into a results/ dir hierarchy
#
# Use with a config.xml command of the form
# <cmd>script_assimilator -d 3 --app worker --script "sample_assimilate.py wu_name batch_id files"</cmd>

# With this command, this script will be invoked either as
# sample_assimilate.py wu_name batch_id outfile_path1 ...
# or
# sample_assimilator.py --error error_code wu_name wu_id batch_id
#
# in the 1st case, move the output files from the upload hierarchy
# to results/<batch_id>/<wu_name>_i
# in the 2nd case, write the error code 
# to results/<batch_id>/<wu_name>_error

import sys, os

if sys.argv[1] == '--error':
    error_code = sys.argv[2]
    wu_name = sys.argv[3]
    wu_id = sys.argv[4]
    batch_id = sys.argv[5]
    outdir = '../results/%s'%(batch_id)
    cmd = 'mkdir -p %s'%(outdir)
    if os.system(cmd):
        raise Exception('%s failed'%(cmd))
    with open('%s/%s_errors'%(outdir, wu_name), 'a') as f:
        f.write('%s\n'%(error_code))
else:
    wu_name = sys.argv[1]
    batch_id = sys.argv[2]
    outdir = '../results/%s'%(batch_id)
    cmd = 'mkdir -p %s'%(outdir)
    if os.system(cmd):
        raise Exception('%s failed'%(cmd))

    nfiles = len(sys.argv) - 3
    if nfiles == 1:
        outfile_path = sys.argv[3]
        cmd = 'mv %s %s/%s'%(outfile_path, outdir, wu_name)
        if os.system(cmd):
            raise Exception('%s failed'%(cmd))
    else:
        for i in range(nfiles):
            outfile_path = sys.argv[i+3]
            cmd = 'mv %s %s/%s_%d'%(outfile_path, outdir, wu_name, i)
            if os.system(cmd):
                raise Exception('%s failed'%(cmd))
