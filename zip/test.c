#define DIRREF long

#include "config.h"
#include "boinc_zip.h"
#ifndef _WIN32
#endif

int main()
{
	int retval = -2;
        ZipFileList zf;

#ifdef _WIN32
	// replace with the path/file wildcard of your choice
	int retval = boinc_zip(ZIP_IT, 
              "e:\\temp\\netsup.vxd e:\\temp\\vredir.vxd", 
              "e:\\temp\\test.zip", NULL);
        retval = boinc_zip(UNZIP_IT, "e:\\temp\\test.zip", 
              "/home/carlc/testing", NULL);
#else
        std::string zipfile = "test.zip";
        if (boinc_filelist("/var/home/carlc/", ".txt", &zf) && zf.size()) {
	  retval = boinc_zip(ZIP_IT, zipfile, &zf);
          retval = boinc_zip(UNZIP_IT, zipfile, NULL);
        }
#endif

   return retval;
}


const char *BOINC_RCSID_9f234ef8a3 = "$Id: test.c 4979 2005-01-02 18:29:53Z ballen $";
