/**************************************************
 * Uppercase Master program
 * 
 * Created by: Gabor Vida
 * Fine tuned by: Adam Kornafeld
 *
 * The aim of the program is to process text files 
 * (make all of their characters uppercase) in a Public Resource Computing
 * environment. The program follows the client server model, therefore
 * having a server-side and a client-side part.
 *
 * This is the server-side part of the program. It creates WorkUnits (WUs) 
 * for a Boinc project using the DC Application Programming Interface.
 * The program takes a text file (master_input) divides it up to x (MAX_N_OF_WUS)
 * parts, creates x WUs of them and submits them into the PRC-infrastructure,
 * where they get processed by the clients. After the clients have returned
 * the results of the WUs, the program assimilates them and constructs a 
 * final output file, by concatenating the output of the results.
 *
 *****************************************************/

#include <dc.h>

#include <stdio.h>
#include <stdlib.h>

#define	MAX_N_OF_WUS	20

static DC_Workunit wus[MAX_N_OF_WUS];
static int max_wus;
static int assimilatedWUs = 0;

/* Create the master_output.txt file, from the minor output files */
static void CalculateFinalResult(void){

    /* We have to open the minor_output files (results) for reading
     * and write their content in the major_output file.
     */

    int i;
    FILE *major_output;
    FILE *minor_output;
    char minor_filename[256];
    char buf[512];

    major_output = fopen("master_output.txt", "w");
    if (major_output == NULL){
	fprintf(stderr, "Error: Cannot open master_output.txt for writing.\n");
	fflush(stderr);
	exit(1);
    }

    for(i = 0; i < max_wus; i++){
	snprintf(minor_filename, sizeof(minor_filename), "master_output_%d.txt", i);
	minor_output = fopen(minor_filename, "r");
	if (minor_output == NULL){
	    fprintf(stderr, "Error: Cannot open %s for reading.\n", minor_filename);
	    fflush(stderr);
	    exit(1);
	}
	while(fgets(buf, sizeof(buf), minor_output)){
	    fprintf(major_output, buf);
	}
	fclose(minor_output);
    }    

    fclose(major_output);    
}

/* Copies the output files into the master working directory */
static void cb_assimilate_result(DC_Result result)
{
    char syscmd[256];
    int i;

    fprintf(stdout, "Num of output files: %d\n", result->noutfiles);
    fprintf(stdout, "result_dir: %s\n", result->outfiles_dir);
    fprintf(stdout, "outputfile: %s\n", result->outfiles[0]);
    fflush(stdout);
    if (result->noutfiles != 1){
	fprintf(stderr, "Master: Error: the number of output files :%d\n", result->noutfiles);
	fflush(stderr);
	exit(1);
    }

    for (i = 0; i < max_wus; i++){
	if (result->wu == wus[i]){
	    snprintf(syscmd, 256, "cp %s/%s master_output_%d.txt", result->outfiles_dir, result->outfiles[0], i);
	    system(syscmd);
	    assimilatedWUs++;
	}
    }

    DC_destroyWU(result->wu);
}

/* Waiting for the results */
static void WaitForResults(void)
{
    while(assimilatedWUs < max_wus){
	DC_checkForResult(0, cb_assimilate_result);
    }
}

/* split the major input file into MAX_N_OF_WUS minor input files */
static void CreateInputFiles(void)
{
    int i, j;
    int major_lines_num = 0;
    div_t minor_lines_num;
    char buf[512];
    char minor_filename[256];
    FILE *major_input;	
    FILE *minor_input;

    /* Count how many lines the major input file has, and how many the minor files will get */
    major_input = fopen("master_input.txt", "r");
    if (major_input == NULL){
	fprintf(stderr, "Cannot open master_input.txt file.\n");
	fflush(stderr);
	exit(1);
    }
    while(!feof(major_input)){
        fgets(buf, sizeof(buf), major_input);
        major_lines_num++;
    }
    fseek(major_input, 0, SEEK_SET);

    if (MAX_N_OF_WUS > major_lines_num) max_wus = major_lines_num;
    else max_wus = MAX_N_OF_WUS;
    
    minor_lines_num = div(major_lines_num, max_wus);


    /* Create the minor input files */
    for (i = 0; i < max_wus; i++){
	snprintf(minor_filename, sizeof(minor_filename), "master_input_%d.txt", i);
	minor_input = fopen(minor_filename, "w");
	if (minor_input == NULL){
	    fprintf(stderr, "Cannot open %s file.\n", minor_filename);
	    exit(1);
	}
	for(j = 0; j < minor_lines_num.quot; j++){
	    snprintf(buf, sizeof(buf), "");
	    fgets(buf, sizeof(buf), major_input);
	    fprintf(minor_input, buf);
	}
	if (i < minor_lines_num.rem){
	    fgets(buf, sizeof(buf), major_input);
	    fprintf(minor_input, buf);
	}
	fclose(minor_input);
    }

    fclose(major_input);
}

static void CreateWork(void)
{
    int i;
    char minor_filename[256];

    CreateInputFiles();
    for (i = 0; i < max_wus; i++){
	wus[i] = DC_createWU("uppercase", "");
	if (wus[i] == -1)
	{
	    fprintf(stderr, "Cannot create more workunit!\n");
	    fflush(stderr);
	    exit(1);
	}
	snprintf(minor_filename, sizeof(minor_filename), "master_input_%d.txt", i);
	DC_setInput(wus[i], minor_filename, "in.txt");
	DC_submitWU(wus[i]);
    }

}

int main(int argc, char *argv[])
{
    /* Initializing DC-API */
    if (DC_init(NULL, "uppercase", "uppercase-master.conf") != DC_OK) {
        fprintf(stderr, "Master: DC_init failed, exiting.\n");
	exit(1);
    }

    /* Creating Workunits : splitting up the input file */
    fprintf(stdout, "Master: Creating %d WorkUnit.\n", MAX_N_OF_WUS);
    fflush(stdout);
    CreateWork();
    fprintf(stdout, "Master: %d WorkUnit created successfully. Waiting for Results...\n", MAX_N_OF_WUS);
    fflush(stdout);
    
    /* Waiting for the returning results */
    WaitForResults();

    /* Got all the results -> building up the final output file */
    CalculateFinalResult();

    return 0;
}
