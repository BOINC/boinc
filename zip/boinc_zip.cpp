// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2019 University of California
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


#if defined(_WIN32) && !defined(__STDWX_H__)
#pragma warning( disable : 4786 )  // Disable warning messages for vector
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#ifndef __APPLE_CC__
#include "config.h"
#endif
#include <algorithm>
#include <string>
#include <string.h>
using std::string;

#endif

#include <zlib.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdint.h>

#include "boinc_zip.h"
#include "filesys.h" // from BOINC for DirScan

// send in an output filename, advanced options (usually NULL), and numFileIn, szfileIn

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

unsigned char g_ucSort;

// a "binary predicate" for use by the std::sort algorithm
// return true if "first > second" according to the g_ucSort type

bool StringVectorSort(const std::string& first, const std::string& second) {
    bool bRet = false;
    if (g_ucSort & SORT_NAME
        && g_ucSort & SORT_ASCENDING
        && strcmp(first.c_str(), second.c_str())<0
    ) {
        bRet = true;
    } else if (g_ucSort & SORT_NAME
        && g_ucSort & SORT_DESCENDING
        && strcmp(first.c_str(), second.c_str())>0
    ) {
        bRet = true;
    } else if (g_ucSort & SORT_TIME) {
        struct stat st[2];
        stat(first.c_str(), &st[0]);
        stat(second.c_str(), &st[1]);
        if (g_ucSort & SORT_ASCENDING) {
            bRet = st[0].st_mtime < st[1].st_mtime;
        } else {
            bRet = st[0].st_mtime > st[1].st_mtime;
        }
    }
    return bRet;
}

int boinc_zip(
    int bZipType, const std::string szFileZip, const std::string szFileIn
) {
    ZipFileList tempvec;
    tempvec.push_back(szFileIn);
    return boinc_zip(bZipType, szFileZip, &tempvec);
}

// same, but with char[] instead of string
//
int boinc_zip(int bZipType, const char* szFileZip, const char* szFileIn) {
    string strFileZip, strFileIn;
    strFileZip.assign(szFileZip);
    strFileIn.assign(szFileIn);
    ZipFileList tempvec;
    tempvec.push_back(strFileIn);
    return boinc_zip(bZipType, strFileZip, &tempvec);
}

// --- Minimal ZIP (deflate) writer/reader using zlib (no Info-ZIP) ---
namespace {
    inline void write_le16(std::ostream& os, uint16_t v) {
        unsigned char b[2] = { (unsigned char)(v & 0xFF), (unsigned char)((v >> 8) & 0xFF) };
        os.write((const char*)b, 2);
    }
    inline void write_le32(std::ostream& os, uint32_t v) {
        unsigned char b[4] = {
            (unsigned char)(v & 0xFF),
            (unsigned char)((v >> 8) & 0xFF),
            (unsigned char)((v >> 16) & 0xFF),
            (unsigned char)((v >> 24) & 0xFF)
        };
        os.write((const char*)b, 4);
    }
    inline uint16_t read_le16(std::istream& is) {
        unsigned char b[2]; is.read((char*)b, 2);
        return (uint16_t)(b[0] | (b[1] << 8));
    }
    inline uint32_t read_le32(std::istream& is) {
        unsigned char b[4]; is.read((char*)b, 4);
        return (uint32_t)(b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24));
    }

    std::string base_name(const std::string& path) {
        size_t p = path.find_last_of("/\\");
        if (p == std::string::npos) return path;
        return path.substr(p+1);
    }

    bool deflate_raw(const std::string& input, std::vector<unsigned char>& out, int level, uint32_t& out_crc) {
        out_crc = crc32(0L, Z_NULL, 0);
        out_crc = crc32(out_crc, (const Bytef*)input.data(), (uInt)input.size());

        z_stream strm{};
        int ret = deflateInit2(&strm, level, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
        if (ret != Z_OK) return false;
        strm.next_in = (Bytef*)input.data();
        strm.avail_in = (uInt)input.size();
        out.clear();
        out.reserve(deflateBound(&strm, (uLong)input.size()));
        unsigned char buf[16384];
        do {
            strm.next_out = buf;
            strm.avail_out = sizeof(buf);
            ret = deflate(&strm, strm.avail_in ? Z_NO_FLUSH : Z_FINISH);
            if (ret == Z_STREAM_ERROR) { deflateEnd(&strm); return false; }
            size_t produced = sizeof(buf) - strm.avail_out;
            out.insert(out.end(), buf, buf + produced);
        } while (ret != Z_STREAM_END);
        deflateEnd(&strm);
        return true;
    }

    bool inflate_raw(const unsigned char* input, size_t in_size, std::string& out, size_t expected_size) {
        z_stream strm{};
        int ret = inflateInit2(&strm, -MAX_WBITS);
        if (ret != Z_OK) return false;
        strm.next_in = (Bytef*)input;
        strm.avail_in = (uInt)in_size;
        out.clear();
        out.reserve(expected_size ? expected_size : in_size * 2);
        unsigned char buf[16384];
        do {
            strm.next_out = buf;
            strm.avail_out = sizeof(buf);
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR) { inflateEnd(&strm); return false; }
            size_t produced = sizeof(buf) - strm.avail_out;
            out.append((const char*)buf, produced);
        } while (ret != Z_STREAM_END);
        inflateEnd(&strm);
        return true;
    }

    struct CDEntry {
        std::string name;
        uint32_t crc;
        uint32_t comp_size;
        uint32_t uncomp_size;
        uint16_t method;
        uint32_t local_header_offset;
        uint16_t mod_time;
        uint16_t mod_date;
    };

    // Parse central directory into entries; return false on failure
    bool parse_central_dir(std::istream& is, std::vector<CDEntry>& entries, uint32_t& cd_offset) {
        // Find EOCD by scanning last 64k+22 bytes
        is.seekg(0, std::ios::end);
        std::streamoff file_size = is.tellg();
        std::streamoff max_back = std::min<std::streamoff>(file_size, 0xFFFF + 22);
        std::vector<char> tail((size_t)max_back);
        is.seekg(file_size - max_back, std::ios::beg);
        is.read(tail.data(), tail.size());
        int found = -1;
        for (int i = (int)tail.size() - 22; i >= 0; --i) {
            if ((unsigned char)tail[i] == 0x50 && (unsigned char)tail[i+1] == 0x4b &&
                (unsigned char)tail[i+2] == 0x05 && (unsigned char)tail[i+3] == 0x06) {
                found = i; break;
            }
        }
        if (found < 0) return false;
        std::istringstream eocd(std::string(tail.data() + found, tail.size() - found));
        uint32_t sig = read_le32(eocd);
        (void)sig;
        (void)read_le16(eocd); // disk no
        (void)read_le16(eocd); // disk start
        uint16_t total_entries_disk = read_le16(eocd);
        uint16_t total_entries = read_le16(eocd);
        uint32_t cd_size = read_le32(eocd);
        cd_offset = read_le32(eocd);
        (void)read_le16(eocd); // comment len
        (void)total_entries_disk;
        (void)cd_size;
        // Read central directory
        is.seekg(cd_offset, std::ios::beg);
        entries.clear();
        for (uint16_t idx = 0; idx < total_entries; ++idx) {
            if (read_le32(is) != 0x02014b50) return false;
            (void)read_le16(is); // ver made by
            (void)read_le16(is); // ver needed
            uint16_t gp = read_le16(is);
            uint16_t method = read_le16(is);
            uint16_t mtime = read_le16(is);
            uint16_t mdate = read_le16(is);
            uint32_t crc = read_le32(is);
            uint32_t csize = read_le32(is);
            uint32_t usize = read_le32(is);
            uint16_t nlen = read_le16(is);
            uint16_t xlen = read_le16(is);
            uint16_t clen = read_le16(is);
            (void)read_le16(is); // disk no
            (void)read_le16(is); // internal attr
            (void)read_le32(is); // external attr
            uint32_t lhoff = read_le32(is);
            std::string name(nlen, '\0');
            is.read(&name[0], nlen);
            if (xlen) is.seekg(xlen, std::ios::cur);
            if (clen) is.seekg(clen, std::ios::cur);
            (void)gp;
            CDEntry e{ name, crc, csize, usize, method, lhoff, mtime, mdate };
            entries.push_back(e);
        }
        return true;
    }
}

int boinc_zip(
    int bZipType, const std::string szFileZip, const ZipFileList* pvectszFileIn
) {
    if (bZipType == ZIP_IT) {
        if (!pvectszFileIn || pvectszFileIn->empty()) return 0;
        // Remove existing to mimic previous behavior
        if (access(szFileZip.c_str(), 0) == 0) unlink(szFileZip.c_str());
        std::ofstream os(szFileZip.c_str(), std::ios::binary);
        if (!os) return 1;
        struct Item { CDEntry cd; std::vector<unsigned char> comp; std::string name; };
        std::vector<Item> items;
        for (size_t i = 0; i < pvectszFileIn->size(); ++i) {
            const std::string& path = pvectszFileIn->at(i);
            std::ifstream is(path.c_str(), std::ios::binary);
            if (!is) return 1;
            std::ostringstream ss; ss << is.rdbuf();
            std::string data = ss.str();
            uint32_t crc=0; std::vector<unsigned char> comp;
            if (!deflate_raw(data, comp, Z_BEST_COMPRESSION, crc)) return 1;
            Item it;
            it.cd.name = base_name(path); // mimic -j option (no dir names)
            it.cd.crc = crc;
            it.cd.comp_size = (uint32_t)comp.size();
            it.cd.uncomp_size = (uint32_t)data.size();
            it.cd.method = 8; // deflate
            it.cd.mod_time = 0; it.cd.mod_date = 0;
            it.comp.swap(comp);
            it.name = it.cd.name;
            items.push_back(std::move(it));
        }
        // Write local headers and data
        for (auto& it : items) {
            it.cd.local_header_offset = (uint32_t)os.tellp();
            write_le32(os, 0x04034b50);
            write_le16(os, 20); // version needed
            write_le16(os, 0);  // gp flag
            write_le16(os, it.cd.method);
            write_le16(os, it.cd.mod_time);
            write_le16(os, it.cd.mod_date);
            write_le32(os, it.cd.crc);
            write_le32(os, it.cd.comp_size);
            write_le32(os, it.cd.uncomp_size);
            write_le16(os, (uint16_t)it.cd.name.size());
            write_le16(os, 0); // extra len
            os.write(it.cd.name.data(), it.cd.name.size());
            if (!it.comp.empty()) os.write((const char*)it.comp.data(), it.comp.size());
        }
        // Central directory
        uint32_t cd_offset = (uint32_t)os.tellp();
        for (auto& it : items) {
            write_le32(os, 0x02014b50);
            write_le16(os, 20); // ver made by
            write_le16(os, 20); // ver needed
            write_le16(os, 0);  // gp flag
            write_le16(os, it.cd.method);
            write_le16(os, it.cd.mod_time);
            write_le16(os, it.cd.mod_date);
            write_le32(os, it.cd.crc);
            write_le32(os, it.cd.comp_size);
            write_le32(os, it.cd.uncomp_size);
            write_le16(os, (uint16_t)it.cd.name.size());
            write_le16(os, 0); // extra
            write_le16(os, 0); // comment
            write_le16(os, 0); // disk no
            write_le16(os, 0); // internal attr
            write_le32(os, 0); // external attr
            write_le32(os, it.cd.local_header_offset);
            os.write(it.cd.name.data(), it.cd.name.size());
        }
        uint32_t cd_size = (uint32_t)os.tellp() - cd_offset;
        // EOCD
        write_le32(os, 0x06054b50);
        write_le16(os, 0); // disk
        write_le16(os, 0); // disk start
        write_le16(os, (uint16_t)items.size());
        write_le16(os, (uint16_t)items.size());
        write_le32(os, cd_size);
        write_le32(os, cd_offset);
        write_le16(os, 0); // comment
        return 0;
    } else {
        // UNZIP
        if (access(szFileZip.c_str(), 0) != 0) return 2;
        std::ifstream is(szFileZip.c_str(), std::ios::binary);
        if (!is) return 2;
        std::vector<CDEntry> entries; uint32_t cd_off=0;
        if (!parse_central_dir(is, entries, cd_off)) return 1;
        std::string dest;
        if (pvectszFileIn && pvectszFileIn->size() == 1) dest = pvectszFileIn->at(0);
        // Ensure dest ends with separator if provided and not empty
        if (!dest.empty()) {
            char last = dest.back();
            if (last != '/' && last != '\\') dest += '/';
        }
        for (auto& e : entries) {
            // Read local header
            is.seekg(e.local_header_offset, std::ios::beg);
            if (read_le32(is) != 0x04034b50) return 1;
            (void)read_le16(is); // ver needed
            uint16_t gp = read_le16(is);
            uint16_t method = read_le16(is);
            (void)read_le16(is); // time
            (void)read_le16(is); // date
            uint32_t crc = read_le32(is);
            uint32_t csize = read_le32(is);
            uint32_t usize = read_le32(is);
            uint16_t nlen = read_le16(is);
            uint16_t xlen = read_le16(is);
            std::string lname(nlen, '\0');
            is.read(&lname[0], nlen);
            if (xlen) is.seekg(xlen, std::ios::cur);
            (void)gp; (void)crc; // not strictly validated here
            std::vector<unsigned char> comp(csize);
            if (csize) is.read((char*)comp.data(), csize);
            std::string out;
            if (method == 0) {
                out.assign((const char*)comp.data(), comp.size());
            } else if (method == 8) {
                if (!inflate_raw(comp.data(), comp.size(), out, usize)) return 1;
            } else {
                return 1; // unsupported
            }
            // Write file
            std::string outPath = dest + lname;
            std::ofstream of(outPath.c_str(), std::ios::binary);
            if (!of) return 1;
            of.write(out.data(), out.size());
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
    const string directory,
    const string pattern,
    ZipFileList* pList,
    const unsigned char ucSort,
    const bool bClear
) {
    g_ucSort = ucSort;  // set the global sort type right off the bat
    string strFile;
    // at most three |'s may be passed in pattern match
    int iPos[3], iFnd, iCtr, i, lastPos;
    string strFullPath;
    char strPart[3][32];
    string spattern = pattern;
    string strDir = directory;
    string strUserDir = directory;
    int iLen = strUserDir.size();

    if (!pList) return false;

    // wildcards are blank!
    if (pattern == "*" || pattern == "*.*") spattern.assign("");

    if (bClear) {
        pList->clear();  // removes old entries that may be in pList
    }

    // first tack on a final slash on user dir if required
    // fix off-by-one: check the last valid character at iLen-1 (not iLen)
    // also guard against empty directory string
    if (iLen == 0 || (strUserDir[iLen-1] != '\\' && strUserDir[iLen-1] != '/')) {
        // need a final slash, but what type?
        // / is safe on all OS's for CPDN at least
        // but if they already used \ use that
        // well they didn't use a backslash so just use a slash
        if (strUserDir.find("\\") == string::npos) {
            strUserDir += "/";
        } else {
            strUserDir += "\\";
        }
    }

    // transform strDir to either all \\ or all /
    int j;
    for (j=0; j<(int)directory.size(); j++)  {
        // take off final / or backslash
        if (j == ((int)directory.size()-1)
             && (strDir[j] == '/' || strDir[j]=='\\')
        ) {
            strDir.resize(directory.size()-1);
        } else {
#ifdef _WIN32  // transform paths appropriate for OS
           if (directory[j] == '/') strDir[j] = '\\';
#else
           if (directory[j] == '\\') strDir[j] = '/';
#endif
        }
    }

    // Normalize user directory separators to match OS style so returned
    // full paths have expected separators (e.g., backslashes on Windows).
#ifdef _WIN32
    for (j = 0; j < (int)strUserDir.size(); j++) {
        if (strUserDir[j] == '/') strUserDir[j] = '\\';
    }
#else
    for (j = 0; j < (int)strUserDir.size(); j++) {
        if (strUserDir[j] == '\\') strUserDir[j] = '/';
    }
#endif

    DirScanner dirscan(strDir);
    memset(strPart, 0x00, 3*32);
    while (dirscan.scan(strFile)) {
        iCtr = 0;
        lastPos = 0;
        iPos[0] = -1;
        iPos[1] = -1;
        iPos[2] = -1;
        // match the filename against the regexp to see if it's a hit
        // first get all the |'s to get the pieces to verify
        //
        while (iCtr<3 && (iPos[iCtr] = (int) spattern.find('|', lastPos)) > -1) {
            if (iCtr==0)  {
                strncpy(strPart[0], spattern.c_str(), iPos[iCtr]);
            } else {
                strncpy(strPart[iCtr], spattern.c_str()+lastPos, iPos[iCtr]-lastPos);
            }
            lastPos = iPos[iCtr]+1;
            iCtr++;
        }
        if (iCtr>0) {
            // found a | so need to get the part from lastpos onward
            strncpy(strPart[iCtr], spattern.c_str()+lastPos, spattern.length() - lastPos);
        }

        // check no | were found at all
        if (iCtr == 0) {
            strcpy(strPart[0], spattern.c_str());
            iCtr++; // fake iCtr up 1 to get in the loop below
        }

        bool bFound = true;
        for (i = 0; i <= iCtr && bFound; i++) {
            if (i==0)  {
                iFnd = (int) strFile.find(strPart[0]);
                bFound = (bool) (iFnd > -1);
            } else {
                // search forward of the old part found
                iFnd = (int) strFile.find(strPart[i], iFnd+1);
                bFound = bFound && (bool) (iFnd > -1);
            }
        }

        if (bFound) {
            // this pattern matched the file, add to vector
            // NB: first get stat to make sure it really is a file
            strFullPath = strUserDir + strFile;
            // only add if the file really exists (i.e. not a directory)
            if (is_file(strFullPath.c_str())) {
                pList->push_back(strFullPath);
            }
        }
    }

    // sort by file creation time
    // sort if list is greather than 1
    if (pList->size()>1)  {
       sort(pList->begin(), pList->end(), StringVectorSort);  // may as well sort it?
    }
    return true;
}

// Read compressed file to memory.
//
int boinc_UnzipToMemory (char *zip, char *file, string &retstr) {
    if (!zip || !file) return 0;
    std::ifstream is(zip, std::ios::binary);
    if (!is) return 0;
    std::vector<CDEntry> entries; uint32_t cd_off=0;
    if (!parse_central_dir(is, entries, cd_off)) return 0;
    for (auto& e : entries) {
        if (e.name == file) {
            // Read local header and data
            is.seekg(e.local_header_offset, std::ios::beg);
            if (read_le32(is) != 0x04034b50) return 0;
            (void)read_le16(is);
            (void)read_le16(is);
            uint16_t method = read_le16(is);
            (void)read_le16(is);
            (void)read_le16(is);
            (void)read_le32(is);
            uint32_t csize = read_le32(is);
            uint32_t usize = read_le32(is);
            uint16_t nlen = read_le16(is);
            uint16_t xlen = read_le16(is);
            is.seekg(nlen + xlen, std::ios::cur);
            std::vector<unsigned char> comp(csize);
            if (csize) is.read((char*)comp.data(), csize);
            std::string out;
            if (method == 0) {
                out.assign((const char*)comp.data(), comp.size());
            } else if (method == 8) {
                if (!inflate_raw(comp.data(), comp.size(), out, usize)) return 0;
            } else {
                return 0;
            }
            retstr = out;
            return 1; // non-zero on success, to match previous behavior
        }
    }
    return 0;
}
