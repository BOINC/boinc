#include <stdio.h>
#include <string.h>

#include "dc.h"

/* Function prototypes */
static DC_Workunit * createWU(void);

static void callbackResult(DC_Workunit *wu, DC_Result *result)
{
	printf("Callback function for result destroying WU.\n");

	DC_destroyWU(wu);
	return;
}

static void callbackSubresult(DC_Workunit *wu, const char*, const char*)
{
	// not impl yet

	return;
}

static void callbackMessage(DC_Workunit *wu, const char*)
{
	// not impl yet

	return;
}


int main( int argc, char *argv[])
{
	DC_Workunit *wu;
	int retval;
	//char ch;

	retval = DC_init("local.conf");
	if (retval != DC_OK) {
		printf("DC_init returned with error. Exit application\n");
		return 1;
	}
	printf("DC_init returned OK.\n");

	DC_setcb(callbackResult, callbackSubresult, callbackMessage);

	wu = createWU();

	DC_submitWU(wu);

	retval = DC_processEvents(10);

	if (retval)
	{
		printf("Error:  Not processed the WU in 10 sec.  retval = %d\n", retval);
	}

	printf("Exit application\n");

	return 0;
}

static DC_Workunit * createWU(void)
{
	DC_Workunit *wu;
	char *clientname;
	const char *arguments[2];
	int subresults;
	char *tag;

	clientname = strdup("execdir/uppercase-client");
	tag = strdup("testing");
	subresults = 5;
	arguments[0] = strdup("-argument1");
	arguments[1] = strdup("-argument2");
	wu = DC_createWU(clientname, arguments, subresults, tag);

	DC_addWUInput(wu, "in.txt", "master.c", DC_FILE_PERSISTENT);
	DC_addWUOutput(wu, "out.txt");

	return (wu);
}
