<?php

require_once('error.inc');

if(!@mysql_pconnect($cfg['db']['host'], $cfg['db']['user'], $cfg['db']['password']) || !@mysql_select_db($cfg['db']['database']))
	doError('The server is currently unavailable.');

function sql_query($sql) {
	if(!$result = mysql_query($sql))
		doError('A database error has occurred.<br>MySQL #'.mysql_errno().': '.mysql_error()."\n\nSQL: $sql");
	return $result;
}

function sql_fetch_array($result) {
	return mysql_fetch_array($result);
}

function sql_num_rows($result) {
        return mysql_num_rows($result);
}

function sql_insert_id() {
	return mysql_insert_id();
}

?>