// ziptest.cpp : Defines the entry point for the console application.
//

#ifndef _WIN32
#include "../boinc/config.h"
#endif
#include "boinc_zip.h"

int main(int argc, char* argv[])
{
	ZipFileList zflist;
#ifdef _WIN32
	boinc_filelist("temp/linux", "", &zflist, SORT_NAME | SORT_ASCENDING);
#else
	boinc_filelist("/var/home/carlc", "", &zflist, SORT_NAME | SORT_ASCENDING);
#endif
        int jj;
	for (jj = 0; jj < zflist.size(); jj++)
		printf("%s  %d\n", zflist[jj].c_str(), zflist[jj].m_statFile.st_mtime);

#ifdef _WIN32
     boinc_zip(ZIP_IT, "c:\\temp\\bztest", &zflist);
#else
     boinc_zip(ZIP_IT, "./ziptest.zip", &zflist);
#endif
    return 0;
}

#ifdef __GNUC__
static volatile const char  __attribute__((unused)) *BOINCrcsid="$Id$";
#else
static volatile const char *BOINCrcsid="$Id$";
#endif
