# $Id$

# boinc_project_path.py - defines the directories where config.xml and
# run_state.xml are kept.
#
# You can override these defaults
#    1) modify this file directly (if you have only one project on your server
#       or have separate copies for each)
#    2) create a new boinc_project_path.py and place it earlier in PYTHONPATH
#       than the default one
#    3) define environment variables

import sys, os

PROGRAM_DIR = os.path.dirname(sys.argv[0])
PROGRAM_PARENT_DIR = os.path.join(PROGRAM_DIR,os.path.pardir)

config_xml_filename = os.environ.get(
    'BOINC_CONFIG_XML',
    os.path.join(PROGRAM_PARENT_DIR,'config.xml'))

run_state_xml_filename = os.environ.get(
    'BOINC_RUN_STATE_XML',
    os.path.join(PROGRAM_PARENT_DIR,'run_state.xml'))
