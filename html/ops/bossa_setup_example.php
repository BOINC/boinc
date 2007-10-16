<?php

require_once("../bossa_inc/bossa_db.inc");
require_once("../inc/db.inc");

db_init();

// Set up Bossa applications.
// Customize and rename this file.

$ba->name = 'bossa_test';
$ba->user_friendly_name = 'Simple pattern recognition';
$ba->start_url = 'test_start.php';

if (Bossa::insert_app($ba)) {
    echo "success\n";
} else {
    echo "failed ", mysql_error();
}

?>
