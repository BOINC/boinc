// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


// "wrapper" for the zip/unzip functions to expose to BOINC clients
// CMC - 18/03/2004, Oxford University for BOINC project

// note that I've disabled zip encryption to try and simplify things
// (zip encryption is fairly weak and easy to break anyway)

// you will need to link against the library (boinc_zip.lib on Windows-Release,
// boinc_zipd.lib for Windows-Debug, or libboinc_zip.a on Unix & Mac OSX)

// also you will need to include filesys.cpp in your project
// (decided not to add that in the lib due to version problems that may arise)

#ifdef __cplusplus
#include <vector>
#include <string>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#define ZIP_IT   1
#define UNZIP_IT 0

// bitmasks for sort type on the ZipFileList 
// optional but CPDN SmallExecs likes reverse date order
#define SORT_ASCENDING    0x01
#define SORT_DESCENDING   0x02

#define SORT_TIME         0x10
#define SORT_NAME         0x20

#ifdef __cplusplus
using std::string;

typedef std::vector<std::string> ZipFileList;

// forward declarations for boinc_zip functions
// note it's basically like running zip/unzip, just comprise an argc/argv
// send in an input file or path wildcard, output filename, and basic options

// default options for zip (bZip = true) are "-j9q" which is 
// DON'T recurse/list subdirectories, best compression, quiet operation
// call it with bZiptype = ZIP_IP to zip, bZip = UNZIP_IT to unzip (duh)

// note boinc_zip is overloaded for ease of use
// i.e. you can just send char filenames, or std::strings, or preferably
// a vector of std::string of filenames to add to the zip archive

// there is also a crude but handy wildcard matching function to build
// the vector of string's, and probably better than using wildcards in zip
// across platforms

bool boinc_filelist(
    const std::string directory,
    const std::string pattern,
    ZipFileList* pList, 
    const unsigned char ucSort = SORT_NAME | SORT_DESCENDING,
    const bool bClear = true
);

int boinc_zip(
    int bZipType,
    const std::string szFileZip,
    const ZipFileList* pvectszFileIn
);
int boinc_zip(
    int bZipType,
    const std::string szFileZip,
    const std::string szFileIn
);

int boinc_UnzipToMemory ( char *zip, char *file, std::string &retstr );
extern "C"
#else
extern
#endif
int boinc_zip(int bZipType, const char* szFileZip, const char* szFileIn);
