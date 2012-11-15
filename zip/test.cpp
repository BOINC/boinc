#define DIRREF long

//#include "config.h"
#include "boinc_zip.h"
#ifdef _WIN32
#include "boinc_win.h"
#endif


int main()
{
	int retval = -2;
    ZipFileList zf;

#ifdef _WIN32
	// replace with the path/file wildcard of your choice
        std::string home = std::string("C:/Documents and Settings/All Users/Documents");
        std::string result_dir = home + std::string("/testresult");
//        CreateDirectoryA(result_dir.c_str(), NULL);
        CreateDirectoryA(result_dir.c_str(), NULL);
        std::string zipfile = result_dir + std::string("/test.zip");
        std::string source_dir = home + std::string("/Testfiles");
        if (boinc_filelist(source_dir.c_str(), ".txt", &zf) && zf.size()) {
            retval = boinc_zip(ZIP_IT, zipfile, &zf);
            retval = boinc_zip(UNZIP_IT, zipfile, result_dir.c_str());
    }
#else
        std::string home = std::string(getenv("HOME"));
        std::string result_dir = home + std::string("/testresult");
        mkdir(result_dir.c_str(), 0777);
        std::string zipfile = result_dir + std::string("/test.zip");
        std::string source_dir = home + std::string("/Testfiles");
        if (boinc_filelist(source_dir.c_str(), ".txt", &zf) && zf.size()) {
            retval = boinc_zip(ZIP_IT, zipfile, &zf);
            retval = boinc_zip(UNZIP_IT, zipfile, result_dir.c_str());
    }
#endif

   return retval;
}
