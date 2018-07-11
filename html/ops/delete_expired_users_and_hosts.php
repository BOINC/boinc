#!/usr/bin/env php
<?php
require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");

echo "Starting: ", time_str(time()), "\n";

$num_deleted = BoincUserDeleted::delete_expired();
echo $num_deleted, " users deleted from user_deleted\n";

$num_deleted = BoincHostDeleted::delete_expired();
echo $num_deleted, " hosts deleted from host_deleted\n";

echo "Finished: ", time_str(time()), "\n";
