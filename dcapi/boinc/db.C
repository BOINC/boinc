/*
 * Wrapper code for accessing the Boinc database from C
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <boinc_db.h>

#include "dc_boinc.h"

int _DC_initDB(void)
{
	int ret;

	ret = boinc_db.open(_DC_getDBName(), _DC_getDBHost(), _DC_getDBUser(),
		_DC_getDBPasswd());
	if (ret)
	{
		DC_log(LOG_ERR, "Failed to connect to the Boinc database "
			"(error #%d)", ret);
		return DC_ERR_DATABASE;
	}
	return 0;
}
