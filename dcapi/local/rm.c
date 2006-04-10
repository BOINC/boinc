#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>

#include "dc_local.h"

static GHashTable *wu_table;

/********************************************************************
 * Functions
 */

DC_Workunit *DC_createWU(const char *clientName, const char *arguments[],
	int subresults, const char *tag)
{
	char uuid_str[37];
	DC_Workunit *wu;
	GString *str;
	int ret;
	char *p;

	if (subresults > MAX_SUBRESULTS){
		DC_log(LOG_ERR, "DC_createWU: The given subresult number: %d is too high. (max:%d)",
			subresults, MAX_SUBRESULTS);
		return NULL;
	}

	if (!clientName)
	{
		DC_log(LOG_ERR, "DC_createWU: clientName is not supplied");
		return NULL;
	}

	wu = g_new0(DC_Workunit, 1);

	wu->argv = g_strdupv((char **)arguments);

	for (wu->argc = 0; arguments && arguments[wu->argc]; wu->argc++)
		/* Nothing */;

	wu->subresults = subresults;
	wu->tag = g_strdup(tag);

	uuid_generate(wu->uuid);
	uuid_unparse_lower(wu->uuid, uuid_str);
	wu->uuid_str = g_strdup(uuid_str);

	if (tag)
		wu->name = g_strdup_printf("%s_%s_%s", project_uuid_str,
			uuid_str, tag);
	else
		wu->name = g_strdup_printf("%s_%s", project_uuid_str, uuid_str);

	/* Calculate & create the working directory. The working directory
	 * has the form:
	 * <project work dir>/.dcapi-<project uuid>/<hash>/<wu uuid>
	 * Where <hash> is the first 2 hex digits of the uuid
	 */
	str = g_string_new(_DC_getCfgStr(CFG_WORKDIR));
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, ".dcapi-");
	g_string_append(str, project_uuid_str);
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append_printf(str, "%02x", wu->uuid[0]);
	g_string_append_c(str, G_DIR_SEPARATOR);
	g_string_append(str, uuid_str);

	ret = g_mkdir_with_parents(str->str, 0700);
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to create WU working directory %s: %s",
			str->str, strerror(errno));
		DC_destroyWU(wu);
		return NULL;
	}

	wu->workdir = str->str;
	g_string_free(str, FALSE);

	p = strrchr(clientName, '/');
	if (p == NULL)
        {
                wu->client_name = g_strdup(clientName);
                wu->client_path = g_strdup(".");
        }
        else
        {
                wu->client_path = g_strdup(clientName);
                wu->client_path[strlen(wu->client_path)-strlen(p)] = 0;
                p++;
                wu->client_name = g_strdup(p);
        }

	if (!wu_table)
		wu_table = g_hash_table_new_full(g_str_hash, g_str_equal,
			NULL, NULL);
	g_hash_table_insert(wu_table, wu->name, wu);

	return wu;
}

static char *get_workdir_path(DC_Workunit *wu, const char *label,
	WorkdirFile type)
{
	return g_strdup_printf("%s%c%s", wu->workdir, G_DIR_SEPARATOR, label);
}

void DC_destroyWU(DC_Workunit *wu)
{
	if (!wu)
		return;

	if (wu_table)
		g_hash_table_remove(wu_table, wu->name);

	switch (wu->state)
	{
		case DC_WU_RUNNING:
			/* XXX Abort the work unit */
			break;
		default:
			break;
	}

	while (wu->input_files)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)wu->input_files->data;

		unlink(file->path);
		wu->input_files = g_list_delete_link(wu->input_files,
			wu->input_files);
		_DC_destroyPhysicalFile(file);
	}

	while (wu->output_files)
	{
		char *name = (char *)wu->output_files->data;
		char *file = g_strdup_printf("%s%c%s", wu->workdir, G_DIR_SEPARATOR, name);
		unlink(file);
		g_free(file);

		g_free(wu->output_files->data);
		wu->output_files = g_list_delete_link(wu->output_files,
			wu->output_files);
	}

/*	if (wu->ckpt_name)  // XXX
	{
		//char *path = get_workdir_path(wu, wu->ckpt_name, FILE_CKPT);
		char *path = g_strdup_printf("%s%c%s", wu->workdir, G_DIR_SEPARATOR, DC_CHECKPOINT_FILE);
		unlink(path);
		g_free(path);
		//g_free(wu->ckpt_name);
	}
*/
	if (wu->client_name)
	{
		char *path = g_strdup_printf("%s%c%s", wu->workdir, G_DIR_SEPARATOR, wu->client_name);
		unlink(path);
		g_free(path);
		g_free(wu->client_name);
		g_free(wu->client_path);
	}

	if (wu->workdir)
	{
		const char *name;
		GDir *dir;
		int ret;

		dir = g_dir_open(wu->workdir, 0, NULL);
		/* The work directory should not contain any extra files, but
		 * just in case */
		while (dir && (name = g_dir_read_name(dir)))
		{
			GString *str = g_string_new(wu->workdir);
			g_string_append_c(str, G_DIR_SEPARATOR);
			g_string_append(str, name);
			DC_log(LOG_INFO, "Removing unknown file %s",
				str->str);
			unlink(str->str);
			g_string_free(str, TRUE);
		}
		if (dir)
			g_dir_close(dir);

		ret = rmdir(wu->workdir);
		if (ret)
			DC_log(LOG_WARNING, "Failed to remove WU working "
				"directory %s: %s", wu->workdir,
				strerror(errno));
		g_free(wu->workdir);
	}

	g_free(wu->uuid_str);
	g_strfreev(wu->argv);
	g_free(wu->tag);
	g_free(wu->name);

	g_free(wu);
}

/* Check if the logical name is not already registered */
static int check_logical_name(DC_Workunit *wu, const char *logicalFileName)
{
	GList *l;

	if (strchr(logicalFileName, '/') || strchr(logicalFileName, '\\'))
	{
		DC_log(LOG_ERR, "Illegal characters in logical file name %s",
			logicalFileName);
		return DC_ERR_BADPARAM;
	}
	for (l = wu->input_files; l; l = l->next)
	{
		DC_PhysicalFile *file = (DC_PhysicalFile *)l->data;

		if (!strcmp(file->label, logicalFileName))
		{
			DC_log(LOG_ERR, "File %s is already registered as an "
				"input file", logicalFileName);
			return DC_ERR_BADPARAM;
		}
	}
	for (l = wu->output_files; l; l = l->next)
	{
		if (!strcmp((char *)l->data, logicalFileName))
		{
			DC_log(LOG_ERR, "File %s is already registered as an "
				"output file", logicalFileName);
			return DC_ERR_BADPARAM;
		}
	}
	return 0;
}

#define COPY_BUFSIZE 65536
static int copy_file(const char *src, const char *dst)
{
	int sfd, dfd;
	ssize_t ret;
	char *buf;

	buf = (char *)g_malloc(COPY_BUFSIZE);
	sfd = open(src, O_RDONLY);
	if (sfd == -1)
	{
		DC_log(LOG_ERR, "Failed to open %s for copying: %s", src,
			strerror(errno));
		g_free(buf);
		return -1;
	}
	dfd = open(dst, O_WRONLY | O_CREAT | O_TRUNC);
	if (dfd == -1)
	{
		DC_log(LOG_ERR, "Failed to create %s: %s", dst, strerror(errno));
		g_free(buf);
		close(sfd);
		return -1;
	}

	while ((ret = read(sfd, buf, COPY_BUFSIZE)) > 0)
	{
		char *ptr = buf;
		while (ret)
		{
			ssize_t ret2 = write(dfd, ptr, ret);
			if (ret2 < 0)
			{
				DC_log(LOG_ERR, "Error writing to %s: %s", dst,
					strerror(errno));
				close(sfd);
				close(dfd);
				unlink(dst);
				g_free(buf);
				return -1;
			}
			ret -= ret2;
			ptr += ret2;
		}
	}

	if (ret < 0)
	{
		DC_log(LOG_ERR, "Error reading from %s: %s", src, strerror(errno));
		close(sfd);
		close(dfd);
		g_free(buf);
		unlink(dst);
		return -1;
	}

	g_free(buf);
	close(sfd);
	if (close(dfd))
	{
		DC_log(LOG_ERR, "Error writing to %s: %s", dst, strerror(errno));
		unlink(dst);
		return -1;
	}
	return 0;
}

int DC_addWUInput(DC_Workunit *wu, const char *logicalFileName, const char *URL,
	DC_FileMode fileMode)
{
	DC_PhysicalFile *file;
	char *workpath;
	int ret;

	/* Sanity checks */
	ret = check_logical_name(wu, logicalFileName);
	if (ret)
		return ret;

	/* XXX Check if the wu->num_inputs + wu->num_outputs + wu->subresults
	 * does not exceed the max. number of file slots */

	workpath = get_workdir_path(wu, logicalFileName, FILE_IN);
	file = _DC_createPhysicalFile(logicalFileName, workpath);
	g_free(workpath);
	if (!file)
		return DC_ERR_INTERNAL;

	switch (fileMode)
	{
		case DC_FILE_REGULAR:
			ret = copy_file(URL, file->path);
			if (ret)
			{
				_DC_destroyPhysicalFile(file);
				return DC_ERR_BADPARAM; /* XXX */
			}
			break;
		case DC_FILE_PERSISTENT:
			ret = link(URL, file->path);
			if (ret)
			{
				DC_log(LOG_ERR, "Failed to link %s to %s: %s",
					URL, file->path, strerror(errno));
				_DC_destroyPhysicalFile(file);
				return DC_ERR_BADPARAM; /* XXX */
			}
			/* Remember the file mode */
			file->mode = DC_FILE_PERSISTENT;
			break;
		case DC_FILE_VOLATILE:
			ret = rename(URL, file->path);
			if (ret)
			{
				DC_log(LOG_ERR, "Failed to rename %s to %s: %s",
					URL, file->path, strerror(errno));
				_DC_destroyPhysicalFile(file);
				return DC_ERR_BADPARAM; /* XXX */
			}
			break;
	}

	wu->input_files = g_list_append(wu->input_files, file);
	wu->num_inputs++;
	return 0;
}

int DC_addWUOutput(DC_Workunit *wu, const char *logicalFileName)
{
	int ret;

	/* Sanity checks */
	ret = check_logical_name(wu, logicalFileName);
	if (ret)
		return ret;

	/* XXX Check if the wu->num_inputs + wu->num_outputs + wu->subresults
	 * does not exceed the max. number of file slots */

	wu->output_files = g_list_append(wu->output_files,
		g_strdup(logicalFileName));
	wu->num_outputs++;
	return 0;
}

int DC_submitWU(DC_Workunit *wu)
{
	pid_t pid;
	char *old_path, *new_path;

	if (wu->state != DC_WU_READY)
	{
		DC_log(LOG_ERR, "Only WUs in READY state can be submitted");
		return DC_ERR_BADPARAM;
	}

	/* copy the exec into the workdir */
	old_path = g_strdup_printf("%s%c%s", wu->client_path, G_DIR_SEPARATOR, wu->client_name);
	new_path = g_strdup_printf("%s%c%s", wu->workdir, G_DIR_SEPARATOR, wu->client_name);
	if (link(old_path, new_path))
	{
		DC_log(LOG_ERR, "Failed to link %s to %s: %s",
			old_path, new_path, strerror(errno));
		return DC_ERR_BADPARAM;
	}
	g_free(old_path);
	g_free(new_path);

	if((pid=fork())<0) {
		DC_log(LOG_ERR,"Cannot fork!\nerrno=%d  %s\n",
			errno, strerror(errno));
		return DC_ERR_BADPARAM;
	}

	if(pid==0) /* client process */
	{
		/* change into working directory of the WU */
		if (chdir(wu->workdir))
		{
			DC_log(LOG_ERR,"Cannot cd into %s\nerrno=%d  %s\n",
			wu->workdir, errno, strerror(errno));
			return DC_ERR_BADPARAM;
		}

		/* execute the client */
		DC_log(LOG_INFO, "Execute: %s", wu->client_name);
		execv(wu->client_name, wu->argv);
		DC_log(LOG_ERR, "Cannot execute. Errno=%d  %s\n",
			errno, strerror(errno));
		exit(1);
	}

	wu->pid = pid;
	wu->state = DC_WU_RUNNING;

	return DC_OK;
}

DC_Workunit *_DC_getWUByName(const char *name)
{
	DC_Workunit *wu;
	char *uuid_str;
	uuid_t uuid;
	int ret;

	if (wu_table)
	{
		wu = (DC_Workunit *)g_hash_table_lookup(wu_table, name);
		if (wu)
			return wu;
	}

	/* Check if the WU belongs to this application */
	uuid_str = g_strndup(name, 36);
	ret = uuid_parse(uuid_str, uuid);
	g_free(uuid_str);
	if (ret)
	{
		DC_log(LOG_ERR, "WU name contains illegal UUID");
		return NULL;
	}

	if (uuid_compare(uuid, project_uuid))
	{
		DC_log(LOG_WARNING, "WU does not belong to this application");
		return NULL;
	}

	if (name[36] != '_')
	{
		DC_log(LOG_ERR, "Illegal WU name syntax");
		return NULL;
	}

	/* Check the WU's UUID */
	uuid_str = g_strndup(name + 37, 36);
	ret = uuid_parse(uuid_str, uuid);
	g_free(uuid_str);
	if (ret)
	{
		DC_log(LOG_ERR, "WU name contains illegal UUID");
		return NULL;
	}

//	return load_from_disk(uuid);

	return NULL;
}

void _DC_testWUEvents(gpointer key, DC_Workunit *wu, gpointer user_data)
{
	DC_Result *result;
	char syscmd[256];
	int retval;

	snprintf(syscmd, 256, "ps -p %d >/dev/null", wu->pid);
	retval = system(syscmd);

	/* retval == 0 means that the process is still in output of ps
		but it can be <defunct>.
		So check it again.
	*/
	if (retval == 0) {
		snprintf(syscmd, 256, "ps -p %d | grep defunct >/dev/null",
			wu->pid);
		retval = system(syscmd);
		if (retval == 0) retval = 1; /* defunct means finished */
		else if (retval == 1) retval = 0;
	}
	if (retval == 1) { /* process finished (not exists) */
		/* create the result object */
		DC_log(LOG_INFO, "Work unit %s with pid %d is found to be finished\n",
			wu->name, wu->pid);
		wu->state = DC_WU_FINISHED;

		result = _DC_createResult(wu->name);
		if (result)
		{
			_dc_resultcb(result->wu, result);
			_DC_destroyResult(result);
		}
	}

	return;
}

int _DC_searchForEvents()
{
	if (!wu_table)
	{
		DC_log(LOG_WARNING, "Searching for events is only usefull if there is any running work unit!");
		return DC_ERR_BADPARAM;
	}

	g_hash_table_foreach(wu_table, (GHFunc)_DC_testWUEvents, NULL);

	return DC_OK;
}
