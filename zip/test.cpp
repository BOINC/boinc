#include <errno.h>
#include <stdio.h>
#include <string>

using std::string;

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <stdlib.h>
#endif

#include "boinc_zip.h"

int main() {
    int retval = -2;
    ZipFileList zf;

#ifdef _WIN32
    // replace with the path/file wildcard of your choice
    string home = string("C:/Documents and Settings/All Users/Documents");
    string result_dir = home + string("/testresult");
    CreateDirectoryA(result_dir.c_str(), NULL);
    string zipfile = result_dir + string("/test.zip");
    string source_dir = home + string("/Testfiles");
    if (boinc_filelist(source_dir.c_str(), ".txt", &zf) && zf.size()) {
        retval = boinc_zip(ZIP_IT, zipfile, &zf);
        retval = boinc_zip(UNZIP_IT, zipfile, result_dir.c_str());
    }
#else
    string home = string(getenv("HOME"));
    string result_dir = home + string("/testresult");
    if (mkdir(result_dir.c_str(), 0777) < 0) {
        perror("mkdir");
    }
    string zipfile = result_dir + string("/test.zip");
    string source_dir = home + string("/Testfiles");
    if (boinc_filelist(source_dir, string(".txt"), &zf) && zf.size()) {
        retval = boinc_zip(ZIP_IT, zipfile, &zf);
        retval = boinc_zip(UNZIP_IT, zipfile, result_dir.c_str());
    }
#endif

   return retval;
}
