/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <glib.h>

#include "dc.h"

#include "condor_file.h"


DC_PhysicalFile *
_DC_create_physical_file(const char *label, const char *path)
{
	DC_PhysicalFile *file;

	file= g_new(DC_PhysicalFile, 1);
	file->label= g_strdup(label);
	file->path= g_strdup(path);
	file->mode= DC_FILE_REGULAR;
	DC_log(LOG_DEBUG, "Physical file %s created for %s",
	       path, label);

	return(file);
}


void
_DC_destroy_physical_file(DC_PhysicalFile *file)
{
	if (!file)
		return;

	DC_log(LOG_DEBUG, "Physical file %s destroying",
	       file->label);
	g_free(file->label);
	g_free(file->path);
	g_free(file);
}


/* End of condor/condor_file.c */
