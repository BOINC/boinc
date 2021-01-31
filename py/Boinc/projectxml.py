#!/usr/bin/env python

# projectxml.py - module to read and parse project.xml

'''
SYNOPSIS:  parses and writes project.xml
USAGE:     from Boinc import projectxml
           project = projectxml.ProjectFile().read()
           project.commit_all()
'''

import sys
from Boinc import boinc_project_path
from Boinc.boincxml import *
from Boinc.add_util import *

default_project_file = None

class ProjectFile(XMLConfig):
    '''
    embodies project.xml
    '''
    default_filename = boinc_project_path.project_xml_filename
    def __init__(self, *args, **kwargs):
        XMLConfig.__init__(self, *args, **kwargs)
        global default_project_file
        default_project_file = self
    def _get_elements(self):
        self.xml_boinc   = get_element(self.xml, 'boinc', optional=False)
        self.elements = ConfigDictList(self.xml_boinc)
        self.add_objects_and_args = []
        for node in self.xml_boinc.childNodes:
            add_object = add_objects.get(node.nodeName)
            if not add_object:
                raise SystemExit("Error in %s: No such object '%s' to add." %(self.filename,node.nodeName))
            self.add_objects_and_args.append((add_object, get_elements_as_dict(node)))
    def _set_elements(self):
        self.elements.save()
    def commit_all(self):
        '''Commits all new data to the BOINC project database.'''
        for add_object, untranslated_args_dict in self.add_objects_and_args:
            try:
                do_add_object(add_object, untranslated_args_dict, skip_old=True)
            except AddObjectException as e:
                raise SystemExit('Error in %s: %s' %(self.filename,e))
    default_xml = '<boinc></boinc>'

def default_project():
    '''If any project file has been read, return it.  Else open the default one
    and return it.'''
    if not default_project_file: ProjectFile().read()
    assert(default_project_file)
    return default_project_file
