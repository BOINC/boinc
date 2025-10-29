#! /usr/bin/env python3

# Assimilator for batch-oriented apps
# Use with the script-based assimilator (sched/script_assimilator.cpp)
# Moves output files into a results/ hierarchy
#
# Use with a config.xml command of the form
# <cmd>script_assimilator -d 3 --app worker --script "sample_assimilate.py wu_name batch_id files2"</cmd>

# With this command, this script will be invoked either as
#       sample_assimilate.py wu_name batch_id outfile_path1 logical_name1 ...
# or
#       sample_assimilator.py --error error_code wu_name wu_id batch_id
#
# in the 1st case, move the output files from the upload hierarchy
#       to results/<batch_id>/<wu_name>__file_<log_name>
#       where <log_name> is the file's logical name
# in the 2nd case, write the error code
#        to results/<batch_id>/<wu_name>_error

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

    nfiles = (len(sys.argv) - 3)//2
    for i in range(nfiles):
        outfile_path = sys.argv[2*i+3]
        logical_name = sys.argv[2*i+4]
        cmd = 'mv %s %s/%s__file_%s'%(
            outfile_path, outdir, wu_name, logical_name
        )
        if os.system(cmd):
            #raise Exception('%s failed'%(cmd))
            sys.stderr.write('%s failed\n'%(cmd))
