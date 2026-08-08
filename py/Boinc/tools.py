## $Id$

from __future__ import print_function
from Boinc import configxml

try:
    # use new hashlib if available
    from hashlib import md5
except:
    import md5
import os, shutil, binascii, filecmp

# from http://www.plope.com/software/uuidgen/view
_urandomfd = None
def urandom(n):
    """urandom(n) -> str

    Return a string of n random bytes suitable for cryptographic use.

    """
    global _urandomfd
    if _urandomfd is None:
        try:
            _urandomfd = os.open("/dev/urandom", os.O_RDONLY)
        except:
            _urandomfd = NotImplementedError
    if _urandomfd is NotImplementedError:
        raise NotImplementedError("/dev/urandom (or equivalent) not found")
    bytes = ""
    while len(bytes) < n:
        bytes += os.read(_urandomfd, n - len(bytes))
    return bytes

def make_uuid():
    return binascii.hexlify(urandom(16))

def md5_file(path):
    """
    Return a 16-digit MD5 hex digest of a file's contents
    Read the file in chunks
    """

    chunk = 8096

    try:
        checksum = md5()
    except TypeError:
        checksum = md5.new()

    fp = open(path, 'r')
    while True:
        buffer = fp.read(chunk)
        if not buffer:
            break
        checksum.update(buffer)

    fp.close()

    return checksum

def file_size(path):
    """Return the size of a file"""
    f = open(path)
    f.seek(0,2)
    return f.tell()

# workaround for dubious function change in python3
#
from sys import version_info
def input_aux():
    if version_info.major == 2:
        return raw_input()
    return input()

def query_yesno(str):
    '''Query user; default Yes'''
    print (str, "[Y/n] ", end="")
    return not input_aux().strip().lower().startswith('n')

def query_noyes(str):
    '''Query user; default No'''
    print (str, "[y/N] ", end="")
    return input_aux().strip().lower().startswith('y')

def get_output_file_path(filename):
    """ Return the filename's path in the upload directory
        Use this if you're developing a validator/assimilator in Python
    """
    config = configxml.default_config()
    fanout = long(config.config.uldl_dir_fanout)
    try:
        s = md5(filename).hexdigest()[1:8]
    except TypeError:
        s = md5.new(filename).hexdigest()[1:8]
    x = long(s, 16)
    return "%s/%x/%s" % (config.config.upload_dir, x % fanout, filename)
