/*
 * db.C - Wrapper code for accessing the Boinc database from C
 *
 * Authors:
 *	Gábor Gombás <gombasg@sztaki.hu>
 *
 * Copyright (c) 2006 MTA SZTAKI
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string>

#include <sched_config.h>
#include <boinc_db.h>
#include <parse.h>

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
	{
		char *name = _DC_getWUName(wu);

		DC_log(LOG_ERR, "Failed to look up the ID of WU %s", name);
		g_free(name);
		wu->state = DC_WU_UNKNOWN;
		return DC_ERR_DATABASE;
	}
	wu->db_id = dbwu.get_id();
	return 0;
}

int _DC_getDBid(DC_Workunit *wu)
{
	return lookup_db_id(wu);
}

void _DC_resultCompleted(DC_Result *result)
{
	char *query;
	int ret;

	ret = lookup_db_id(result->wu);
	if (ret)
		return;

	/* We could use DB_WORKUNIT but that would require doing a SELECT
	 * before the UPDATE, which is unneccessary */
	query = g_strdup_printf("UPDATE workunit "
		"SET assimilate_state = %d, transition_time = %d "
		"WHERE id = %d", ASSIMILATE_DONE, (int)time(NULL),
		result->wu->db_id);
	boinc_db.do_query(query);
	g_free(query);

	result->wu->state = DC_WU_FINISHED;
}

void _DC_updateWUState(DC_Workunit *wu)
{
	DB_WORKUNIT dbwu;
	int ret;

	if (!wu->submitted)
	{
		wu->state = DC_WU_READY;
		return;
	}

	if (wu->db_id)
		ret = dbwu.lookup_id(wu->db_id);
	else
	{
		char *name, *query;

		/* Open-code lookup_db_id to avoid a second database lookup */
		name = _DC_getWUName(wu);
		query = g_strdup_printf("WHERE name = '%s'", name);
		g_free(name);
		ret = dbwu.lookup(query);
		g_free(query);
		if (!ret)
			wu->db_id = dbwu.id;
	}

	if (ret)
		wu->state = DC_WU_UNKNOWN;
	else if (dbwu.error_mask)
		/* If any bits in the error mask are set, the scheduler won't
		 * send out new results */
		wu->state = DC_WU_ABORTED;
	else if (dbwu.assimilate_state == ASSIMILATE_DONE)
		wu->state = DC_WU_FINISHED;
	else
		wu->state = DC_WU_RUNNING;
}

int DC_cancelWU(DC_Workunit *wu)
{
	char *query, *name;
	int ret;

	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return DC_ERR_BADPARAM;
	}

	ret = lookup_db_id(wu);
	if (ret)
		return ret;

	name = _DC_getWUName(wu);
	query = g_strdup_printf("%s:%s:%s", DCAPI_MSG_PFX, DC_MSG_CANCEL, name);
	DC_sendWUMessage(wu, query);
	g_free(name);
	g_free(query);

	query = g_strdup_printf("UPDATE result "
		"SET server_state = %d, outcome = %d "
		"WHERE server_state = %d AND workunitid = %d",
		RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_DIDNT_NEED,
		RESULT_SERVER_STATE_UNSENT, wu->db_id);
	boinc_db.do_query(query);
	g_free(query);

	query = g_strdup_printf("UPDATE workunit "
		"SET error_mask = error_mask | %d "
		"WHERE id = %d",
		WU_ERROR_CANCELLED, wu->db_id);
	boinc_db.do_query(query);
	g_free(query);

	wu->state = DC_WU_ABORTED;
	return 0;
}

int DC_sendWUMessage(DC_Workunit *wu, const char *message)
{
	char *query, *name;
	DB_RESULT result;
	int ret;

	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return DC_ERR_BADPARAM;
	}

	ret = lookup_db_id(wu);
	if (ret)
		return ret;

	name = _DC_getWUName(wu);
	query = g_strdup_printf("WHERE workunitid = %d AND server_state = %d",
		wu->db_id, RESULT_SERVER_STATE_IN_PROGRESS);
	while (!result.enumerate(query))
	{
		DB_MSG_TO_HOST msg;
		char *xmlout;
		int buflen;

		msg.clear();
		msg.create_time = time(NULL);
		msg.hostid = result.hostid;
		msg.handled = false;

		/* BOINC tells output buffer should be 6x input size */
		buflen = 6 * strlen(message) + 1;
		xmlout = g_new(char, buflen);
		if (!xmlout)
		{
			DC_log(LOG_WARNING, "Failed to send message because "
				"ran out of memory");
			break;
		}

		xml_escape(message, xmlout, buflen);
		snprintf(msg.xml, sizeof(msg.xml), "<message>%s</message>", xmlout);
		g_free(xmlout);

		snprintf(msg.variety, sizeof(msg.variety), "%s", name);

		if (msg.insert())
			DC_log(LOG_WARNING, "Failed to send message to host %d",
				result.hostid);
	}
	g_free(name);
	g_free(query);

	return 0;
}
