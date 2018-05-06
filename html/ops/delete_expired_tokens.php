#!/usr/bin/env php
<?php
require_once("../inc/util_ops.inc");
require_once("../inc/db_ops.inc");

echo "Starting: ", time_str(time()), "\n";

$num_deleted = BoincToken::delete_expired();

echo $num_deleted, " tokens deleted\n";

echo "Finished: ", time_str(time()), "\n";
