// this is the "wrapper" file for the zip/unzip functions to expose to BOINC clients

#include "boinc_zip.h"

// send in an input file or path wildcard, output filename, and basic options

#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

int boinc_zip(int bZip, const char *fileIn, const char *fileOut, const char *options)
{
	int carg;
	char** av;
	int iRet = 0, i = 0;

	// if unzipping but no file out, so it just uses cwd, only 3 args
	if (bZip == UNZIP_IT && !fileOut)
		carg = 3;
	else
		carg = 4;

	// make a dynamic array
	av = (char**) calloc(4, sizeof(char*));
	for (i=0;i<carg;i++)
		av[i] = (char*) calloc(_MAX_PATH,sizeof(char));

	// just form an argc/argv to spoof the "main"
	// default options are to recurse into directories
	if (options && strlen(options)) 
		strcpy(av[1], options);

	if (bZip == ZIP_IT)
	{
		strcpy(av[0], "zip");
		// default zip options -- no dir names, no subdirs, highest compression, quiet mode
		if (strlen(av[1])==0)
			strcpy(av[1], "-j9q");
		strcpy(av[2], fileOut);
		strcpy(av[3], fileIn);
	}
	else 
	{
		strcpy(av[0], "unzip");
		// default unzip options -- preserve subdirs, overwrite 
		if (strlen(av[1])==0)
			strcpy(av[1], "-o");
		strcpy(av[2], fileIn);

		// if they passed in a directory unzip there
		if (carg == 4)
			sprintf(av[3], "-d%s", fileOut);
	}
	//strcpy(av[carg-1], "");  // null arg	

	if (bZip == ZIP_IT)
	{
		if (access(fileOut, 0) == 0)
		{ // old zip file exists so unlink (otherwise zip will reuse, doesn't seem to be a flag to 
			// bypass zip reusing it
			unlink(fileOut);   
		}
		iRet = zip_main(carg, av);
	}
	else
		iRet = unzip_main(carg, av);

	for (i=0;i<carg;i++)
		free(av[i]);
	free(av);
	return iRet;
}


/*
// this is a test of the boinc_zip/unzip Info-Zip wrapper

int main()
{
	// replace with the path/file wildcard of your choice
	boinc_zip(ZIP_IT, "e:\\temp\\*.vxd", "e:\\temp\\test.zip", NULL);

	//boinc_zip(UNZIP_IT, "e:\\temp\\test.zip", NULL, NULL);
	boinc_zip(UNZIP_IT, "e:\\temp\\test.zip", "\\\\snowdon\\Writtable\\", NULL);

	return 0;
}

*/