#include <stdio.h>
#include <string.h>

#include "dc.h"

/* Global variables */
static DC_Workunit *wu;

/* Function prototypes */
static int createWU(void);

int main( int argc, char *argv[])
{
	int retval;
	char ch;

	retval = DC_init("local.conf");
	if (retval != DC_OK) {
		printf("DC_init returned with error. Exit application\n");
		return 1;
	}
	printf("DC_init returned OK.\n");

	createWU();

//	DC_submitWU(wu);

	ch = getchar();

	DC_destroyWU(wu);

	return 0;
}

static int createWU(void)
{
	char *clientname;
	const char *arguments[2];
	int subresults;
	char *tag;

//	clientname = strdup("/home/vida/szdg/dcapi/trunk/local/test/execdir/uppercase-client");
	clientname = strdup("execdir/uppercase-client");
	tag = strdup("testing");
	subresults = 5;
	arguments[0] = strdup("-argument1");
	arguments[1] = strdup("-argument2");
	wu = DC_createWU(clientname, arguments, subresults, tag);

//	DC_addWUInput(wu, "in.txt", "master.c", DC_FILE_PERSISTENT);
//	DC_addWUOutput(wu, "out.txt");
}
