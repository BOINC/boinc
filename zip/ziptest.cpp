// ziptest.cpp : Defines the entry point for the console application.
//

#ifndef _WIN32
#include "../config.h"
#endif
#include "boinc_zip.h"
#include "filesys.h"
#include <zlib.h>  // CMC -- test that we "co-exist" with the "stock" zlib library

int do_gzip(const char* strGZ, const char* strInput)
{
	// take an input file (strInput) and turn it into a compressed file (strGZ)
	// get rid of the input file after 

	FILE* fIn = boinc_fopen(strInput, "rb");
	if (!fIn)  return 1; //error
	gzFile fOut = gzopen(strGZ, "wb");
	if (!fOut) return 1; //error

	fseek(fIn, 0, SEEK_SET);  // go to the top of the files
	gzseek(fOut, 0, SEEK_SET);
	unsigned char buf[1024];
	long lRead = 0, lWrite = 0;
	while (!feof(fIn)) { // read 1KB at a time until end of file
		memset(buf, 0x00, 1024);
		lRead = 0;
		lRead = fread(buf, 1, 1024, fIn);
		lWrite = gzwrite(fOut, buf, lRead);
		if (lRead != lWrite) break;
	}
	gzclose(fOut);
	fclose(fIn);

	if (lRead != lWrite) return 1;  //error -- read bytes != written bytes

	// if we made it here, it compressed OK, can erase strInput and leave
	boinc_delete_file(strInput);
	return 0;
}

int main(int argc, char* argv[])
{
	ZipFileList zflist;
#ifdef _WIN32
	boinc_filelist("c:\\temp", "", &zflist, SORT_NAME | SORT_ASCENDING);
#else
	boinc_filelist("/tmp/junk", "", &zflist, SORT_NAME | SORT_ASCENDING);
#endif
        int jj; 
		char strTmp[128], strTmp2[128];
		for (jj = 0; jj < zflist.size(); jj++) {
			printf("%s  %d\n", zflist[jj].c_str(), zflist[jj].m_statFile.st_mtime);
			// now gzip it, silly but see how it goes!
			sprintf(strTmp, "%s.gz", zflist[jj].c_str());
			sprintf(strTmp2, "%s.zip", strTmp);
                     printf("infile=%s  outfile=%s\n", strTmp, strTmp2);
			do_gzip(strTmp, zflist[jj].c_str());
                    boinc_zip(ZIP_IT, strTmp2, strTmp);
		}
    boinc_zip(UNZIP_IT, "/tmp/boinc_zip.zip", "/tmp/junk/boinc_zip");
    return 0;
}

const char *BOINC_RCSID_9851414a72 = "$Id: ziptest.cpp 7481 2005-08-25 21:33:28Z davea $";
