#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "defines.h"
#include "dc.h"
#include "get_out.h"

static char *extract_id(const char *wu_name);

static char *get_path(void)
{
	char buf[1024];

	getcwd(buf, sizeof(buf));
	return strdup(buf);
}

static int exit_code(char *filename)
{
        FILE *log;
        char line[512];
        char *tmp1, *tmp2;
	int exitcode = 0;
	//int i;

	log = fopen(filename, "r");
	if(log != NULL)

	{	
		while(!feof(log))
		{ 
			fgets(line, sizeof(line), log);
			tmp1 = strdup(line);
			tmp1 = strtok(tmp1, " ");
			// search for a line that contains 'terminated.'
			while((tmp1 = strtok(NULL, " ")) != NULL)
			{
				if(strcmp(tmp1, "terminated.\n") == 0)
				{
					// if found, than read the line coming after
					fgets(line, sizeof(line), log);
                                        tmp1 = strdup(line);
					tmp1 = strtok(tmp1, " ");
					tmp2 = NULL;
					while((tmp1 = strtok(NULL, " ")) != NULL)
					{
					       tmp2 = strdup(tmp1);
					}
					exitcode = atoi(tmp2);
					DC_log(LOG_DEBUG,"GETOUT: ***exitcode: %d,  log: %s;", exitcode, tmp2);
				}
			}
		}
		fclose(log);
	}
	return exitcode;

}


int get_out(const char *wu_name, const char *work_dir,
	char *output_dir[1], char *result_name[1],
        char *std_out[1], char *std_err[1], char *sys_log[1], int *exitcode, int IsFinished)
{ 

    char *pwd;
    int i;
    char *id;
    char outputdir[512];
    char file[128];
    FILE *exist;
    
    FILE *clgr_getout;
    char command[512];
    char result[512];
    
    DC_log(LOG_DEBUG,"GET_OUT: started..."); 
    
    pwd = get_path();

    chdir(work_dir);

    if(IsFinished == GETOUT_FINISHED)
	/* SATATE=FINISHED, get_out results and delete from clustergrid  */
	snprintf(command, 512, "clgr_getout -t -d -i %s", wu_name);
    else
	/* SATATE!=FINISHED, get_out results BUT DO NOT delete from the clustergrid */
	snprintf(command, 512, "clgr_getout -t -i %s", wu_name);
	
    clgr_getout = popen(command, "r");

    if(clgr_getout == NULL)
    {
	    chdir(pwd);
	    DC_log(LOG_ERR,"Cannot make 'clgr_getout' command.");
	    return DC_ERROR;
    }
    fgets(result, sizeof(result), clgr_getout);	
    pclose(clgr_getout);

    /* search for 'ok' after the third space, string 'result' is NOT modified */
    id = strdup(result);
    id = strtok(result, " ");
    id = strtok(NULL, " ");
    id = strtok(NULL, " ");
    id = strtok(NULL, " ");    

    if(!strcmp(id, "ok"))
    {
	chdir(pwd);
	DC_log(LOG_ERR,"Clgr_getout error message: '%s'", result);
	return DC_ERROR;
    }
        
    DC_log(LOG_DEBUG,"%s job finished.",wu_name);

    i = 0;
    id = (char *)extract_id(wu_name);
    result_name[0] = strdup(wu_name);

    DC_log(LOG_DEBUG,"GET_OUT: result_name: %s", result_name[0]);

    sprintf(outputdir, "%s/output", work_dir);

    chdir(pwd);
    
    if(chdir(outputdir) != 0)
    {
	DC_log(LOG_ERR,"GET_OUT: Cannot move into %s dir.",outputdir);
	return DC_ERROR;
    }
    
    output_dir[0] = outputdir;
    DC_log(LOG_DEBUG,"***output_dir: %s",output_dir[0]);
    
    /* .out file */
    sprintf(file, "%s.out", id);
    exist = fopen(file, "r");
    if(exist != NULL)
    {
	std_out[0] = strdup(file);
	fclose(exist);
    }
    else
    {
	DC_log(LOG_DEBUG, "GET_OUT: no subresult output file found!");
        std_out[0] = NULL;
    }
    
    /* .err file */
    exist = NULL;
    sprintf(file, "%s.err", id);
    exist = fopen(file, "r");
    if(exist != NULL)
    {
	std_err[0] = strdup(file);
	fclose(exist);
    }
    else
    {
	DC_log(LOG_DEBUG, "GET_OUT: no subresult error file found!");
        std_err[0] = NULL;
    }
    
    /* .log file */
    exist = NULL;
    sprintf(file, "%s.log", id);
    exist = fopen(file, "r");
    if(exist != NULL)
    {
	sys_log[0] = strdup(file);
	fclose(exist);
	*exitcode = exit_code(file);
    }
    else
    {
	DC_log(LOG_DEBUG, "GET_OUT: no subresult log file found!");
        sys_log[0] = NULL;
    }

    chdir(pwd);
    free(pwd);

    return 0;
}

static char *extract_id(const char *wu_name)
{
    int i;
    char* id;

    /* id is the last 36 characters of the wu_name */
    id = (char *)strrchr(wu_name, '\0');
    for(i=0;i<36;i++)
    {
	id--;
    }
    return id;    
}
