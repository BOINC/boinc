/*
 * condor/condor_utils.c
 *
 * DC-API usefull functions
 *
 * (c) Daniel Drotos, 2006
 */

#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include "dc_internal.h"
#include "dc_common.h"

#include "condor_utils.h"
#include "condor_common.h"


void
_DC_init_utils(void)
{
}


int
_DC_mkdir_with_parents(char *dn, mode_t mode)
{
	char *p= dn;
	int res;

	/*fprintf(stderr, "making %s (EEXIST=%d)\n", dn, EEXIST);*/
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
			{
				/*fprintf(stderr, "mkdir(%s,%d)=",dn,mode);*/
				res= mkdir(dn, mode);
				/*fprintf(stderr, "%d %d\n", res, errno);*/
			}
			*p= '/';
			p++;
		}
	}
	/*fprintf(stderr, "mkdir(%s,%d)=",dn,mode);*/
	res= mkdir(dn, mode);
	/*fprintf(stderr, "%d %d\n", res, errno);*/
	if (res != 0 &&
	    errno == EEXIST)
		return(DC_OK);
	return(res?DC_ERR_SYSTEM:DC_OK);
}


static int
_DC_rmsubdir(char *name)
{
	DIR *d;
	struct dirent *de;
	int i= 0;

	if ((d= opendir(name)) == NULL)
	{
		DC_log(LOG_DEBUG, "Open %s: %s", name, strerror(errno));
		return(0);
	}
	while ((de= readdir(d)))
	{
		if (strcmp(de->d_name, ".") != 0 &&
		    strcmp(de->d_name, "..") != 0)
		{
			char *s= malloc(strlen(name)+strlen(de->d_name) + 10);
			strcpy(s, name);
			strcat(s, "/");
			strcat(s, de->d_name);
			i+= _DC_rm(s);
			free(s);
		}
	}
	closedir(d);
	if (rmdir(name))
	{
		DC_log(LOG_ERR, "Failed to rmdir %s: %s",
		       name, strerror(errno));
	}
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
	{
		return(0);
	}
	if (S_ISDIR(s.st_mode) &&
	    strcmp(name, ".") != 0 &&
	    strcmp(name, "..") != 0)
	{
		return(_DC_rmsubdir(name));
	}
	else
		{
			if (remove(name))
				DC_log(LOG_ERR, "Failed to remove %s: %s",
				       name, strerror(errno));
		}
	return(1);
}


int
_DC_file_exists(char *fn)
{
	return(access(fn, F_OK) == 0);
}


int
_DC_file_empty(char *fn)
{
	struct stat s;
	
	if (stat(fn, &s))
		return(1);
	return(s.st_size == 0);
}


int
_DC_create_file(char *fn, char *what)
{
	FILE *f= fopen(fn, "w");
	if (f)
	{
		if (what)
			fprintf(f, "%s", what);
		fclose(f);
		return(DC_OK);
	}
	else
		return(DC_ERR_SYSTEM);
}


char *
_DC_get_file(char *fn)
{
	FILE *f;
	char *buf= NULL;

	if ((f= fopen(fn, "r")) != NULL)
	{
		int bs= 100, i;
		int c;

		buf= malloc(bs);
		i= 0;
		buf[i]= '\0';
		while ((c= fgetc(f)) != EOF)
		{
			if (i > bs-2)
			{
				bs+= 100;
				buf= realloc(buf, bs);
			}
			buf[i]= (char)c;
			i++;
		}
		buf[i]= '\0';
		fclose(f);
	}
	return(buf);
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
		{
			if (_DC_copyFile(msgfile, fn))
				DC_log(LOG_ERR, "copyFile(%s,%s) error",
				       msgfile, fn);
		}
		else
		{
			DC_log(LOG_WARNING, "No message specified for "
			       "creation");
		}
		if (fsync(fileno(f)) != 0)
			DC_log(LOG_ERR, "Sync of %s: %d %s", fn,
			       errno, strerror(errno));
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
	DC_log(LOG_DEBUG, "Message created: %s", fn2);
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
	s= malloc(100 + (name?strlen(name):strlen(_DCAPI_MSG_MESSAGE)));
	strcpy(s, name?name:_DCAPI_MSG_MESSAGE);
	strcat(s, ".");
	while ((de= readdir(d)) != NULL)
	{
		char *found= strstr(de->d_name, s);
		if (found == de->d_name)
			nuof++;
	}
	if (d)
		closedir(d);
	free(s);
	return(nuof);
}


char *
_DC_message_name(char *box, char *name)
{
	char *dn, *n;
	DIR *d;
	struct dirent *de;
	int min_id;

	if (!box ||
	    !name)
		return(NULL);
	if (!*box ||
	    !*name)
		return(NULL);
	/*DC_log(LOG_DEBUG, "_DC_message_name(%s,%s)", box, name);*/
	dn= strdup(box);
	n= malloc(strlen(name)+10);
	strcpy(n, name);
	strcat(n, ".");
	/*DC_log(LOG_DEBUG, "opendir(%s)=", dn);*/
	d= opendir(dn);
	/*DC_log(LOG_DEBUG, "%p", d);*/
	min_id= -1;
	while (d &&
	       (de= readdir(d)) != NULL)
	{
		char *found= strstr(de->d_name, n);
		/*DC_log(LOG_DEBUG, "%s <-> %s", de->d_name, n);*/
		if (found == de->d_name)
		{
			char *pos= strrchr(de->d_name, '.');
			/*DC_log(LOG_DEBUG, "found");*/
			if (pos)
			{
				int id= 0;
				pos++;
				id= strtol(pos, NULL, 10);
				/*DC_log(LOG_DEBUG, "id=%d", id);*/
				if (id > 0)
				{
					if (min_id < 0)
						min_id= id;
					else
						if (id < min_id)
							min_id= id;
				}
			}
		}
		/*else
		  DC_log(LOG_DEBUG, "not found");*/
	}
	if (d)
		closedir(d);
	if (min_id >= 0)
	{
		dn= realloc(dn, strlen(box)+strlen(name)+100);
		sprintf(dn, "%s/%s.%d", box, name, min_id);
		free(n);
		/*DC_log(LOG_DEBUG, "return: %s", dn);*/
		return(dn);
	}
	if (dn)
		free(dn);
	if (n)
		free(n);
	/*DC_log(LOG_DEBUG, "return null");*/
	return(NULL);
}


char *
_DC_read_message(char *box, char *name, int del_msg)
{
	FILE *f;
	char *fn= _DC_message_name(box, name);
	char *buf= NULL;

	if (fn==NULL)
		return(NULL);
	/*DC_log(LOG_DEBUG, "Reading message from %s", fn);*/
	if ((f= fopen(fn, "r")) != NULL)
	{
		int bs= 100, i;
		int c;

		buf= malloc(bs);
		i= 0;
		buf[i]= '\0';
		while ((c= fgetc(f)) != EOF)
		{
			if (i > bs-2)
			{
				bs+= 100;
				buf= realloc(buf, bs);
			}
			buf[i]= (char)c;
			i++;
		}
		buf[i]= '\0';
		fclose(f);
		if (del_msg)
			unlink(fn);
	}
	if (fn!=NULL)
		free(fn);
	return(buf);
}


char *
_DC_quote_string(char *str)
{
	int i, n;
	char *s, *p;

	if (!str)
	{
		s= strdup("\\nnn");
		return(s);
	}
	if (!*str)
	{
		s= strdup("\\000");
		return(s);
	}

	for (i= n= 0; str[i]; i++)
	{
		if (!isalnum(str[i]))
			n++;
	}

	s= calloc(strlen(str)+10*n+10, 1);
	for (i= 0, p= s; str[i]; i++)
	{
		if (isalnum(str[i]))
		{
			*p= str[i];
			p++;
		}
		else
		{
			sprintf(p, "\\%03o", str[i]);
			p+= strlen(p);
		}
	}
	return(s);
}


char *
_DC_unquote_string(char *str)
{
	char *s, *p, *i;

	if (!str)
		return(NULL);
	s= calloc(strlen(str)+1, 1);
	p= str;
	i= s;
	while (*p != '\0')
	{
		if (*p != '\\')
			*i= *p;
		else
		{
			char c, *x= p+4;
			int l;
			c= *x;
			*x= '\0';
			if (strcmp(p+1, "nnn") == 0)
			{
				free(s);
				return(NULL);
			}
			l= strtol(p+1, 0, 8);
			*i= (char)l;
			*x= c;
			p= x-1;
		}
		p++;
		i++;
	}
	return(s);
}


/* End of condor_utils.c */

/* Local variables: */
/* c-file-style: "linux" */
/* End: */
