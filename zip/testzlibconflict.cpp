#define DIRREF long

//#include "config.h"
#include "boinc_zip.h"
#ifdef _WIN32
#include "boinc_win.h"
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>  // test that we "co-exist" with the "stock" zlib library

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

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

int main()
{
	int retval = -2;
    ZipFileList zf;
    std::string home, result_dir, zipfile, source_dir;
    std::string source_file, result_zipfile, result_file;

#ifdef _WIN32
	// replace with the path/file wildcard of your choice
        home = std::string("C:/Documents and Settings/All Users/Documents");
        result_dir = home + std::string("/testresult");
        CreateDirectoryA(result_dir.c_str(), NULL);
#else
        home = std::string(getenv("HOME"));
        result_dir = home + std::string("/testresult");
        mkdir(result_dir.c_str(), 0777);
        zipfile = result_dir + std::string("/test.zip");
        source_dir = home + std::string("/Testfiles");
#endif

        zipfile = result_dir + std::string("/test.zip");
        source_dir = home + std::string("/Testfiles");

        if (boinc_filelist(source_dir.c_str(), ".txt", &zf) && zf.size()) {
            // Compress and uncompress using boinc_zip
            retval = boinc_zip(ZIP_IT, zipfile, &zf);
            retval = boinc_zip(UNZIP_IT, zipfile, result_dir.c_str());
    
            // Compress and uncompress using zlib
            source_file = zf[0].c_str();
            result_zipfile = result_dir + "/zlib_test.zip";

            result_file = result_dir + "/zlib_resultfile.txt";
            FILE *sf = fopen(source_file.c_str(), "rb");
            FILE *rzf = fopen(result_zipfile.c_str(), "wb");
            retval = def(sf, rzf, Z_DEFAULT_COMPRESSION);
            fclose(sf);
            fclose(rzf);

            rzf = fopen(result_zipfile.c_str(), "rb");
            FILE *rf = fopen(result_file.c_str(), "wb");

            retval = inf(rzf, rf);
            fclose(rzf);
            fclose(rf);
    }

   return retval;
}
