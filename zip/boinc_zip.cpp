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
//
// NOTE: The libboinc_zip library is intended for use with libzip.
// On a Mac, you must download and build libzip, using the script
// mac_build/buildlibzip.sh

// Standard headers and platform config
#if defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#ifndef __APPLE_CC__
#include "config.h"
#endif
#endif

#include <algorithm>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <unistd.h>
#endif

// libzip
#include <zip.h>

using std::string;

#include "boinc_zip.h"
#include "filesys.h" // from BOINC for DirScan and helpers

// Helpers
namespace {
    // Return base filename component (no directories)
    static string basename_of(const string& path) {
        const size_t pos1 = path.find_last_of("/\\");
        if (pos1 == string::npos) return path;
        return path.substr(pos1+1);
    }

    // Join two path segments with '/'
    static string join_path(const string& a, const string& b) {
        if (a.empty()) return b;
        if (b.empty()) return a;
#ifdef _WIN32
        const char sep = '\\';
        if (a.back() == '/' || a.back() == '\\') return a + b;
#else
        const char sep = '/';
        if (a.back() == '/') return a + b;
#endif
        return a + sep + b;
    }

    // Ensure that the subdirectories for relative file "rel"
    // under base dir exist
    static int ensure_subdirs(const string& base_dir, const string& rel) {
        // Use BOINC helper to create nested dirs relative to base_dir
        return boinc_make_dirs(base_dir.c_str(), rel.c_str());
    }

#ifndef _WIN32
    // Set POSIX permissions from libzip external attributes when present
    static void apply_permissions_if_possible(const string& path, zip_t* za,
            zip_uint64_t idx) {
        zip_uint8_t opsys = 0;
        zip_uint32_t attributes = 0;
        if (zip_file_get_external_attributes(za, idx, 0, &opsys,
                    &attributes) == 0) {
            // For UNIX hosts, upper 16 bits contain st_mode
            if (opsys == ZIP_OPSYS_UNIX) {
                mode_t mode = (mode_t)((attributes >> 16) & 0777);
                if (mode) {
                    chmod(path.c_str(), mode);
                }
            }
        }
    }

    // When adding to archive, try to preserve original permission bits
    static void set_entry_permissions(zip_t* za, zip_uint64_t idx,
            const char* src_path) {
        (void)za; (void)idx; (void)src_path;
        struct stat sb;
        if (!stat(src_path, &sb)) {
            zip_file_set_external_attributes(za, idx, 0, ZIP_OPSYS_UNIX,
                    ((zip_uint32_t)sb.st_mode) << 16);
        }
    }
#endif
}

int boinc_zip(
    int bZipType, const std::string& szFileZip, const std::string& szFileIn
) {
    ZipFileList tempvec;
    tempvec.push_back(szFileIn);
    return boinc_zip(bZipType, szFileZip, &tempvec);
}

// same, but with char[] instead of string
//
int boinc_zip(int bZipType, const char* szFileZip, const char* szFileIn) {
    ZipFileList tempvec;
    tempvec.push_back(szFileIn);
    return boinc_zip(bZipType, szFileZip, &tempvec);
}

int boinc_zip(
    int bZipType, const std::string& szFileZip, const ZipFileList* pvectszFileIn
) {
    const size_t nVecSize = pvectszFileIn ? pvectszFileIn->size() : 0;

    if (bZipType == ZIP_IT) {
        // Create or truncate existing archive
        int errorp = 0;
        zip_t* za = zip_open(szFileZip.c_str(), ZIP_CREATE | ZIP_TRUNCATE,
                &errorp);
        if (!za) {
            return 2; // match previous nonzero error semantics
        }

        int retcode = 0;
        for (size_t jj = 0; jj < nVecSize; ++jj) {
            const string& src = pvectszFileIn->at(jj);
            // add as basename only (junk paths)
            string entry_name = basename_of(src);
            zip_source_t* zs = zip_source_file(za, src.c_str(), 0, -1);
            if (!zs) {
                retcode = 2;
                break;
            }
            const zip_int64_t idx = zip_file_add(za, entry_name.c_str(), zs,
                    ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
            if (idx < 0) {
                zip_source_free(zs);
                retcode = 2;
                break;
            }
#ifndef _WIN32
            // attempt to preserve permissions on non-Windows
            set_entry_permissions(za, (zip_uint64_t)idx, src.c_str());
#endif
        }

        if (zip_close(za) != 0) {
            retcode = 2;
        }
        return retcode;
    } else {
        // UNZIP: extract whole archive into CWD or provided directory
        // Destination directory is first (and only) element if provided
        string dest_dir = ".";
        if (nVecSize >= 1 && !pvectszFileIn->at(0).empty()) {
            dest_dir = pvectszFileIn->at(0);
        }

        // make sure zip file exists
#if defined(_WIN32)
        if (GetFileAttributesA(szFileZip.c_str()) == INVALID_FILE_ATTRIBUTES) {
            return 2;
        }
#else
        if (access(szFileZip.c_str(), F_OK) != 0) {
            return 2;
        }
#endif

        // Ensure destination directory exists
        boinc_mkdir(dest_dir.c_str());

        int errorp = 0;
        zip_t* za = zip_open(szFileZip.c_str(), ZIP_RDONLY, &errorp);
        if (!za) return 2;

        const zip_int64_t num_entries_result = zip_get_num_entries(za, 0);
        if (num_entries_result < 0) {
            zip_discard(za);
            return 2;
        }
        const zip_uint64_t num_entries =
            static_cast<zip_uint64_t>(num_entries_result);
        for (zip_uint64_t i = 0; i < num_entries; ++i) {
            struct zip_stat zs;
            zip_stat_init(&zs);
            if (zip_stat_index(za, i, 0, &zs) != 0) {
                zip_discard(za);
                return 2;
            }
            string name = zs.name ? string(zs.name) : string();
            if (name.empty()) continue;

            // Directory entry (convention: ends with '/')
            if (name.back() == '/' || name.back() == '\\') {
                ensure_subdirs(dest_dir, name);
                continue;
            }

            // Ensure parent directories exist
            ensure_subdirs(dest_dir, name);

            // Open source entry
            zip_file_t* zf = zip_fopen_index(za, i, 0);
            if (!zf) {
                zip_discard(za);
                return 2;
            }

            string out_path = join_path(dest_dir, name);
            FILE* out = boinc_fopen(out_path.c_str(), "wb");
            if (!out) {
                zip_fclose(zf);
                zip_discard(za);
                return 2;
            }

            const size_t BUF_SIZE = 64 * 1024;
            std::vector<unsigned char> buf(BUF_SIZE);
            while (1) {
                const zip_int64_t n = zip_fread(zf, buf.data(), BUF_SIZE);
                if (n < 0) {
                    // read error
                    zip_fclose(zf);
                    boinc::fclose(out);
                    zip_discard(za);
                    return 2;
                }
                if (n == 0) break;
                const size_t read = static_cast<size_t>(n);
                const size_t m = boinc::fwrite(buf.data(), 1, read, out);
                if (m != read) {
                    zip_fclose(zf);
                    boinc::fclose(out);
                    zip_discard(za);
                    return 2;
                }
            }
            boinc::fclose(out);
            zip_fclose(zf);
#ifndef _WIN32
            // Try to restore permissions if present
            apply_permissions_if_possible(out_path, za, i);
#endif
        }

        if (zip_close(za) != 0) {
            return 2;
        }
        return 0;
    }
}


// -------------------------------------------------------------------
//
// Description: Supply a directory and a pattern as arguments, along
//    with a FileList variable to hold the result list; sorted by name.
//    Returns a vector of files in the directory which match the
//    pattern.  Returns true for success, or false if there was a problem.
//
// CMC Note: this is a 'BOINC-friendly' implementation of "old" CPDN code
//    the "wildcard matching"/regexp is a bit crude, it matches substrings in
//    order in a file; to match all files such as *.pc.*.x4.*.nc" pass
//        ".pc|.x4.|.nc" for 'pattern'
//
// --------------------------------------------------------------------

bool boinc_filelist(
    const string& directory,
    const string& pattern,
    ZipFileList* pList,
    const unsigned char ucSort,
    const bool bClear
) {
    string spattern = pattern;
    string strDir = directory;
    string strUserDir = directory;

    if (!pList) return false;

    // wildcards are blank!
    if (pattern == "*" || pattern == "*.*") spattern.assign("");

    if (bClear) {
        pList->clear();  // removes old entries that may be in pList
    }

    // first tack on a final slash on user dir if required
    if (strUserDir.back() != '\\' && strUserDir.back() != '/') {
        // need a final slash, but what type?
        // / is safe on all OS's for CPDN at least
        // but if they already used \ use that
        // well they didn't use a backslash so just use a slash
        if (strUserDir.find("\\") == string::npos) {
#ifdef _WIN32
            strUserDir += "\\";
#else
            strUserDir += "/";
#endif
        } else {
            strUserDir += "\\";
        }
    }

    // transform strDir to either all \\ or all /
    for (char& c : strDir)  {
#ifdef _WIN32  // transform paths appropriate for OS
        if (c == '/') c = '\\';
#else
        if (c == '\\') c = '/';
#endif
    }
    // take off final / or backslash
    if (strDir.back() == '\\' || strDir.back() == '/') {
        strDir.pop_back();
    }

    DirScanner dirscan(strDir);
    string strFile;
    while (dirscan.scan(strFile)) {
        size_t lastPos = 0;
        std::vector<string> strPart;
        size_t iPos = string::npos;
        while ((iPos = spattern.find('|', lastPos)) != string::npos) {
            strPart.push_back(spattern.substr(lastPos, iPos - lastPos));
            lastPos = iPos+1;
        }
        if (strPart.size() > 0) {
            // found a | so need to get the part from lastpos onward
            strPart.push_back(spattern.substr(lastPos));
        }

        // check no | were found at all
        if (strPart.size() == 0) {
            strPart.push_back(spattern);
        }

        size_t iFnd = 0;
        bool bFound = false;
        for (const string& s : strPart) {
            iFnd = strFile.find(s, iFnd);
            if (iFnd == string::npos) {
                bFound = false;
                break;
            }
            bFound = true;
            iFnd += s.length(); // move forward for next search
        }

        if (bFound) {
            // this pattern matched the file, add to vector
            // NB: first get stat to make sure it really is a file
            string strFullPath = strUserDir + strFile;
            // only add if the file really exists (i.e. not a directory)
            if (is_file(strFullPath.c_str())) {
                pList->push_back(strFullPath);
            }
        }
    }

    // sort by file creation time
    // sort if list is greather than 1
    if (pList->size()>1) {
        sort(pList->begin(), pList->end(),
               [&](const string& first, const string& second)->bool {
                        if (ucSort & SORT_NAME
                            && ucSort & SORT_ASCENDING
                            && first < second) {
                            return true;
                        } else if (ucSort & SORT_NAME
                            && ucSort & SORT_DESCENDING
                            && first > second) {
                            return true;
                        } else if (ucSort & SORT_TIME) {
                            struct stat st[2];
                            stat(first.c_str(), &st[0]);
                            stat(second.c_str(), &st[1]);
                            if (ucSort & SORT_ASCENDING) {
                                return st[0].st_mtime < st[1].st_mtime;
                            } else {
                                return st[0].st_mtime > st[1].st_mtime;
                            }
                        }
                        return false;
               });
    }
    return true;
}

// Read compressed file to memory.
//
int boinc_UnzipToMemory (char *zip_path, char *file, string &retstr) {
    retstr.clear();
    if (!zip_path || !file || !*file) {
        return 0; // indicate error (tests expect != 1)
    }

    int errorp = 0;
    zip_t* za = zip_open(zip_path, ZIP_RDONLY, &errorp);
    if (!za) return 0;

    const zip_int64_t idx = zip_name_locate(za, file, 0);
    if (idx < 0) {
        zip_close(za);
        return 0;
    }

    zip_file_t* zf = zip_fopen_index(za, static_cast<zip_uint64_t>(idx), 0);
    if (!zf) {
        zip_close(za);
        return 0;
    }

    // Read full file content
    const size_t BUF_SIZE = 64 * 1024;
    std::vector<char> buf(BUF_SIZE);
    string out;
    while (1) {
        const zip_int64_t n = zip_fread(zf, buf.data(), BUF_SIZE);
        if (n < 0) {
            zip_fclose(zf);
            zip_close(za);
            return 0;
        }
        if (n == 0) break;
        out.append(buf.data(), static_cast<size_t>(n));
    }

    zip_fclose(zf);
    zip_close(za);

    retstr = out;
    return 1; // success expected by tests
}

