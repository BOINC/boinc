#include <glib.h>
#include <stdio.h>
#include <string.h>

#include "dc.h"
#include "dc_common.h"

#include "condor_wu.h"


/*****************************************************************************/

/* Check if the logical name is not already registered */
int wu_check_logical_name(DC_Workunit *wu, const char *logicalFileName)
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


char *wu_get_workdir_path(DC_Workunit *wu,
			  const char *label,
			  WorkdirFile type)
{
  return g_strdup_printf("%s%c%s", wu->workdir, G_DIR_SEPARATOR, label);
}


int wu_gen_condor_submit(DC_Workunit *wu)
{
  FILE *f= NULL;
  GString *fn;

  if (!wu)
    return(-2);

  fn= g_string_new(wu->workdir);
  fn= g_string_append(fn, "/condor_submit.txt");
  if ((f= fopen(fn->str, "w+")) == NULL)
    {
      DC_log(LOG_ERR, "Condor submit file %s creation failed",
	     fn->str);
      return(-1);
    }
  
  fprintf(f, "Executable = %s\n", wu->client_name);
  fprintf(f, "Universe = vanilla\n");
  fprintf(f, "output = internal_output.txt\n");
  fprintf(f, "error = internal_error.txt\n");
  fprintf(f, "log = internal_log.txt\n");
  fprintf(f, "\n");
  fprintf(f, "Queue\n");

  if (f)
    fclose(f);
  return(0);
}


/* End of condor/condor_wu.c */
