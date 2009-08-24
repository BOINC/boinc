from distutils.core import setup, Extension

# May need to adjust these to suit your system, but a standard installation of the BOINC client from source will put things in these places
pyboinc = Extension('boinc', runtime_library_dirs=['/usr/local/lib', '../../lib', '../../api'],
        extra_objects=['../../lib/libboinc.a', '../../api/libboinc_api.a'],
        sources = ['boincmodule.C'],
        libraries=['stdc++', 'boinc', 'boinc_api', 'dl','crypto','ssl'],
        library_dirs=['/usr/local/lib/', '../../lib', '../../api'],
        include_dirs=['/usr/local/include/boinc/', '../../lib', '../../api', '../../'],
        extra_compile_args=['-fPIC'],extra_link_args=['-fPIC'])

setup (name = 'PyBoinc',
       version = '0.1',
       description = 'Basic python bindings for BOINC network computing package',
       ext_modules = [pyboinc])
