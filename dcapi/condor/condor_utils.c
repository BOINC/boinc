/* Local variables: */
/* c-file-style: "linux" */
/* End: */

#include <sys/stat.h>

#include "condor_utils.h"


int
_DC_mkdir_with_parents(char *dn, mode_t mode)
{
	char *p= dn;
	int res;

	if (!dn ||
	    !*dn)
		return(0);
	while (*p)
	{
		while (*p != '/' &&
		       *p != '\0')
			p++;
		if (*p == '/')
		{
			*p= '\0';
			if (*dn)
				res= mkdir(dn, mode);
			*p= '/';
			p++;
		}
	}
	res= mkdir(dn, mode);
	return(res);
}


/* End of condor_utils.c */
