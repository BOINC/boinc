#include <glib.h>
#include <string.h>

#include "dc.h"

#include "condor_wu.h"


/*****************************************************************************/

/* Check if the logical name is not already registered */
int check_logical_name(DC_Workunit *wu, const char *logicalFileName)
{
  GList *l;

  if (strchr(logicalFileName, '/') || strchr(logicalFileName, '\\'))
    {
      DC_log(LOG_ERR, "Illegal characters in logical file name %s",
	     logicalFileName);
      return(DC_ERR_BADPARAM);
    }
  for (l= wu->input_files; l; l= l->next)
    {
      DC_PhysicalFile *file= (DC_PhysicalFile *)l->data;
      
      if (!strcmp(file->label, logicalFileName))
	{
	  DC_log(LOG_ERR, "File %s is already registered as an "
		 "input file", logicalFileName);
	  return(DC_ERR_BADPARAM);
	}
    }
  for (l= wu->output_files; l; l= l->next)
    {
      if (!strcmp((char *)l->data, logicalFileName))
	{
	  DC_log(LOG_ERR, "File %s is already registered as an "
		 "output file", logicalFileName);
	  return(DC_ERR_BADPARAM);
	}
    }
  return(0);
}


char *get_workdir_path(DC_Workunit *wu,
		       const char *label,
		       WorkdirFile type)
{
  return g_strdup_printf("%s%c%s", wu->workdir, G_DIR_SEPARATOR, label);
}


/* End of condor/condor_wu.c */
