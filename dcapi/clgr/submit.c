#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "defines.h"
#include "dc.h"
#include "submit.h"

static int create_work_dir(const char *work_dir_base, const char *workdir_id, const char *work_dir_num);
static int create_jobdir_structure(void);
static int create_submit_file(const char *job_name, const char *executable, const char *args);
static int copy_executables(const char *executable_dir, const char *work_dir);
static int copy_inputfiles(int num_of_infiles,const char *infiles[MAX_INFILES],
		                    const char *localfilenames[MAX_INFILES], const char *work_dir);
static char *job_submit(const char *work_dir);
static char *retreive_id(const char *result);

static char *get_path(void)
{
	char buf[1024];

	getcwd(buf, sizeof(buf));
	return strdup(buf);
}

char *submit(const char *work_dir_base, const char *work_dir_id, const char *work_dir_num,
		const char *executable_dir, const char *executable, const char *job_name, 
		const char *args, int num_of_infiles, 
		const char *infiles[MAX_INFILES], const char *localfilenames[MAX_INFILES])
{	
	char *pwd;
	char *id;
	char work_dir[512];
	sprintf(work_dir, "%s/%s/%s", work_dir_base, work_dir_id, work_dir_num);
	
	pwd = get_path();

	/* create workdir */
	if(create_work_dir(work_dir_base, work_dir_id, work_dir_num) != DC_OK)
	{
	    chdir(pwd);
	    free(pwd);
	    return NULL;
	}
	//DC_log(LOG_DEBUG,"'%s' working directory created!", work_dir);

	  /***************************/
  	 /* create jobdir structure */
	/***************************/
	if(create_jobdir_structure() != DC_OK)
	{
		chdir(pwd);
		free(pwd);
		return NULL;
	}
	//DC_log(LOG_DEBUG,"Jobdir structure created!");
	
	  /**********************/
	 /* create submit file */
	/**********************/
	if(create_submit_file(job_name, executable, args) != DC_OK)
	{
		chdir(pwd);
		free(pwd);
		return NULL;
	}

	// change back the System directory to the original
	chdir(pwd);

	  /***************************/
	 /* copy executables in bin */
	/***************************/
	if(copy_executables(executable_dir, work_dir) != DC_OK)
	{
		chdir(pwd);
		free(pwd);
		return NULL;
	}
	//DC_log(LOG_DEBUG,"Executable file(s) copied into '%s/bin'!", work_dir);

  	  /********************/
	 /* copy input files */
	/********************/
	if(copy_inputfiles(num_of_infiles, infiles, 
			    localfilenames, work_dir) != DC_OK)
	{
		chdir(pwd);
		free(pwd);
		return NULL;
	}
	//DC_log(LOG_DEBUG,"Input file(s) copied into '%s/input'!", work_dir);

	id = job_submit(work_dir);
	//DC_log(LOG_DEBUG,"'%s' job submitted!", id);

	if(chdir(pwd))
	{
		free(pwd);
		return NULL;
	}
	free(pwd);

	return id;

}

int resubmit(char *workdir, char *wu_name[1])
{
        char *pwd;
	char *id;

	//DC_log(LOG_DEBUG,"SUBMIT: workdir: '%s'", workdir);
	
        pwd = get_path();

	//DC_log(LOG_DEBUG,"SUBMIT: locate path: '%s'", pwd);
	
        id = job_submit(workdir);

	//DC_log(LOG_DEBUG,"SUBMIT: wuname: '%s'", id);

	if(chdir(pwd))
	{
	    free(pwd);
	    return DC_ERROR;
	}
	free(pwd);

	wu_name[0] = strdup(id);
	return DC_OK;	
}

int create_work_dir(const char *work_dir_base, const char *work_dir_id, const char *work_dir_num)
{
	char path[128];

	if(mkdir(work_dir_base, (S_IRWXU|S_IRWXG|S_IRWXO)) != 0)
	{
	    
	    //return -1;
	}
	
	sprintf(path, "%s", work_dir_base);
	
	if(chdir(path) != 0)
	{
	    DC_log(LOG_ERR,"Cannot move into '%s' directory!", work_dir_base);
	    return DC_ERROR;
	}
	
	if(mkdir(work_dir_id, (S_IRWXU|S_IRWXG|S_IRWXO)) != 0)
	{
	    //return -3;
	}
	
	sprintf(path, "./%s", work_dir_id);
	
	if(chdir(path) != 0)
	{
	    DC_log(LOG_ERR,"Cannot move into '%s/%s' directory!", work_dir_base, work_dir_id);
	    return DC_ERROR;
	}
	
	if(mkdir(work_dir_num, (S_IRWXU|S_IRWXG|S_IRWXO)) != 0)
	{
	    DC_log(LOG_ERR,"The following directory already exists : '%s/%s/%s'!",
			    work_dir_base, work_dir_id, work_dir_num);
	    return DC_ERROR;
	}
	
	sprintf(path, "./%s", work_dir_num);
	
	if(chdir(path) != 0)
	{
	    DC_log(LOG_ERR,"Cannot move into '%s/%s/%s' directory!",
			    work_dir_base, work_dir_id, work_dir_num);
	    
	    return DC_ERROR;
	}

	return DC_OK;
}

int create_jobdir_structure()
{
	if (mkdir("bin", (S_IRWXU|S_IRWXG|S_IRWXO) ) != 0)
	{		
		DC_log(LOG_ERR,"Cannot create 'bin' directory!");
		return DC_ERROR;
	}
	
	if (mkdir("input", (S_IRWXU|S_IRWXG|S_IRWXO)) != 0)
	{
		DC_log(LOG_ERR,"Cannot create 'input' directory!");
		return DC_ERROR;
	}

	if (mkdir("output", (S_IRWXU|S_IRWXG|S_IRWXO)) != 0)
	{
		DC_log(LOG_ERR,"Cannot create 'output' directory!");
		return DC_ERROR;
	}

	if (mkdir("tmp", (S_IRWXU|S_IRWXG|S_IRWXO)) != 0)
	{
		DC_log(LOG_ERR,"Cannot create 'tmp' directory!");
		return DC_ERROR;
	}

	return DC_OK;		
}

int create_submit_file(const char *job_name, const char *executable, 
    const char *args)
{
	FILE *f;
	f = fopen("submit","w");
	if(f != NULL)
	{
		/* jobname */
		fprintf(f, "[%s]\n", job_name);

		/* executable */
		fprintf(f, "executable = %s\n", executable);

		/* arguments */
		fprintf(f, "arguments = %s\n", args);

		/* arg type */
		fprintf(f, "type = seq\n");

		/* These (3) lines make the job run on sztaki.testlab */
		//fprintf(f, "\n");
		//fprintf(f, "resource_id = https://n0.sztaki-testlab.grid/\n");
		fprintf(f, "mercury = true\n");
		
		fclose(f);
		return DC_OK;
	}else{
		return DC_ERROR;
	}
}

int copy_executables(const char *executable_dir, const char *work_dir)
{
	char command[2024];
	
	//chdir("../../..");
	snprintf(command, 2024, "cp -r %s/* %s/bin/", executable_dir, work_dir);
	if (system(command) == -1) return DC_ERROR;
	
	return DC_OK;
}

int copy_inputfiles(int num_of_infiles, const char *infiles[MAX_INFILES], 
		     const char *localfilenames[MAX_INFILES], const char *work_dir)
{
	int i;
	char command[2024];
	
	for(i=0; i < num_of_infiles; i++)
	{
		snprintf(command, 2024, "cp %s %s/input/%s", infiles[i], work_dir, localfilenames[i]);
		if (system(command) == -1) return DC_ERROR;
	}
	
	return DC_OK;
}

char *job_submit(const char *work_dir)
{
	FILE *clgr_submit;
	char *command = "clgr_submit .";
	char result[100];
	char *id;
	
	if(chdir(work_dir) != 0)
	{
	    DC_log(LOG_ERR,"job_submit: cannot move into '%s' directory!", work_dir);
	    return NULL;
	}

	clgr_submit = popen(command, "r");
	if(clgr_submit == NULL)
	{
	    id = NULL;
	}else
	{
	    fgets(result, sizeof(result), clgr_submit);
	    pclose(clgr_submit);
	    	
    	    /* retreive identifier string */
	    id = retreive_id(result);
	}
	//DC_log(LOG_DEBUG,"%s job submitted!", id);
	
	return id;
}

char *retreive_id(const char *result)
{
	char *ok;
	int len;
	char *res;
	char *id;
	ok = strstr(result, "ok");
	if(ok == NULL)
	{
	    id = NULL;
	}else{
		res = (char *)strchr(result, ' ');
		res++;
		len = strcspn(res, " ");
		id = (char *) malloc(len*sizeof(char));
		if(id != NULL)
		    snprintf(id, len+1, "%s", res);
	}
	return id;	
}

