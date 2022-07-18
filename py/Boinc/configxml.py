#!/usr/bin/env python

# configxml.py - module to read and parse config.xml, run_state.xml

'''
SYNOPSIS:  parses and writes config.xml and run_state.xml

USAGE:     from Boinc import configxml
           config = configxml.ConfigFile().read()
           run_state = configxml.RunStateFile().read()
           print config.config.db_name
           print config.tasks[4].cmd
           run_state.enabled = True
           new_task = newConfigDict()
           new_task.cmd = "echo hi | mail quarl"
           config.tasks.append(new_task)
           config.write()
           run_state.write()

'''

from __future__ import print_function
import sys
from Boinc import boinc_project_path
from Boinc.boincxml import *

default_config_file = None

class ConfigFile(XMLConfig):
    '''
    embodies config.xml
    Public attributes:
        config   - ConfigDict
        tasks    - list of ConfigDict elements
        daemons  -
    '''
    default_filename = boinc_project_path.config_xml_filename
    def __init__(self, *args, **kwargs):
        XMLConfig.__init__(self, *args, **kwargs)
        global default_config_file
        default_config_file = self
    def _get_elements(self):
        self.xml_boinc   = get_element(self.xml,  'boinc', optional=False)
        self.xml_config  = get_element(self.xml_boinc, 'config', optional=False)
        self.xml_tasks   = get_element(self.xml_boinc, 'tasks')
        self.xml_daemons = get_element(self.xml_boinc, 'daemons')
        self.config      = ConfigDict(self.xml_config)
        self.daemons     = ConfigDictList(self.xml_daemons)
        self.tasks       = ConfigDictList(self.xml_tasks)
    def _set_elements(self):
        self.config.save()
        self.daemons.save()
        self.tasks.save()
    def debug_print_all(self):
        '''print everything to stdout.'''

        print('Debug dump of', self.filename)
        print('-- parsed xml -------------------------------------------------------')
        self.xml.writexml(sys.stdout)
        print()
        print('-- Config -----------------------------------------------------------')
        self.config.debug_print()
        print()
        print('-- Daemons ------------------------------------------------------------')
        for daemon in self.daemons:
            daemon.debug_print()
            print()
        print()
        print('-- Tasks ------------------------------------------------------------')
        for task in self.tasks:
            task.debug_print()
            print()
    default_xml = '<boinc><config></config></boinc>'

# keeps BoincCron's timestamp status file
class RunStateFile(XMLConfig):
    '''
    embodies run_state.xml
    Public attributes:
       tasks - list of ConfigDict elements
       enabled - boolean
    '''
    default_filename = boinc_project_path.run_state_xml_filename
    def _get_elements(self):
        self.xml_boinc    = get_element(self.xml,  'boinc', optional=False)
        self.xml_tasks    = get_element(self.xml_boinc, 'tasks')
        self.xml_enabled  = get_element(self.xml_boinc, 'enabled')
        self.tasks        = ConfigDictList(self.xml_tasks)
        self.enabled      = get_element_int(self.xml_enabled)
    def _set_elements(self):
        set_element( self.xml_enabled, self.enabled )
        self.tasks.save()
    default_xml = '<boinc></boinc>'

def default_config():
    '''If any config file has been read, return it.  Else open the default one
    and return it.'''
    if not default_config_file: ConfigFile().read()
    assert(default_config_file)
    return default_config_file

if __name__ == '__main__':
    config = ConfigFile().read()
    # print("setting config.enabled = True")
    # config.enabled = True
    config.debug_print_all()
    print(" -- saving xml and rewriting -----------------------------------------------")
    config.write(sys.stdout)
