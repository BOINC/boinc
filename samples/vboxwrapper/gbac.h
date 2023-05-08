
#ifndef _GBAC_H_
#define _GBAC_H_

#include <vector>
#include <string>
#include <zlib.h>

using namespace std;

class GBAC
{
 private:
    char *hostdir;
    // TODO: this is actually not needed, '/' works on windows as well
    char *dirsep;
    vector<string> environment;
    char **argv;
    int argc;
    int doGunzip(const char* strGZ, const char* strInput, bool bKeep = true);
    int hasEnding(std::string const &fullString, std::string const &ending);

 public:
    GBAC();
    ~GBAC();
    int init(int argc_, char **argv_);
    int parse(const char* file);
    int prepareHostSharedDir();
    int copyOutputFiles();
    int copyLogFiles();
	int copyDebugLog();
    int getExitStatus(int &status);
    int prepareVa(std::string &strVaFilename);
    int printVersion();
};

extern GBAC gbac;

#endif
