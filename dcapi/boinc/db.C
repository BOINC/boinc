/*
 * Wrapper code for accessing the Boinc database from C
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sched_config.h>
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

static int lookup_db_id(DC_Workunit *wu)
{
	char *query, *name;
	DB_WORKUNIT dbwu;
	int ret;

	if (wu->db_id)
		return 0;

	name = _DC_getWUName(wu);
	query = g_strdup_printf("WHERE name = '%s'", name);
	g_free(name);
	ret = dbwu.lookup(query);
	g_free(query);

	if (ret)
		return DC_ERR_DATABASE;
	wu->db_id = dbwu.get_id();
	return 0;
}

void _DC_resultCompleted(DC_Result *result)
{
	char *query;
	int ret;

	if (!result->wu->db_id)
	{
		ret = lookup_db_id(result->wu);
		if (ret)
		{
			char *name = _DC_getWUName(result->wu);

			DC_log(LOG_ERR, "Failed to look up the ID of WU %s",
				name);
			return;
		}
	}

	/* We could use DB_WORKUNIT but that would require doing a SELECT
	 * before the UPDATE, which is unneccessary */
	query = g_strdup_printf("UPDATE workunit "
		"SET assimilate_state = %d, transition_time = %d "
		"WHERE id = %d", ASSIMILATE_DONE, (int)time(NULL),
		result->wu->db_id);
	boinc_db.do_query(query);
	g_free(query);
}
