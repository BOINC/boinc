boinc_zip -- any questions/comments please email carlc@comlab.ox.ac.uk

boinc_zip is used on the climateprediction.net project to compress our 
large input/output files.  It is provided to BOINC developers as part 
of the package, although it is not a "mandatory" component.  It is 
based on the "Info-Zip" libraries, but combines both zip & unzip
functionality in one library.  (http://www.info-zip.org)

Basically, it will allow you to build a library that you can link 
against to provide basic zip/unzip compression functionality.  It 
should only add a few hundred KB to your app (basically like 
distributing zip & unzip exe's for different platforms).

Limitations:  the "unzip" functionality is there, that is you can unzip
a file and it will create all directories & files in the zip file.  
The "zip" functionality has some limitations due to the cross-platform
nature:  mainly it doens't provide zipping recursively (i.e. 
subdirectories); and wildcard handling is done using the "boinc_filelist" 
function which will be explained below.

Building:  For Windows, you can just add the project "boinc_zip" to your 
Visual Studio "Solution" or "Workspace."  Basically just "Insert Existing 
Project" from the Visual Studio IDE, navigate over to the boinc/zip 
directory, and it should load the appropriate files.  You can then build 
"Debug" and "Release" versions of the library.  Then just add the 
appropriate reference to "boinc_zip.lib" (Release build) or "boinc_zipd.lib"
(Debug build) in your app.

For Linux & Mac, you should be able to run "./configure" and then do a "make"
to build the "libboinc_zip.a" lib that you will link against.  In extreme
cases, you may need to do an "aclocal && autoconf && automake" first, 
to build properly for your platform.

Also, please note that boinc_zip relies on some BOINC functions that you 
will need (and will most likely be in your app already since they are handy)
 -- namely boinc/lib/filesys.C and boinc/lib/util.C

Using:
Basically, you will need to #include "boinc_zip.h" in your app (of course 
your compiler will need to know where it is, i.e. -I../boinc/zip).

Then you can just call the function "boinc_zip" with the appropriate arguments
to zip or unzip.  There are three overridden boinc_zip's provided:

int boinc_zip(int bZipType, const std::string szFileZip, 
     const ZipFileList* pvectszFileIn);
int boinc_zip(int bZipType, const std::string szFileZip, 
     const std::string szFileIn);
int boinc_zip(int bZipType, const char* szFileZip, const char* szFileIn);

bZipType is ZIP_IT or UNZIP_IT (self-explanatory)

szFileZip is the name of the zip file to create or extract
(I assume the user will provide it with the .zip extension)

The main differences are in the file parameter.  The zip library used was 
exhibiting odd behavior when "coexisting" with unzip, particularly in the 
wildcard handling.  So a function was made that creates a "ZipFileList" class, 
which is basically a vector of filenames.  If you are just compressing a 
single file, you can use either the std::string or const char* szFileIn overrides.  

You can also just pass in a "*" or a "*.*" to zip up all files in a directory.

To zip multiple files in a "mix & match" fashion, you can use the boinc_filelist
function provided.  Basically, it's a crude pattern matching of files in a
directory, but it has been useful for us on the CPDN project.  Just create a 
ZipFileList instance, and then pass this into boinc_filelist as follows:

bool boinc_filelist(const std::string directory,
                  const std::string pattern,
                  ZipFileList* pList, 
		  const unsigned char ucSort = SORT_NAME | SORT_DESCENDING,
		  const bool bClear = true);

if you want to zip up all text (.txt) files in a directory, just pass in:
the directory as a std::string, the pattern, i.e. ".txt", &yourZipList

The last two flags are the sort order of the file list (CPDN files need to be
in a certain order -- descending filenames, which is why that's the default).
The default is to "clear" your list, you can set that to "false" to keep adding
files to your "ZipFileList".

When you have created your "ZipFileList" just pass that pointer to boinc_zip.
You will be able to add files in other directories this way.

There is a "ziptest" Project for Windows provided to experiment, which can 
also be run (the "ziptest.cpp") on Unix & Mac to experiment 
with how boinc_zip work (just g++ with the boinc/lib/filesys.C & util.C as
described above).

NB -- this library can now "co-exist" with zlib (libz) as of 19/08/2005
 took out the USE_ZLIB -- causing conflicts, so now this InfoZip based boinc_zip is "independent" of any zlib use/linkage you may have/need.