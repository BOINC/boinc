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

#define DIRREF long

#include <fstream>
#include <filesystem>

#include <zlib.h>  // test that we "co-exist" with the "stock" zlib library

#include "gtest/gtest.h"
#include "boinc_zip.h"
#include "util.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

using namespace std;

namespace test_zip {
    class test_zip : public ::testing::Test {};

    filesystem::path prepare_test_directory(bool pause_between_files) {
        filesystem::path test_dir = "test_data";
        filesystem::create_directory(test_dir);
        ofstream(test_dir / "file1.txt") << "This is a txt file 1.";
        if (pause_between_files) {
            boinc_sleep(1);
        }
        ofstream(test_dir / "file2.txt") << "This is a txt file 2.";
        if (pause_between_files) {
            boinc_sleep(1);
        }
        ofstream(test_dir / "file3.txt") << "This is a txt file 3.";
        ofstream(test_dir / "file1.log") << "This is a log file 1.";
        ofstream(test_dir / "file2.log") << "This is a log file 2.";
        ofstream(test_dir / "file3.log") << "This is a log file 3.";
        ofstream(test_dir / "file1.dat") << "This is a dat file 1.";
        ofstream(test_dir / "file2.dat") << "This is a dat file 2.";
        ofstream(test_dir / "file3.dat") << "This is a dat file 3.";
        filesystem::path subdir = test_dir / "subdir";
        filesystem::create_directory(subdir);
        ofstream(subdir / "file4.txt") << "This is a txt file 4.";
        ofstream(subdir / "file5.txt") << "This is a txt file 5.";
        ofstream(subdir / "file4.log") << "This is a log file 4.";
        ofstream(subdir / "file5.log") << "This is a log file 5.";
        ofstream(subdir / "file4.dat") << "This is a dat file 4.";
        ofstream(subdir / "file5.dat") << "This is a dat file 5.";
        return test_dir;
    }

    // *** ROUTINES TO COMPRESS AND UNCOMPRESS USING ZLIB ***

    #define CHUNK 16384

    /* Compress from file source to file dest until EOF on source.
       def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
       allocated for processing, Z_STREAM_ERROR if an invalid compression
       level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
       version of the library linked do not match, or Z_ERRNO if there is
       an error reading or writing the files. */
    int def(FILE *source, FILE *dest, int level)
    {
        int ret, flush;
        unsigned have;
        z_stream strm;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];

        /* allocate deflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        ret = deflateInit(&strm, level);
        if (ret != Z_OK)
            return ret;

        /* compress until end of file */
        do {
            strm.avail_in = fread(in, 1, CHUNK, source);
            if (ferror(source)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
            flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
            strm.next_in = in;

            /* run deflate() on input until output buffer not full, finish
               compression if all of source has been read in */
            do {
                strm.avail_out = CHUNK;
                strm.next_out = out;
                ret = deflate(&strm, flush);    /* no bad return value */
                assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
                have = CHUNK - strm.avail_out;
                if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                    (void)deflateEnd(&strm);
                    return Z_ERRNO;
                }
            } while (strm.avail_out == 0);
            assert(strm.avail_in == 0);     /* all input will be used */

            /* done when last data in file processed */
        } while (flush != Z_FINISH);
        assert(ret == Z_STREAM_END);        /* stream will be complete */

        /* clean up and return */
        (void)deflateEnd(&strm);
        return Z_OK;
    }

    /* Decompress from file source to file dest until stream ends or EOF.
       inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
       allocated for processing, Z_DATA_ERROR if the deflate data is
       invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
       the version of the library linked do not match, or Z_ERRNO if there
       is an error reading or writing the files. */
    int inf(FILE *source, FILE *dest)
    {
        int ret;
        unsigned have;
        z_stream strm;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];

        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        ret = inflateInit(&strm);
        if (ret != Z_OK)
            return ret;

        /* decompress until deflate stream ends or end of file */
        do {
            strm.avail_in = fread(in, 1, CHUNK, source);
            if (ferror(source)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
            if (strm.avail_in == 0)
                break;
            strm.next_in = in;

            /* run inflate() on input until output buffer not full */
            do {
                strm.avail_out = CHUNK;
                strm.next_out = out;
                ret = inflate(&strm, Z_NO_FLUSH);
                assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
                switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                    [[fallthrough]];
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return ret;
                }
                have = CHUNK - strm.avail_out;
                if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                    (void)inflateEnd(&strm);
                    return Z_ERRNO;
                }
            } while (strm.avail_out == 0);

            /* done when inflate() says it's done */
        } while (ret != Z_STREAM_END);

        /* clean up and return */
        (void)inflateEnd(&strm);
        return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
    }

    /* report a zlib or i/o error */
    void zerr(int ret)
    {
        fputs("zpipe: ", stderr);
        switch (ret) {
        case Z_ERRNO:
            if (ferror(stdin))
                fputs("error reading stdin\n", stderr);
            if (ferror(stdout))
                fputs("error writing stdout\n", stderr);
            break;
        case Z_STREAM_ERROR:
            fputs("invalid compression level\n", stderr);
            break;
        case Z_DATA_ERROR:
            fputs("invalid or incomplete deflate data\n", stderr);
            break;
        case Z_MEM_ERROR:
            fputs("out of memory\n", stderr);
            break;
        case Z_VERSION_ERROR:
            fputs("zlib version mismatch!\n", stderr);
        }
    }

    // *** END OF ROUTINES TO COMPRESS AND UNCOMPRESS USING ZLIB ***


#ifndef __APPLE__
    TEST_F(test_zip, Test_boinc_filelist_sorting) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(true));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), "", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 9);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file1.dat").string());
        EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file1.log").string());
        EXPECT_EQ(file_list[2], filesystem::absolute(test_dir / "file1.txt").string());
        EXPECT_EQ(file_list[3], filesystem::absolute(test_dir / "file2.dat").string());
        EXPECT_EQ(file_list[4], filesystem::absolute(test_dir / "file2.log").string());
        EXPECT_EQ(file_list[5], filesystem::absolute(test_dir / "file2.txt").string());
        EXPECT_EQ(file_list[6], filesystem::absolute(test_dir / "file3.dat").string());
        EXPECT_EQ(file_list[7], filesystem::absolute(test_dir / "file3.log").string());
        EXPECT_EQ(file_list[8], filesystem::absolute(test_dir / "file3.txt").string());

        result = boinc_filelist(test_dir.string(), "", &file_list, SORT_NAME | SORT_DESCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 9);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file3.txt").string());
        EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file3.log").string());
        EXPECT_EQ(file_list[2], filesystem::absolute(test_dir / "file3.dat").string());
        EXPECT_EQ(file_list[3], filesystem::absolute(test_dir / "file2.txt").string());
        EXPECT_EQ(file_list[4], filesystem::absolute(test_dir / "file2.log").string());
        EXPECT_EQ(file_list[5], filesystem::absolute(test_dir / "file2.dat").string());
        EXPECT_EQ(file_list[6], filesystem::absolute(test_dir / "file1.txt").string());
        EXPECT_EQ(file_list[7], filesystem::absolute(test_dir / "file1.log").string());
        EXPECT_EQ(file_list[8], filesystem::absolute(test_dir / "file1.dat").string());

        result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_TIME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file1.txt").string());
        EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file2.txt").string());
        EXPECT_EQ(file_list[2], filesystem::absolute(test_dir / "file3.txt").string());

        result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_TIME | SORT_DESCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file3.txt").string());
        EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file2.txt").string());
        EXPECT_EQ(file_list[2], filesystem::absolute(test_dir / "file1.txt").string());
 
        filesystem::remove_all(test_dir);
    }

    TEST_F(test_zip, Test_boinc_filelist_empty_pattern) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;
        vector<string> patterns = {"", "*", "*.*"};
        for (const auto& pattern : patterns) {
            bool result = boinc_filelist(test_dir.string(), pattern, &file_list, SORT_NAME | SORT_ASCENDING);
            EXPECT_TRUE(result);
            ASSERT_EQ(file_list.size(), 9);
            EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file1.dat").string());
            EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file1.log").string());
            EXPECT_EQ(file_list[2], filesystem::absolute(test_dir / "file1.txt").string());
            EXPECT_EQ(file_list[3], filesystem::absolute(test_dir / "file2.dat").string());
            EXPECT_EQ(file_list[4], filesystem::absolute(test_dir / "file2.log").string());
            EXPECT_EQ(file_list[5], filesystem::absolute(test_dir / "file2.txt").string());
            EXPECT_EQ(file_list[6], filesystem::absolute(test_dir / "file3.dat").string());
            EXPECT_EQ(file_list[7], filesystem::absolute(test_dir / "file3.log").string());
            EXPECT_EQ(file_list[8], filesystem::absolute(test_dir / "file3.txt").string());
        }
        filesystem::remove_all(test_dir);
    }

    TEST_F(test_zip, Test_boinc_filelist_pattern_matching) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file1.txt").string());
        EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file2.txt").string());
        EXPECT_EQ(file_list[2], filesystem::absolute(test_dir / "file3.txt").string());

        result = boinc_filelist(test_dir.string(), ".log", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file1.log").string());
        EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file2.log").string());
        EXPECT_EQ(file_list[2], filesystem::absolute(test_dir / "file3.log").string());

        result = boinc_filelist(test_dir.string(), ".dat", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file1.dat").string());
        EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file2.dat").string());
        EXPECT_EQ(file_list[2], filesystem::absolute(test_dir / "file3.dat").string());

        result = boinc_filelist(test_dir.string(), "f|1|t", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 2);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file1.dat").string());
        EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file1.txt").string());

        result = boinc_filelist(test_dir.string(), "file1|.txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 1);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file1.txt").string());

        result = boinc_filelist(test_dir.string(), "file|2", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);
        EXPECT_EQ(file_list[0], filesystem::absolute(test_dir / "file2.dat").string());
        EXPECT_EQ(file_list[1], filesystem::absolute(test_dir / "file2.log").string());
        EXPECT_EQ(file_list[2], filesystem::absolute(test_dir / "file2.txt").string());
        filesystem::remove_all(test_dir);
    }

    TEST_F(test_zip, Test_boinc_filelist_no_matches) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".pdf", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 0);

        result = boinc_filelist(test_dir.string(), "nonexistent", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 0);
        filesystem::remove_all(test_dir);
    }

    TEST_F(test_zip, Test_boinc_filelist_invalid_directory) {
        ZipFileList file_list;

        bool result = boinc_filelist("nonexistent_directory", ".txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 0);
    }

    TEST_F(test_zip, Test_boinc_filelist_null_pointer) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));

        bool result = boinc_filelist(test_dir.string(), ".txt", nullptr, SORT_NAME | SORT_ASCENDING);
        EXPECT_FALSE(result);
        filesystem::remove_all(test_dir);
    }

    TEST_F(test_zip, Test_boinc_filelist_clear_option) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_NAME | SORT_ASCENDING, true);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);

        result = boinc_filelist(test_dir.string(), ".log", &file_list, SORT_NAME | SORT_ASCENDING, false);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 6); // should have both .txt and .log files

        result = boinc_filelist(test_dir.string(), ".dat", &file_list, SORT_NAME | SORT_ASCENDING, true);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3); // should have only .dat files now
        filesystem::remove_all(test_dir);
    }

    TEST_F(test_zip, Test_boinc_filelist_relative_path) {
        const filesystem::path test_dir = prepare_test_directory(false);
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);
        EXPECT_EQ(file_list[0], (test_dir / "file1.txt").string());
        EXPECT_EQ(file_list[1], (test_dir / "file2.txt").string());
        EXPECT_EQ(file_list[2], (test_dir / "file3.txt").string());
        filesystem::remove_all(test_dir);
    }

    TEST_F(test_zip, Test_boinc_zip_and_unzip_single_file) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);

        filesystem::path zip_path = "test_archive.zip";
        int zip_result = boinc_zip(ZIP_IT, zip_path.string(), file_list[0]);
        EXPECT_EQ(zip_result, 0);
        EXPECT_TRUE(filesystem::exists(zip_path));

        filesystem::path unzip_dir = "test_archive_unzipped";
        filesystem::create_directory(unzip_dir);
        zip_result = boinc_zip(UNZIP_IT, zip_path.string(), unzip_dir.string());
        EXPECT_EQ(zip_result, 0);

        filesystem::path filename = filesystem::path(file_list[0]).filename();
        EXPECT_TRUE(filesystem::exists(unzip_dir / filename));
        ifstream original(file_list[0]);
        ifstream unzipped(unzip_dir / filename);
        string original_content((istreambuf_iterator<char>(original)), istreambuf_iterator<char>());
        string unzipped_content((istreambuf_iterator<char>(unzipped)), istreambuf_iterator<char>());
        EXPECT_EQ(original_content, unzipped_content);

        filesystem::remove_all(test_dir);
        filesystem::remove_all(unzip_dir);
        filesystem::remove(zip_path);
    }

    TEST_F(test_zip, Test_boinc_zip_and_unzip_multiple_files) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);

        filesystem::path zip_path = "test_archive.zip";
        int zip_result = boinc_zip(ZIP_IT, zip_path.string(), &file_list);
        EXPECT_EQ(zip_result, 0);
        EXPECT_TRUE(filesystem::exists(zip_path));

        filesystem::path unzip_dir = "test_archive_unzipped";
        filesystem::create_directory(unzip_dir);
        zip_result = boinc_zip(UNZIP_IT, zip_path.string(), unzip_dir.string());
        EXPECT_EQ(zip_result, 0);

        for (const auto& file : file_list) {
            filesystem::path filename = filesystem::path(file).filename();
            EXPECT_TRUE(filesystem::exists(unzip_dir / filename));
            ifstream original(file);
            ifstream unzipped(unzip_dir / filename);
            string original_content((istreambuf_iterator<char>(original)), istreambuf_iterator<char>());
            string unzipped_content((istreambuf_iterator<char>(unzipped)), istreambuf_iterator<char>());
            EXPECT_EQ(original_content, unzipped_content);
        }

        filesystem::remove_all(test_dir);
        filesystem::remove_all(unzip_dir);
        filesystem::remove(zip_path);
    }

#ifndef _WIN32
    TEST_F(test_zip, Test_boinc_zip_and_unzip_file_permissions) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);

        // Set specific permissions on the original files
        filesystem::permissions(test_dir / "file1.txt", filesystem::perms::owner_read | filesystem::perms::owner_write);
        filesystem::permissions(test_dir / "file2.txt", filesystem::perms::owner_read);
        filesystem::permissions(test_dir / "file3.txt", filesystem::perms::owner_read | filesystem::perms::owner_write | filesystem::perms::owner_exec);

        filesystem::path zip_path = "test_archive.zip";
        int zip_result = boinc_zip(ZIP_IT, zip_path.string(), &file_list);
        EXPECT_EQ(zip_result, 0);
        EXPECT_TRUE(filesystem::exists(zip_path));

        filesystem::path unzip_dir = "test_archive_unzipped";
        filesystem::create_directory(unzip_dir);
        zip_result = boinc_zip(UNZIP_IT, zip_path.string(), unzip_dir.string());
        EXPECT_EQ(zip_result, 0);

        // Check that the permissions are preserved after unzipping
        EXPECT_EQ(filesystem::status(unzip_dir / "file1.txt").permissions(),
                  filesystem::perms::owner_read | filesystem::perms::owner_write);
        EXPECT_EQ(filesystem::status(unzip_dir / "file2.txt").permissions(),
                  filesystem::perms::owner_read);
        EXPECT_EQ(filesystem::status(unzip_dir / "file3.txt").permissions(),
                  filesystem::perms::owner_read | filesystem::perms::owner_write | filesystem::perms::owner_exec);

        filesystem::remove_all(test_dir);
        filesystem::remove_all(unzip_dir);
        filesystem::remove(zip_path);
    }
#endif

    TEST_F(test_zip, Test_boinc_UnzipToMemory) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);

        filesystem::path zip_path = "test_archive.zip";
        int zip_result = boinc_zip(ZIP_IT, zip_path.string(), &file_list);
        EXPECT_EQ(zip_result, 0);
        EXPECT_TRUE(filesystem::exists(zip_path));

        string file_content;
        zip_result = boinc_UnzipToMemory(const_cast<char*>(zip_path.string().c_str()),
                                         const_cast<char*>("file2.txt"), file_content);
        EXPECT_EQ(zip_result, 1);
        EXPECT_EQ(file_content, "This is a txt file 2.");

        // Test for a non-existent file in the zip
        zip_result = boinc_UnzipToMemory(const_cast<char*>(zip_path.string().c_str()),
                                         const_cast<char*>("nonexistent.txt"), file_content);
        EXPECT_NE(zip_result, 1); // Expecting an error code

        filesystem::remove_all(test_dir);
        filesystem::remove(zip_path);
    }

    TEST_F(test_zip, Test_boinc_UnzipToMemory_invalid_zip) {
        string file_content;
        int zip_result = boinc_UnzipToMemory(const_cast<char*>("nonexistent.zip"),
                                             const_cast<char*>("file.txt"), file_content);
        EXPECT_NE(zip_result, 1); // Expecting an error code
    }

    TEST_F(test_zip, Test_boinc_UnzipToMemory_empty_file) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);

        filesystem::path zip_path = "test_archive.zip";
        int zip_result = boinc_zip(ZIP_IT, zip_path.string(), &file_list);
        EXPECT_EQ(zip_result, 0);
        EXPECT_TRUE(filesystem::exists(zip_path));

        string file_content;
        zip_result = boinc_UnzipToMemory(const_cast<char*>(zip_path.string().c_str()),
                                         const_cast<char*>(""), file_content);
        EXPECT_NE(zip_result, 1); // Expecting an error code

        filesystem::remove_all(test_dir);
        filesystem::remove(zip_path);
    }

    TEST_F(test_zip, Test_boinc_UnzipToMemory_null_pointers) {
        string file_content;
        int zip_result = boinc_UnzipToMemory(nullptr, const_cast<char*>("file.txt"), file_content);
        EXPECT_NE(zip_result, 1); // Expecting an error code

        zip_result = boinc_UnzipToMemory(const_cast<char*>("test.zip"), nullptr, file_content);
        EXPECT_NE(zip_result, 1); // Expecting an error code
    }

    TEST_F(test_zip, Test_boinc_UnzipToMemory_empty_zip) {
        // Create an empty zip file
        filesystem::path zip_path = "empty_archive.zip";
        ofstream(zip_path).close();
        EXPECT_TRUE(filesystem::exists(zip_path));

        string file_content;
        int zip_result = boinc_UnzipToMemory(const_cast<char*>(zip_path.string().c_str()),
                                             const_cast<char*>("file.txt"), file_content);
        EXPECT_NE(zip_result, 1); // Expecting an error code

        filesystem::remove(zip_path);
    }

    TEST_F(test_zip, Test_boinc_UnzipToMemory_large_file) {
        const filesystem::path test_dir = filesystem::absolute(prepare_test_directory(false));
        ZipFileList file_list;

        bool result = boinc_filelist(test_dir.string(), ".txt", &file_list, SORT_NAME | SORT_ASCENDING);
        EXPECT_TRUE(result);
        ASSERT_EQ(file_list.size(), 3);

        // Create a large file
        ofstream(test_dir / "large_file.txt");
        for (int i = 0; i < 100000; ++i) {
            ofstream(test_dir / "large_file.txt", ios::app) << "This is a line in a large file.\n";
        }
        file_list.push_back((test_dir / "large_file.txt").string());

        filesystem::path zip_path = "test_archive.zip";
        int zip_result = boinc_zip(ZIP_IT, zip_path.string(), &file_list);
        EXPECT_EQ(zip_result, 0);
        EXPECT_TRUE(filesystem::exists(zip_path));

        string file_content;
        zip_result = boinc_UnzipToMemory(const_cast<char*>(zip_path.string().c_str()),
                                         const_cast<char*>("large_file.txt"), file_content);
        EXPECT_EQ(zip_result, 1);
        EXPECT_GT(file_content.size(), 100000 * 30); // Rough estimate of size

        filesystem::remove_all(test_dir);
        filesystem::remove(zip_path);
    }

#endif
}

