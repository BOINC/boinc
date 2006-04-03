<?php

// script to delete results with no corresponding workunit.
// In theory these shouldn't exist,
// but (because of crashes or bugs) they sometimes do.
// db_purge doesn't get rid of them; this does

require_once("../inc/util.inc");
require_once("../inc/db.inc");

set_time_limit(0);
$config = get_config();
$db_name = parse_config($config, "<db_name>");
system("mysql $db_name -e \"select id from workunit\" | tail +2 | sort -n > dbc_wu.dat");
system("mysql $db_name -e \"select workunitid, id from result \" | tail +2 | sort -n > dbc_res.dat");
system("join -v 1 dbc_res.dat dbc_wu.dat > dbc_diff.dat");

?>
