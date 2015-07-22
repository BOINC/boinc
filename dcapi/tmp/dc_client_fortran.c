#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <dc_client.h>

/********************************************************************
 * Helper Functions
 */

// Fortran strings are passed as character array which is not null-terminated plus length
//
static char *string_from_fortran(const char *string, int string_len)
{
	char *p;

	p = (char *)malloc(string_len + 1);
	memcpy(p, string, string_len);
	p[string_len] = 0;

	return p;
}

// remove terminating null and pad with blanks for FORTRAN
//
static void string_to_fortran(char *string, int string_len)
{
	int i;
	for (i = strlen(string); i < string_len; i++)
	{
		string[i] = ' ';
	}
}

// remove whitespace from start and end of a string
//
void strip_whitespace(char *string)
{
	int n;

	while (1)
	{
		if (!string[0]) break;
		if (!isascii(string[0])) break;
		if (!isspace(string[0])) break;
		strcpy(string, string+1);
	}

	while (1)
	{
		n = (int)strlen(string);
		if (n == 0) break;
		if (!isascii(string[n-1])) break;
		if (!isspace(string[n-1])) break;
		string[n-1] = 0;
	}
}

/********************************************************************
 * FORTRAN DC Client API Functions
 */

void dc_initclient_(int* retval)
{
	*retval = DC_initClient();
}

char *DC_resolveFileNameAuto(const char *logicalFileName)
{
	char *filename;

	filename = DC_resolveFileName(DC_FILE_IN, filename);
	if (!filename)
	{
		DC_resolveFileName(DC_FILE_TMP, filename);
		if (!filename)
		{
			DC_log(LOG_ERR, "Resolving file '%s' failed!", filename);
			return NULL;
		}
		DC_log(LOG_DEBUG, "file %s resolved by DC_FILE_TMP", filename);
		return filename;
	}
	DC_log(LOG_DEBUG, "file %s resolved by DC_FILE_IN", filename);
	return filename;
}

void dc_resolvefilename_(int* fileType, const char *logicalFileName, char *physicalFileName,
					int logicalFileNameLen, int physicalFileNameLen)
{
	char *filename;

	filename = string_from_fortran(logicalFileName, logicalFileNameLen);
	strip_whitespace(filename);

	switch(*fileType)
	{
		case 1:
			physicalFileName = DC_resolveFileName(DC_FILE_IN, filename);
			break;
		case 2:
			physicalFileName = DC_resolveFileName(DC_FILE_OUT, filename);
			break;
		case 3:
			physicalFileName = DC_resolveFileName(DC_FILE_TMP, filename);
			break;
		case 4:
			physicalFileName = DC_resolveFileNameAuto(filename);
			break;
		default:
			DC_log(LOG_ERR, "Undefined DC_FileType in DC_resolveFileName_ function call.");	
			DC_log(LOG_ERR, "DC_FileType: %d, logicalFileName: %s",	*fileType, filename);
			if (filename)
				free(filename);
			exit(1);
	}

	string_to_fortran(physicalFileName, physicalFileNameLen);
	if(filename)
		free(filename);
}

void dc_sendresult_(const char *logicalFileName, const char *path,
        int* fileMode, int logicalFileNameLen, int pathLen, int* retval)
{
	char *filename;
	char *filePath;

	filename = string_from_fortran(logicalFileName, logicalFileNameLen);
	strip_whitespace(filename);
	filePath = string_from_fortran(path, pathLen);
	strip_whitespace(filePath);

	switch(*fileMode)
	{
		case 1:
			*retval = DC_sendResult(filename, filePath, DC_FILE_REGULAR);
			break;
		case 2:
			*retval = DC_sendResult(filename, filePath, DC_FILE_PERSISTENT);
			break;
		case 3:
			*retval = DC_sendResult(filename, filePath, DC_FILE_VOLATILE);
			break;
		default:
			DC_log(LOG_ERR, "Undefined DC_FileMode in DC_sendResult function call.");	
			DC_log(LOG_ERR, "DC_FileMode: %d, logicalFileName: %s, path: %s",
					*fileMode, filename, filePath);
			if (filename)
				free(filename);
			if (filePath)
				free(filePath);
			exit(1);
	}
	if (filename)
		free(filename);
	if (filePath)
		free(filePath);
}

void dc_sendmessage_(const char *message, int messageLen, int* retval)
{
	char *msg;

	msg = string_from_fortran(message, messageLen);
	strip_whitespace(msg);
	*retval = DC_sendMessage(msg);
	if(msg)
		free(msg);
}

void dc_checkclientevent_(void)
{
	// XXX not impl yet
}

void dc_destroyclientevent_(void)
{
	// XXX not impl yet
}

void dc_checkpointmade_(const char *filename, int filename_len)
{
	char *file;

	file = string_from_fortran(filename, filename_len);
	strip_whitespace(file);
	DC_checkpointMade(file);
	if (file)
		free(file);
}

void dc_fractiondone_(double* fraction)
{
	DC_fractionDone(*fraction);
}

void dc_finishclient_(int* exitcode)
{
	DC_finishClient(*exitcode);
}

void dc_getmaxmessagesize_(int* retval)
{
	*retval = DC_getMaxMessageSize();
}

void dc_getmaxsubresults_(int* retval)
{
	*retval = DC_getMaxSubresults();
}

void dc_getgridcapabilities_(void)
{
	// XXX not impl yet
}

void dc_getcfgstr_(const char *name, char *string, int nameLen, int stringLen)
{
	char *key;
	char *value;

	key = string_from_fortran(name, nameLen);
	strip_whitespace(key);
	string = DC_getCfgStr(key);
	string_to_fortran(string, stringLen);
	if (key)
		free(key);
}

void dc_getcfgint_(const char *name, int* defaultValue, int nameLen, int* retval)
{
	char *key;

	key = string_from_fortran(name, nameLen);
	strip_whitespace(key);
	*retval = DC_getCfgInt(key, *defaultValue);
	if (key)
		free(key);
}

void dc_getcfgbool_(const char *name, int* defaultValue, int nameLen, int* retval)
{
	char *key;

	key = string_from_fortran(name, nameLen);
	strip_whitespace(key);
	*retval = DC_getCfgBool(key, *defaultValue);
	if (key)
		free(key);
}
