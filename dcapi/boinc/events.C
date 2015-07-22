/*
 * events.C - BOINC event handling
 *
 * Authors:
 *	Gábor Gombás <gombasg@sztaki.hu>
 *
 * Copyright (c) 2006 MTA SZTAKI
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <strings.h>

#include "dc_boinc.h"

#include <boinc_db.h>
#include <sched_util.h>
#include <sched_msgs.h>
#include <parse.h>

/* Be nice to the database and sleep for a long time */
#define SLEEP_UNIT		15

static void mark_wu_complete(DB_WORKUNIT &dbwu, int do_callback)
{
	DC_Workunit *wu;

	/* Invoke the callback with an empty result so the application knows
	 * that the WU has failed */
	wu = _DC_getWUByName(dbwu.name);
	if (wu && _dc_resultcb && do_callback)
		_dc_resultcb(wu, NULL);

	dbwu.assimilate_state = ASSIMILATE_DONE;
	dbwu.transition_time = time(NULL);
	dbwu.update();
}

static int wu_has_failed(DB_WORKUNIT &wu)
{
	if (wu.error_mask & WU_ERROR_CANCELLED)
	{
		DC_log(LOG_INFO, "Acknowledging cancelled WU %s", wu.name);
		return TRUE;
	}

	if (wu.error_mask & WU_ERROR_TOO_MANY_ERROR_RESULTS)
	{
		DC_log(LOG_WARNING, "There are too many errors for WU %s",
			wu.name);
		return TRUE;
	}

	if (wu.error_mask & WU_ERROR_TOO_MANY_TOTAL_RESULTS)
	{
		DC_log(LOG_WARNING, "There are too many results for WU %s",
			wu.name);
		return TRUE;
	}

	if (wu.error_mask & WU_ERROR_COULDNT_SEND_RESULT)
	{
		if (wu.canonical_resultid)
			return FALSE;

		/* XXX We could count the results that still have a chance to
		 * complete and error out only if there are fewer than
		 * min_quorum, but that's complex */
		DC_log(LOG_WARNING, "One or more results for WU %s coudln't "
			"be sent", wu.name);
		return TRUE;
	}

	if (!wu.canonical_resultid)
	{
		DC_log(LOG_WARNING, "No canonical result for WU %s", wu.name);
		return TRUE;
	}

	return FALSE;
}

static int process_results(void)
{
	DC_Result *result;
	DB_WORKUNIT wu;
	char *query;
	int done;

	/* XXX Check LIMIT value */
	query = g_strdup_printf("WHERE name LIKE '%s\\_%%' "
		"AND assimilate_state = %d LIMIT 100", project_uuid_str,
		ASSIMILATE_READY);
	done = 0;
	while (!wu.enumerate(query))
	{
		DB_RESULT canonical_result;

		if (wu_has_failed(wu))
		{
			mark_wu_complete(wu, TRUE);
			continue;
		}

		if (canonical_result.lookup_id(wu.canonical_resultid))
		{
			DC_log(LOG_ERR, "Result #%d is not in the database",
				wu.canonical_resultid);
			mark_wu_complete(wu, TRUE);
			continue;
		}

		/* Call the callback function */
		result = _DC_createResult(wu.name, wu.id,
			canonical_result.xml_doc_in, canonical_result.cpu_time);
		if (!result)
		{
			mark_wu_complete(wu, TRUE);
			continue;
		}
		_dc_resultcb(result->wu, result);
		_DC_destroyResult(result);
		done++;
	}

	g_free(query);
	return done;
}

static void handle_special_msg(DC_Workunit *wu, const char *msg)
{
	if (!strncmp(msg, DC_MSG_UPLOAD, strlen(DC_MSG_UPLOAD)) &&
			_dc_subresultcb)
	{
		const char *p = strchr(msg, ':');
		if (!p)
			return;
		const char *q = strchr(p + 1, ':');
		if (!q)
			return;

		char *subresult_name = g_strndup(p + 1, (q - p) - 1);
		char *client_label = g_strdup(q + 1);

		/* Paranoia; the XML signature should already prevent the
		 * client from tampering with the destination path */
		if (strchr(subresult_name, G_DIR_SEPARATOR))
		{
			DC_log(LOG_ERR, "Client sent insecure subresult name, "
				"ignoring");
			g_free(subresult_name);
			g_free(client_label);
			return;
		}

		char *path = _DC_hierPath(subresult_name, TRUE);
		_dc_subresultcb(wu, client_label, path);

		g_free(subresult_name);
		g_free(client_label);
		g_free(path);
	}
	else
		DC_log(LOG_WARNING, "Received unknown control message %s", msg);
}

static int process_messages(void)
{
	DB_MSG_FROM_HOST msg;
	char *query, *buf;
	DC_Workunit *wu;
	int done, ret;

	query = g_strdup_printf("WHERE variety LIKE '%s\\_%%' AND handled = 0",
		project_uuid_str);
	buf = (char *)g_malloc(MAX_MESSAGE_SIZE);
	done = 0;
	while (!msg.enumerate(query))
	{
		wu = _DC_getWUByName(msg.variety);
		if (!wu)
		{
			DC_log(LOG_WARNING, "Received message for unknown "
				"WU %s", msg.variety);
			goto handled;
		}

		ret = parse_str(msg.xml, "<message>", buf, MAX_MESSAGE_SIZE);
		if (!ret)
		{
			DC_log(LOG_WARNING, "Failed to parse message %s",
				msg.xml);
			goto handled;
		}

		if (!strncmp(buf, DCAPI_MSG_PFX, strlen(DCAPI_MSG_PFX)))
			handle_special_msg(wu, buf + strlen(DCAPI_MSG_PFX) + 1);
		else if (_dc_messagecb)
			_dc_messagecb(wu, buf);
		done++;
handled:
		msg.handled = 1;
		msg.update();
	}
	g_free(query);
	g_free(buf);
	return done;
}

int DC_processMasterEvents(int timeout)
{
	time_t end, now;
	static int once;
	int done;

	if (!_dc_resultcb && !_dc_subresultcb && !_dc_messagecb)
	{
		DC_log(LOG_ERR, "%s: callbacks are not initialized", __func__);
		return DC_ERR_CONFIG;
	}

	/* Warn the user whether she knows what she is doing */
	if (!once)
	{
		if (!_dc_resultcb)
			DC_log(LOG_NOTICE, "The result callback is not set, "
				"completed WUs will not be reported");
		if (!_dc_subresultcb)
			DC_log(LOG_NOTICE, "The subresult callback is not "
				"set, subresults will not be reported");
		if (!_dc_messagecb)
			DC_log(LOG_NOTICE, "The message callback is not set, "
				"messages will not be reported");
		once = 1;
	}

	end = time(NULL) + timeout;
	done = 0;
	while (1)
	{
		if (_dc_resultcb)
			done += process_results();
		if (_dc_subresultcb || _dc_messagecb)
			done += process_messages();
		if (done)
			break;

		now = time(NULL);
		if (now >= end)
			break;
		sleep(MIN(end - now, SLEEP_UNIT));
	}

	return done ? 0 : DC_ERR_TIMEOUT;
}

/* Look for a single result that matches the filter */
static DC_MasterEvent *look_for_results(const char *wuFilter, const char *wuName)
{
	DC_MasterEvent *event;
	DB_RESULT result;
	DB_WORKUNIT wu;
	char *query;

	if (wuFilter)
		query = g_strdup_printf("WHERE name LIKE '%s\\_%%\\_%s' "
			"AND assimilate_state = %d LIMIT 1", project_uuid_str,
			wuFilter, ASSIMILATE_READY);
	else if (wuName)
		query = g_strdup_printf("WHERE name LIKE '%s\\_%s%%' "
			"AND assimilate_state = %d LIMIT 1", project_uuid_str,
			wuName, ASSIMILATE_READY);
	else
		query = g_strdup_printf("WHERE name LIKE '%s\\_%%' "
			"AND assimilate_state = %d LIMIT 1", project_uuid_str,
			ASSIMILATE_READY);

	if (wu.enumerate(query))
	{
		g_free(query);
		return NULL;
	}
	g_free(query);

	if (wu_has_failed(wu))
	{
		mark_wu_complete(wu, FALSE);
		return NULL;
	}

	if (result.lookup_id(wu.canonical_resultid))
	{
		DC_log(LOG_ERR, "Result #%d is not in the database",
			wu.canonical_resultid);
		mark_wu_complete(wu, FALSE);
		return NULL;
	}

	event = g_new(DC_MasterEvent, 1);
	event->type = DC_MASTER_RESULT;
	event->result = _DC_createResult(wu.name, wu.id, result.xml_doc_in,
		result.cpu_time);
	if (!event->result)
	{
		mark_wu_complete(wu, FALSE);
		g_free(event);
		return NULL;
	}
	event->wu = event->result->wu;

	return event;
}

static DC_MasterEvent *look_for_messages(const char *wuFilter, const char *wuName)
{
	DC_MasterEvent *event;
	DB_MSG_FROM_HOST msg;
	char *query, *buf;
	DC_Workunit *wu;
	int ret;

restart:
	if (wuFilter)
		query = g_strdup_printf("WHERE variety LIKE '%s\\_%%\\_%s' "
			"AND handled = 0", project_uuid_str, wuFilter);
	else if (wuName)
		query = g_strdup_printf("WHERE variety LIKE '%s\\_%s%%' "
			"AND handled = 0", project_uuid_str, wuName);
	else
		query = g_strdup_printf("WHERE variety LIKE '%s\\_%%' "
			"AND handled = 0", project_uuid_str);

	if (msg.enumerate(query))
	{
		g_free(query);
		return NULL;
	}
	g_free(query);

	wu = _DC_getWUByName(msg.variety);
	if (!wu)
	{
		DC_log(LOG_WARNING, "Received message for unknown WU %s",
			msg.variety);
		msg.handled = 1;
		msg.update();
		return NULL;
	}

	buf = (char *)g_malloc(MAX_MESSAGE_SIZE);
	ret = parse_str(msg.xml, "<message>", buf, MAX_MESSAGE_SIZE);
	if (!ret)
	{
		DC_log(LOG_WARNING, "Failed to parse message %s", msg.xml);
		g_free(buf);
		msg.handled = 1;
		msg.update();
		return NULL;
	}

	if (!strncmp(buf, DCAPI_MSG_PFX, strlen(DCAPI_MSG_PFX)))
	{
		handle_special_msg(wu, buf + strlen(DCAPI_MSG_PFX) + 1);
		g_free(buf);
		msg.handled = 1;
		msg.update();
		goto restart;
	}

	event = g_new(DC_MasterEvent, 1);
	event->wu = wu;
	event->type = DC_MASTER_MESSAGE;
	event->message = buf;
	return event;
}

DC_MasterEvent *DC_waitMasterEvent(const char *wuFilter, int timeout)
{
	time_t end, now;
	DC_MasterEvent *event;

	end = time(NULL) + timeout;
	while (1)
	{
		event = look_for_results(wuFilter, NULL);
		if (event)
			break;
		event = look_for_messages(wuFilter, NULL);
		if (event)
			break;
		now = time(NULL);
		if (now >= end)
			break;
		sleep(MIN(end - now, SLEEP_UNIT));
	}
	return event;
}

DC_MasterEvent *DC_waitWUEvent(DC_Workunit *wu, int timeout)
{
	char uuid_str[36];
	DC_MasterEvent *event;
	time_t end, now;

	if (!wu)
	{
		DC_log(LOG_ERR, "%s: Missing WU", __func__);
		return NULL;
	}

	uuid_unparse_lower(wu->uuid, uuid_str);

	end = time(NULL) + timeout;
	while (1)
	{
		event = look_for_results(NULL, uuid_str);
		if (event)
			break;
		event = look_for_messages(NULL, uuid_str);
		if (event)
			break;
		now = time(NULL);
		if (now >= end)
			break;
		sleep(MIN(end - now, SLEEP_UNIT));
	}
	return event;
}

void DC_destroyMasterEvent(DC_MasterEvent *event)
{
	if (!event)
		return;

	switch (event->type)
	{
		case DC_MASTER_RESULT:
			_DC_destroyResult(event->result);
			break;
		case DC_MASTER_SUBRESULT:
			_DC_destroyPhysicalFile(event->subresult);
			break;
		case DC_MASTER_MESSAGE:
			g_free(event->message);
			break;
		default:
			DC_log(LOG_ERR, "%s: Unknown event type %d",
				__func__, event->type);
			break;
	}
}
