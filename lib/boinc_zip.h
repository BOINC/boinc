// this is the "wrapper" header file for the zip/unzip functions to expose to BOINC clients
// CMC - 18/03/2004, Oxford University for BOINC project
// released under the BOINC license

// note that I've disabled zip encryption to try and simplify things
// (zip encryption is fairly weak and easy to break anyway)

#include "../unzip/unzip.h"
#include "../zip/zip.h"

// forward declarations for boinc_zip functions
// note it's basically like running zip/unzip, just comprise an argc/argv
// send in an input file or path wildcard, output filename, and basic options

// default options for zip (bZip = true) are "-j9q" which is 
// DON'T recurse subdirectories, best compression, quiet operation
// call it with bZip = ZIP to zip, bZip = UNZIP to unzip (duh)

#define ZIP_IT   1
#define UNZIP_IT 0


#ifdef __cplusplus
extern "C" {
#endif
int boinc_zip(int bZip, const char *fileIn, const char *fileOut, const char *options);
#ifdef __cplusplus
}
#endif
