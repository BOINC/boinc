#!/usr/bin/env python

# $Id$

# boinc_config.py - module to read and parse config.xml, run_state.xml

# The contents of this file are subject to the BOINC Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http:#boinc.berkeley.edu/license_1.0.txt
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the Berkeley Open Infrastructure for Network Computing.
#
# The Initial Developer of the Original Code is the SETI@home project.
# Portions created by the SETI@home project are Copyright (C) 2002
# University of California at Berkeley. All Rights Reserved.
#
# Contributor(s):
#

'''
SYNOPSIS:  parses and writes config.xml and run_state.xml

USAGE:     from boinc_config import *
           config = BoincConfig('confing.xml').read()
           run_state = BoincRunState('run_state.xml').read()
           print config.config.db_name
           print config.tasks[4].cmd
           run_state.enabled = True
           new_task = newConfigDict()
           new_task.cmd = "echo hi | mail quarl"
           config.tasks.append(new_task)
           config.write()
           run_state.write()

TODO: create a Boinc package and remove these Boinc prefix qualifiers
'''

import sys
import xml.dom.minidom

CONFIG_FILE = '../config.xml'
RUN_STATE_FILE = '../run_state.xml'

def _get_elements(node, name):
    return node.getElementsByTagName(name)

def _get_element(node, name, optional=False):
    try:
        return _get_elements(node,name)[0]
    except IndexError:
        if optional:
            new_element = xml.dom.minidom.Element(name)
            node.appendChild(new_element)
            return new_element
        raise SystemExit("ERROR: Couldn't find xml node <%s>"% name)

def _None2Str(object):
    if object == None:
        return ''
    else:
        return object

def _get_element_data(node):
    return node and _None2Str(node.firstChild and node.firstChild.data)

def _get_element_int(node, default=0):
    try:
        return int(_get_element_data(node))
    except:
        return default

def _get_child_elements(node):
    return filter(lambda node: node.nodeType == node.ELEMENT_NODE, node.childNodes)

def _set_element(node, new_data):
    if node.firstChild and node.firstChild.data:
        node.firstChild.data = str(new_data)
    else:
        new_data_node = xml.dom.minidom.Text()
        new_data_node.data = str(new_data)
        node.appendChild(new_data_node)

class ConfigDict:
    def __init__(self, dom_node):
        self._node = dom_node
        self._name = self._node.nodeName
        for node in _get_child_elements(self._node):
            self.__dict__[node.nodeName] = _get_element_data(node)
    def save(self):
        for key in self.__dict__:
            if key.startswith('_'): continue
            _set_element( _get_element(self._node,key,1), str(self.__dict__[key]) )
    def debug_print(self):
        for key in self.__dict__.keys():
            print key.rjust(15), '=', self.__dict__[key]

def newConfigDict(name):
    return ConfigDict(xml.dom.minidom.Element(name))

class ConfigDictList(list):
    def __init__(self, dom_node=None, item_class=ConfigDict):
        self._node = dom_node
        list.__init__(self, map(item_class, _get_child_elements(self._node)))
    def save(self):
        map(ConfigDict.save, self)
    def append(self, cd):
        list.append(self, cd)
        self._node.appendChild(cd._node)
    def make_node_and_append(self, name):
        '''Make a new ConfigDict and append it. Returns new ConfigDict.'''
        cd = newConfigDict(name)
        self.append(cd)
        return cd

def newConfigDictList(name):
    return ConfigDictList(xml.dom.minidom.Element(name))

# base class for xml config files
class XMLConfig:
    def __init__(self, filename):
        self.filename = filename
    def read(self, failopen_ok=False):
        try:
            self.xml     = xml.dom.minidom.parse(self.filename)
        except IOError, e:
            if not failopen_ok:
                raise
            print >>sys.stderr, "Warning:", e
            # self.xml = xml.dom.minidom.Document()
            self.xml = xml.dom.minidom.parseString(self.default_xml)
        self._get_elements()
        return self
    def _get_elements(self):
        pass
    def write(self, output=None):
        self._set_elements()
        if not output:
            output = open(self.filename,'w')
        self.xml.writexml(output)
        print >>output
        return self
    def _set_elements(self):
        pass

# normal config file
class BoincConfig(XMLConfig):
    '''
    embodies config.xml
    Public attributes:
        config   - ConfigDict
        tasks    - list of ConfigDict elements
    '''
    def _get_elements(self):
        self.xml_boinc   = _get_element(self.xml,  'boinc')
        self.xml_config  = _get_element(self.xml_boinc, 'config')
        self.xml_tasks   = _get_element(self.xml_boinc, 'tasks')
        self.xml_daemons = _get_element(self.xml_boinc, 'daemons')
        self.config      = ConfigDict(self.xml_config)
        self.daemons     = ConfigDictList(self.xml_daemons)
        self.tasks       = ConfigDictList(self.xml_tasks)
    def _set_elements(self):
        self.config.save()
        self.daemons.save()
        self.tasks.save()
    def debug_print_all(self):
        '''print everything to stdout.'''

        print 'Debug dump of', self.filename
        print '-- parsed xml -------------------------------------------------------'
        self.xml.writexml(sys.stdout)
        print
        print '-- Enabled =', self.enabled
        print '-- Config -----------------------------------------------------------'
        self.config.debug_print()
        print
        print '-- Daemons ------------------------------------------------------------'
        for daemon in self.daemons:
            daemon.debug_print()
            print
        print
        print '-- Tasks ------------------------------------------------------------'
        for task in self.tasks:
            task.debug_print()
            print
    default_xml = '<boinc><config></config><daemons></daemons><tasks></tasks></boinc>'

# keeps BoincCron's timestamp status file
class BoincRunState(XMLConfig):
    '''
    embodies run_state.xml
    Public attributes:
       tasks - list of ConfigDict elements
       enabled - boolean
    '''
    def _get_elements(self):
        self.xml_boinc    = _get_element(self.xml,  'boinc')
        self.xml_tasks    = _get_element(self.xml_boinc, 'tasks')
        self.xml_enabled  = _get_element(self.xml_boinc, 'enabled', optional=1)
        self.tasks        = ConfigDictList(self.xml_tasks)
        self.enabled      = _get_element_int(self.xml_enabled)
    def _set_elements(self):
        _set_element( self.xml_enabled, self.enabled )
        self.tasks.save()
    default_xml = '<boinc><tasks></tasks></boinc>'

if __name__ == '__main__':
    config = BoincConfig('config.xml')
    config.read()
    # print "setting config.enabled = True"
    # config.enabled = True
    config.debug_print_all()
    print " -- saving xml and rewriting -----------------------------------------------"
    config.write(sys.stdout)
