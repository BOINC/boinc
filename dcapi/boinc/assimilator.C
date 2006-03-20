#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <ctime>
#include <vector>
#include <strings.h>

#include "dc_boinc.h"

#include <boinc_db.h>
#include <parse.h>
#include <util.h>
#include <sched_util.h>
#include <sched_msgs.h>

int DC_processEvents(int timeout)
{
	DC_Result *result;
	char *wu_query;
	DB_WORKUNIT wu;
	int ret = 0;

	/* XXX Check LIMIT value */
	wu_query = g_strdup_printf("WHERE name LIKE '%s_%%' "
		"AND assimilate_state = %d LIMIT 100", project_uuid_str,
		ASSIMILATE_READY);
	while (!wu.enumerate(wu_query))
	{
		DB_RESULT canonical_result;
		char *result_query;

		/* We are only interested in the canonical result */
		if (!wu.canonical_resultid)
		{
			DC_log(LOG_DEBUG, "No canonical result for work unit %d",
				wu.id);
			continue;
		}

		result_query = g_strdup_printf("WHERE id = %d",
			wu.canonical_resultid);
		canonical_result.lookup(result_query);
		g_free(result_query);

		/* Call the callback function */
		result = _DC_createResult(wu.name, canonical_result.xml_doc_in);
		if (!result)
			continue;
		_dc_resultcb(result->wu, result);
		_DC_destroyResult(result);

		/* Mark the work unit as completed */
		wu.assimilate_state = ASSIMILATE_DONE;
		wu.transition_time = time(0);
		wu.update();
	}

	g_free(wu_query);
	return ret;
}
