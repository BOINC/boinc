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
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.

#include "gtest/gtest.h"
#include "zip/boinc_zip.h"
#include <math.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <random>

using std::string;
using std::vector;

using namespace std;

class ZipTests : public ::testing::Test {};

namespace {
// Create a unique temporary directory and return its path (cross-platform)
string make_temp_dir(const char* prefix = "boinc_zip_test_") {
    namespace fs = std::filesystem;
    fs::path base = fs::temp_directory_path();
    std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<unsigned long long> dist;
    for (int i = 0; i < 100; ++i) {
        auto suffix = dist(rng);
        std::ostringstream name;
        name << prefix << std::hex << suffix;
        fs::path candidate = base / name.str();
        std::error_code ec;
        if (fs::create_directories(candidate, ec)) {
            return candidate.string();
        }
        if (!fs::exists(candidate)) {
            // create_directories failed for another reason; try next iteration
            continue;
        }
    }
    // Fallback: last resort, use base itself (should not happen in practice)
    return base.string();
}

// Join two path segments with '/'
string join_path(const string& a, const string& b) {
    namespace fs = std::filesystem;
    return (fs::path(a) / fs::path(b)).string();
}

// Write content to file path
void write_file(const string& path, const string& content) {
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(ofs.is_open()) << "Failed to open file for write: " << path;
    ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
    ASSERT_TRUE(ofs.good()) << "Short write to: " << path;
}

// Read entire file content.
string read_file(const string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs.is_open()) return string();
    std::ostringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

// Set mtime using filesystem clock with a seconds offset from now (cross-platform)
void set_mtime_offset(const string& path, int seconds_offset) {
    namespace fs = std::filesystem;
    using namespace std::chrono;
    auto t = fs::file_time_type::clock::now() + seconds(seconds_offset);
    fs::last_write_time(fs::path(path), t);
}
}

// boinc_filelist: patterns, sorting by name/time, clear/append behavior
TEST_F(ZipTests, FileList_Patterns_And_Sorting) {
    string dir = make_temp_dir();
    ASSERT_FALSE(dir.empty());

    // Create files matching the CPDN-style pattern and one extra file
    string f1 = join_path(dir, "a.pc.test.x4.1.nc");
    string f2 = join_path(dir, "b.pc.test.x4.2.nc");
    string f3 = join_path(dir, "c.txt");
    write_file(f1, "one");
    write_file(f2, "two");
    write_file(f3, "three");
    // Make a subdirectory which should be ignored by boinc_filelist
    string subdir = join_path(dir, "sub");
    std::filesystem::create_directories(subdir);

    // Ensure differing mtimes for time-based sort
    set_mtime_offset(f1, -3);
    set_mtime_offset(f2, -2);
    set_mtime_offset(f3, -1);

    // Use directory with trailing slash to avoid undefined access in implementation
    string dir_with_slash = dir + "/";

    // Pattern with '|' segments should match f1 and f2 only
    ZipFileList list;
    bool ok = boinc_filelist(dir_with_slash, ".pc|.x4.|.nc", &list, SORT_NAME | SORT_ASCENDING, true);
    ASSERT_TRUE(ok);
    ASSERT_EQ(list.size(), (size_t)2);
    EXPECT_EQ(list[0], join_path(dir, "a.pc.test.x4.1.nc"));
    EXPECT_EQ(list[1], join_path(dir, "b.pc.test.x4.2.nc"));

    // No '|' pattern should match only c.txt; append without clearing and check total size
    ok = boinc_filelist(dir_with_slash, "c.txt", &list, SORT_NAME | SORT_ASCENDING, false);
    ASSERT_TRUE(ok);
    ASSERT_EQ(list.size(), (size_t)3);
    // After sort ascending by name (full path), 'c.txt' should be last
    EXPECT_EQ(list[2], join_path(dir, "c.txt"));

    // Wildcard '*' becomes blank pattern => all files; sort by time ascending
    ZipFileList byTime;
    ok = boinc_filelist(dir_with_slash, "*", &byTime, SORT_TIME | SORT_ASCENDING, true);
    ASSERT_TRUE(ok);
    // Should include only regular files: f1, f2, f3
    ASSERT_EQ(byTime.size(), (size_t)3);
    EXPECT_EQ(byTime[0], f1); // oldest mtime
    EXPECT_EQ(byTime[1], f2);
    EXPECT_EQ(byTime[2], f3); // newest mtime

    // Time descending
    ok = boinc_filelist(dir_with_slash, "*.*", &byTime, SORT_TIME | SORT_DESCENDING, true);
    ASSERT_TRUE(ok);
    ASSERT_EQ(byTime.size(), (size_t)3);
    EXPECT_EQ(byTime[0], f3);
    EXPECT_EQ(byTime[1], f2);
    EXPECT_EQ(byTime[2], f1);
}

// boinc_zip string overload for a single file, then unzip to a directory
TEST_F(ZipTests, ZipAndUnzip_SingleFile_StringOverload) {
    string dir = make_temp_dir();
    ASSERT_FALSE(dir.empty());
    string src = join_path(dir, "hello.txt");
    string content = "Hello, BOINC!";
    write_file(src, content);

    string zipPath = join_path(dir, "archive_single.zip");
    // Zip the single file
    int zr = boinc_zip(ZIP_IT, zipPath, src);
    EXPECT_EQ(zr, 0) << "zip returned non-zero";
    // Ensure the zip exists
    EXPECT_TRUE(std::filesystem::exists(zipPath));

    // Unzip to destination directory
    string outDir = join_path(dir, "unzip_out");
    std::filesystem::create_directories(outDir);
    int ur = boinc_zip(UNZIP_IT, zipPath, outDir);
    EXPECT_EQ(ur, 0) << "unzip returned non-zero";

    string outFile = join_path(outDir, "hello.txt");
    EXPECT_EQ(read_file(outFile), content);
}

// boinc_zip vector overload for zipping multiple files, then unzip with vector specifying -d <dir>
TEST_F(ZipTests, ZipAndUnzip_MultipleFiles_VectorOverload) {
    string dir = make_temp_dir();
    ASSERT_FALSE(dir.empty());
    string f1 = join_path(dir, "one.txt");
    string f2 = join_path(dir, "two.txt");
    write_file(f1, "111");
    write_file(f2, "222");

    string zipPath = join_path(dir, "archive_multi.zip");
    ZipFileList inputs{f1, f2};
    int zr = boinc_zip(ZIP_IT, zipPath, &inputs);
    EXPECT_EQ(zr, 0);

    string outDir = join_path(dir, "unz");
    std::filesystem::create_directories(outDir);
    ZipFileList unzipArgs{outDir}; // exactly one arg => used as -d <dir>
    int ur = boinc_zip(UNZIP_IT, zipPath, &unzipArgs);
    EXPECT_EQ(ur, 0);

    EXPECT_EQ(read_file(join_path(outDir, "one.txt")), "111");
    EXPECT_EQ(read_file(join_path(outDir, "two.txt")), "222");
}

// boinc_zip char* overload and overwrite behavior (existing zip is unlinked then recreated)
TEST_F(ZipTests, Zip_CharOverload_OverwriteExisting) {
    string dir = make_temp_dir();
    ASSERT_FALSE(dir.empty());
    string src = join_path(dir, "data.txt");
    write_file(src, "v1");

    string zipPath = join_path(dir, "archive_overwrite.zip");
    int r1 = boinc_zip(ZIP_IT, zipPath.c_str(), src.c_str());
    EXPECT_EQ(r1, 0);

    // Change source content and zip again to same archive name
    write_file(src, "v2_updated");
    int r2 = boinc_zip(ZIP_IT, zipPath.c_str(), src.c_str());
    EXPECT_EQ(r2, 0);

    // Unzip and verify we see updated content, proving overwrite worked
    string outDir = join_path(dir, "out");
    std::filesystem::create_directories(outDir);
    int ur = boinc_zip(UNZIP_IT, zipPath, outDir);
    EXPECT_EQ(ur, 0);
    EXPECT_EQ(read_file(join_path(outDir, "data.txt")), "v2_updated");
}

// Unzip non-existent archive should return 2 per implementation
TEST_F(ZipTests, Unzip_Nonexistent_Returns2) {
    string dir = make_temp_dir();
    ASSERT_FALSE(dir.empty());
    string zipPath = join_path(dir, "no_such.zip");
    // Ensure it doesn't exist
    EXPECT_FALSE(std::filesystem::exists(zipPath));

    string outDir = join_path(dir, "dest");
    std::filesystem::create_directories(outDir);

    int ur = boinc_zip(UNZIP_IT, zipPath, outDir);
    EXPECT_EQ(ur, 2);
}

// boinc_UnzipToMemory happy path and missing file path
TEST_F(ZipTests, UnzipToMemory_Success_And_Failure) {
    string dir = make_temp_dir();
    ASSERT_FALSE(dir.empty());
    string src = join_path(dir, "mem.txt");
    string content = "memory contents";
    write_file(src, content);
    string zipPath = join_path(dir, "mem.zip");
    ASSERT_EQ(boinc_zip(ZIP_IT, zipPath, src), 0);

    // Success: read mem.txt from the zip into memory
    string out;
    int ret = boinc_UnzipToMemory((char*)zipPath.c_str(), (char*)"mem.txt", out);
    // Implementation sets retstr only when ret is non-zero
    EXPECT_NE(ret, 0);
    EXPECT_EQ(out, content);

    // Failure: file not in archive
    string out2;
    int ret2 = boinc_UnzipToMemory((char*)zipPath.c_str(), (char*)"missing.txt", out2);
    EXPECT_EQ(ret2, 0);
    EXPECT_TRUE(out2.empty());
}
