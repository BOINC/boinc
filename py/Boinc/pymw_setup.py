import configxml
from optparse import OptionParser
import os
import logging
import sys

parser = OptionParser(usage="usage: %prog")
parser.add_option("-p", "--pymw_dir", dest="pymw_dir", default="", help="specify the working directory of pymw", metavar="<ABSOLUTE_PATH>")
options, args = parser.parse_args()

logging.basicConfig(level=logging.ERROR, format="%(asctime)s %(levelname)s %(message)s")

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

# Append new instance of pymw_assimilator
config.daemons.make_node_and_append("daemon").cmd = "pymw_assimilator -d 3 -app pymw -pymw_dir " + pymw_dir
config.write()

print "pymw setup successful"
