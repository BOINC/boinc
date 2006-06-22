/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "dc_internal.h"
#include "dc_common.h"
#include "condor_utils.h"


int
_DC_mkdir_with_parents(char *dn, mode_t mode)
{
	char *p= dn;
	int res;

	if (!dn ||
	    !*dn)
		return(0);
	while (*p)
	{
		while (*p != '/' &&
		       *p != '\0')
			p++;
		if (*p == '/')
		{
			*p= '\0';
			if (*dn)
				res= mkdir(dn, mode);
			*p= '/';
			p++;
		}
	}
	res= mkdir(dn, mode);
	return(res);
}


static int
_DC_rmsubdir(char *name)
{
	DIR *d;
	struct dirent *de;
	int i= 0;

	if ((d= opendir(name)) == NULL)
		return(0);
	while ((de= readdir(d)))
	{
		if (strcmp(de->d_name, ".") != 0 &&
		    strcmp(de->d_name, "..") != 0)
		{
			i+= _DC_rm(de->d_name);
		}
	}
	closedir(d);
	if (rmdir(name))
		DC_log(LOG_ERR, "Failed to rmdir %s: %s",
		       name, strerror(errno));
	return(i);
}

int
_DC_rm(char *name)
{
	struct stat s;
	int i;

	if (!name ||
	    !*name)
		return(0);
	i= lstat(name, &s);
	if (i != 0)
		return(0);
	if (S_ISDIR(s.st_mode) &&
	    strcmp(name, ".") != 0 &&
	    strcmp(name, "..") != 0)
		return(_DC_rmsubdir(name));
	else
		{
			if (remove(name))
				DC_log(LOG_ERR, "Failed to remove %s: %s",
				       name, strerror(errno));
		}
	return(1);
}


static int _DC_message_id= 0;

int
_DC_create_message(char *box,
		   char *name,
		   const char *message,
		   char *msgfile)
{
	char *fn, *fn2;
	FILE *f;
	int ret= DC_OK;

	/*DC_log(LOG_DEBUG, "DC_sendMessage(%s)", message);*/
	if (!box ||
	    !name)
		return(DC_ERR_BADPARAM);
	mkdir(box, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
	_DC_message_id++;
	fn= malloc(strlen(box)+strlen(name)+100);
	sprintf(fn, "%s/%s_creating.%d", box, name, _DC_message_id);
	if ((f= fopen(fn, "w")) != NULL)
	{
		if (message)
			fprintf(f, "%s", message);
		else if (msgfile)
			ret= _DC_copyFile(msgfile, fn);
		fclose(f);
		/*DC_log(LOG_DEBUG, "Message %d created", _DC_message_id);*/
	}
	else
	{
		DC_log(LOG_ERR, "Error creating message file (%s)",
		       fn);
		ret= DC_ERR_SYSTEM;
	}
	fn2= malloc(strlen(box)+strlen(name)+100);
	sprintf(fn2, "%s/%s.%d", box, name, _DC_message_id);
	rename(fn, fn2);
	free(fn2);
	free(fn);
	return(ret);
}


int
_DC_nuof_messages(char *box, char *name)
{
	DIR *d;
	struct dirent *de;
	int nuof= 0;
	char *s;

	if (!box)
		return(0);
	if ((d= opendir(box)) == NULL)
		return(0);
	s= malloc(name?strlen(name):20+10);
	name?strcpy(s, name):strcpy(s, "message");
	strcat(s, ".");
	while ((de= readdir(d)) != NULL)
	{
		char *found= strstr(de->d_name, s);
		if (found == de->d_name)
			nuof++;
	}
	free(s);
	closedir(d);
	return(nuof);
}


/* End of condor_utils.c */
