#! /usr/bin/env php
<?php

require_once("../inc/util.inc");

$config = get_config();
$db_name = parse_config($config, "<db_name>");
$db_host = parse_config($config, "<db_host>");
$query = "select email_addr from user where expavg_credit>10 and send_email<>0";

system("mysql -h $db_host $db_name  -e \"$query\" > survey_tmp");
system("tail -n +2 survey_tmp > survey_tmp2");
system("sort -R survey_tmp2 > survey_sort");
?>
