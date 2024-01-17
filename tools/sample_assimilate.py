#! /usr/bin/env python3

# invoked either as
# sample_assimilate.py batch_id outfile_path1 ...
# or
# sample_assimilator.py --error error_code wu_name batch_id
#
# in the 1st case, move the output files from the upload hierarchy
# to sample_results/batch_id/
# in the 2nd case, append a line of the form
#   wu_name error_code
# to samples_results/batch_id/errors

import sys, os

if sys.argv[1] == '--error':
    error_code = sys.argv[2]
    wu_name = sys.argv[3]
    wu_id = sys.argv[4]
    batch_id = sys.argv[5]
    outdir = '../sample_results/%s'%(batch_id)
    cmd = 'mkdir -p %s'%(outdir)
    if os.system(cmd):
        raise Exception('%s failed'%(cmd))
    with open('%s/errors'%(outdir), 'a') as f:
        f.write('%s %s\n'%(wu_name, error_code))
else:
    batch_id = sys.argv[1]
    outfile_path = sys.argv[2]
    fname = os.path.basename(outfile_path)
    outdir = '../sample_results/%s'%(batch_id)
    cmd = 'mkdir -p %s'%(outdir)
    if os.system(cmd):
        raise Exception('%s failed'%(cmd))
    cmd = 'mv %s %s/%s'%(outfile_path, outdir, fname)
    if os.system(cmd):
        raise Exception('%s failed'%(cmd))
