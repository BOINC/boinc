#!/usr/bin/env python

# boincxml.py - XML utilities for boinc

from __future__ import print_function
import sys, os
import xml.dom.minidom

def append_new_element(parent_node, name):
    new_element = xml.dom.minidom.Element(name)
    if isinstance(parent_node,xml.dom.minidom.Document):
        new_element.ownerDocument = parent_node
    else:
        assert(parent_node.ownerDocument)
        new_element.ownerDocument = parent_node.ownerDocument
    parent_node.appendChild(new_element)
    return new_element

def get_elements(node, name):
    return node.getElementsByTagName(name)

def get_element(node, name, optional=True):
    try:
        return get_elements(node,name)[0]
    except IndexError:
        if optional:
            return append_new_element(node, name)
        raise SystemExit("ERROR: Couldn't find xml node <%s>"% name)

def _None2Str(object):
    if object is None:
        return ''
    else:
        return object

def get_element_data(node):
    return node and _None2Str(node.firstChild and node.firstChild.data)

def get_element_int(node, default=0):
    try:
        return int(get_element_data(node))
    except:
        return default

def get_child_elements(node):
    return filter(lambda node: node.nodeType == node.ELEMENT_NODE, node.childNodes)

def set_element(node, new_data):
    if node.firstChild and node.firstChild.data:
        node.firstChild.data = str(new_data)
    else:
        # different versions of pyxml have different interface (!)
        try:
            new_data_node = xml.dom.minidom.Text()
        except:
            new_data_node = xml.dom.minidom.Text('')
        new_data_node.data = str(new_data)
        new_data_node.ownerDocument = node.ownerDocument
        node.appendChild(new_data_node)

def strip_white_space(node):
    ''' strip white space from text nodes, and remove empty nodes. '''
    # the [:] below is to copy the list since removing children modifies the
    # list.
    for child in node.childNodes[:]:
        if child.nodeType == child.TEXT_NODE:
            child.data = child.data.strip()
            if not child.data:# and (child.nextSibling or child.previousSibling):
                node.removeChild(child)
        else:
            strip_white_space(child)

def get_elements_as_dict(node):
    dict = {}
    for child in get_child_elements(node):
        # note: str() changes from unicode to single-byte strings
        dict[str(child.nodeName)] = get_element_data(child)
    return dict

class ConfigDict:
    def __init__(self, dom_node):
        self._node = dom_node
        self._name = self._node.nodeName
        self.__dict__.update(get_elements_as_dict(self._node))
    def save(self):
        for key in self.__dict__:
            if key.startswith('_'): continue
            set_element( get_element(self._node,key,1), str(self.__dict__[key]) )
    def debug_print(self):
        for key in self.__dict__.keys():
            print(key.rjust(15), '=', self.__dict__[key])

class ConfigDictList(list):
    def __init__(self, dom_node, item_class=ConfigDict):
        self._node = dom_node
        list.__init__(self, map(item_class, get_child_elements(self._node)))
    def save(self):
        for item in self:
            ConfigDict.save(item)
    def make_node_and_append(self, name):
        '''Make a new ConfigDict and append it. Returns new ConfigDict.'''
        new_element = append_new_element(self._node, name)
        new_cd      = ConfigDict(new_element)
        self.append(new_cd)
        return new_cd
    def remove_node(self, item):
        self.remove(item)
        self._node.removeChild(item._node)

class XMLConfig:
    '''Base class for xml config files'''
    def __init__(self, filename = None):
        # default_filename should be defined by children
        self.filename = filename or self.default_filename
    def _init_empty_xml(self):
        self.xml = xml.dom.minidom.parseString(self.default_xml)
    def init_empty(self):
        self._init_empty_xml()
        self._get_elements()
        return self
    def read(self, failopen_ok=False):
        """Read the XML object's file source and return self."""
        if failopen_ok and not os.path.exists(self.filename):
            self._init_empty_xml()
        else:
            try:
                self.xml = xml.dom.minidom.parse(self.filename)
                strip_white_space(self.xml)
            except:
                print("Warning: couldn't parse XML file:", self.filename, file=sys.stderr)
                if not failopen_ok:
                    raise Exception("Couldn't parse XML file\n")
                self._init_empty_xml()
        try:
            self._get_elements()
        except:
            if not failopen_ok:
                raise Exception("%s: Couldn't get elements from XML file")
        return self
    def _get_elements(self):
        pass
    def write(self, output=None):
        """Write XML data to filename, or other stream."""
        self._set_elements()
        if not output:
            output = open(self.filename,'w')
        self.xml.writexml(output, "", " "*4, "\n")
        output.write('')
        return self
    def _set_elements(self):
        pass
