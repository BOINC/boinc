#!/usr/bin/env python
try:
    import boinc_path_config
    from  Boinc import configxml, projectxml
except:
    import configxml, projectxml
from optparse import OptionParser
import os, logging, sys, stat

LINUX_WORKER = """\
#!/bin/sh
`python $1 $2 $3`
`touch boinc_finish_called`
"""

WINDOWS_WORKER = """\
@echo off
set _path=%path%
set _python=%_path:*python=python%
set _python=%_python:~0,6%
if %_python% neq python set PATH=%path%;C:\python25
python %1 %2 %s3
type nul > boinc_finish_called
"""

PYMW_APP = """\
import sys
import cPickle

def pymw_get_input():
    infile = open(sys.argv[1], 'r')
    obj = cPickle.Unpickler(infile).load()
    infile.close()
    return obj

def pymw_return_output(output):
    outfile = open(sys.argv[2], 'w')
    cPickle.Pickler(outfile).dump(output)
    outfile.close()
"""

FILE_REF = """\
<copy_file/>
"""

parser = OptionParser(usage="usage: %prog")
parser.add_option("-p", "--pymw_dir", dest="pymw_dir", default="", help="specify the working directory of pymw", metavar="<ABSOLUTE_PATH>")
options, args = parser.parse_args()

logging.basicConfig(level=logging.INFO, format="%(levelname)s %(message)s")

pymw_dir = options.pymw_dir

if pymw_dir == "":
    logging.error("py_mw working directory have to be provided (-p switch)")
    sys.exit(0)
elif not os.path.exists(pymw_dir):
    logging.error(pymw_dir + " does not exist")
    sys.exit(0)

logging.info("pymw working directory set to " + pymw_dir)

logging.info("writing pymw_assimilator in config.xml")
config = configxml.ConfigFile().read()

# Remove old instances of pymw_assimilator
for daemon in config.daemons:
    if daemon.cmd[0:16] == 'pymw_assimilator':
	config.daemons.remove_node(daemon)

# Append new instance of pymw_assimilator to config.xml
config.daemons.make_node_and_append("daemon").cmd = "pymw_assimilator -d 3 -app pymw -pymw_dir " + pymw_dir
config.write()

# Append new instance of pymw worker to project.xml
project = projectxml.ProjectFile().read()
found = False
for element in project.elements:
    if element.name == 'pymw':
	logging.info("PyMW client applications are already installed")
	sys.exit(0)

project.elements.make_node_and_append("app").name = "pymw"
project.write()
for element in project.elements:
    if element.name == "pymw":
	element.user_friendly_name = "PyMW - Master Worker Computing in Python"
        project.write()

# Install worker applications
app_dir = os.path.join(config.config.app_dir, "pymw")
print app_dir
if os.path.isdir(app_dir):
    logging.info("PyMW client applications are already installed")
else:
    os.mkdir(app_dir)
    # Linux
    logging.info("setting up client application for Linux platform")
    linux_dir = os.path.join(app_dir, "pymw_1.00_i686-pc-linux-gnu")
    linux_exe = os.path.join(linux_dir, "pymw_1.00_i686-pc-linux-gnu")
    os.mkdir(linux_dir)
    open(linux_exe, "w").writelines(LINUX_WORKER)
    open(os.path.join(linux_dir, "pymw_1.00_i686-pc-linux-gnu.file_ref_info"), "w").writelines(FILE_REF)
    open(os.path.join(linux_dir, "pymw_app.py"), "w").writelines(PYMW_APP)
    open(os.path.join(linux_dir, "pymw_app.py.file_ref_info"), "w").writelines(FILE_REF)
    os.chmod(linux_exe, stat.S_IRWXU)
    logging.info("client application for Linux platform set up successfully")
    
    # Windows
    logging.info("setting up client application for Windows platform")
    win_dir = os.path.join(app_dir, "pymw_1.00_windows_intelx86")
    win_exe = os.path.join(win_dir, "pymw_1.00_windows_intelx86.exe")
    os.mkdir(win_dir)
    open(win_exe, "w").writelines(WINDOWS_WORKER)
    open(os.path.join(win_dir, "pymw_1.00_windows_intelx86.exe.file_ref_info"), "w").writelines(FILE_REF)
    open(os.path.join(win_dir, "pymw_app.py"), "w").writelines(PYMW_APP)
    open(os.path.join(win_dir, "pymw_app.py.file_ref_info"), "w").writelines(FILE_REF)
    os.chmod(win_exe, stat.S_IRWXU)
    logging.info("client application for Windows platform set up successfully")
    
    # Call update_versions
    project_home = config.config.app_dir.rpartition('/')[0]
    os.chdir(project_home)
    os.system("xadd")
    os.system("update_versions")
logging.info("---------------------")
logging.info("PyMW setup successful")
logging.info("---------------------")
