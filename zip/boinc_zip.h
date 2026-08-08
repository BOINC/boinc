// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

#ifdef __cplusplus
#include <vector>
#include <string>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#define ZIP_IT   1
#define UNZIP_IT 0

#define SORT_ASCENDING    0x01
#define SORT_DESCENDING   0x02

#define SORT_TIME         0x10
#define SORT_NAME         0x20

#ifdef __cplusplus
using std::string;

typedef std::vector<std::string> ZipFileList;

bool boinc_filelist(
    const std::string& directory,
    const std::string& pattern,
    ZipFileList* pList,
    const unsigned char ucSort = SORT_NAME | SORT_DESCENDING,
    const bool bClear = true
);

int boinc_zip(
    int bZipType,
    const std::string& szFileZip,
    const ZipFileList* pvectszFileIn
);
int boinc_zip(
    int bZipType,
    const std::string& szFileZip,
    const std::string& szFileIn
);

int boinc_UnzipToMemory ( char *zip, char *file, std::string &retstr );
extern "C"
#else
extern
#endif
int boinc_zip(int bZipType, const char* szFileZip, const char* szFileIn);
