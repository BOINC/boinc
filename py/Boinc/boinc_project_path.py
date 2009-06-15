# $Id$

# boinc_project_path.py - defines the directories where config.xml and
# run_state.xml are kept.
#
# You can override these defaults with one of these options:
#    1) modify this file directly (if you have only one project on your server
#       or have separate copies for each)
#    2) create a new boinc_project_path.py and place it earlier in PYTHONPATH
#       than the default one
#    3) define environment variables

import sys, os, socket

# Default locations to look for config.xml: ".", "..", "../..", and the parent
# directory of the script's location
_dirs = [ os.curdir, os.pardir, os.path.join(os.pardir, os.pardir),
          os.path.join(os.path.dirname(sys.argv[0]), os.pardir) ]

# BOINC_PROJECT_DIR takes precedence if it is defined
_project_dir = ''
if os.environ.get('BOINC_PROJECT_DIR'):
    _project_dir = os.environ.get('BOINC_PROJECT_DIR')
else:
    for dir in _dirs:
        if os.path.exists(os.path.join(dir, 'config.xml')):
            _project_dir = os.path.abspath(dir)
            break

_local_hostname = socket.gethostname()
_local_hostname = _local_hostname.split('.')[0]

def project_path(name = None):
    if name:
        return os.path.join(_project_dir, name)
    else:
        return _project_dir

config_xml_filename = project_path('config.xml')
project_xml_filename = project_path('project.xml')
run_state_xml_filename = project_path('run_state_' + _local_hostname + '.xml')
