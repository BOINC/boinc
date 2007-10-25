<?php

require_once("../inc/bossa_db.inc");

// Set up Bossa applications.
// Customize and rename this file.

$ba = new BossaApp();
$ba->name = 'bossa_test';
$ba->user_friendly_name = 'Simple pattern recognition';
$ba->start_url = 'bossa_example.php';

if ($ba->insert($ba)) {
    echo "Added application '$ba->name'\n";
} else {
    echo "Couldn't add '$ba->name': ", mysql_error(), "\n";
}

?>
