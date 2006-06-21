/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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

/* End of condor_utils.c */
