#include <stdio.h>

#include "dc.h"

int main( int argc, char *argv[])
{
    if (DC_init("MyExampleProject", "MyExampleApp", "local.conf") != DC_OK) {
	printf("DC_init returned with error. Exit application\n");
	return 1;
    }
    printf("DC_init returned OK.\n");
    return 0;
}

