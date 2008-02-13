<?php

require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/bossa_db.inc");

db_init();

$user = get_logged_in_user();

$bossa_app_id = get_int('bossa_app_id');
$app = BossaApp::lookup_id($bossa_app_id);

if (!$app) {
    error_page("no such app: $bossa_app_id");
}

$ji = BossaJobInst::assign($app, $user);
if ($ji) {
    $url = $app->short_name.".php?bji=$ji->id";
    Header("Location: $url");
} else {
    page_head("No jobs available");
    echo "
        Sorry, no more jobs are available right now.
        <p>
        Please try again later.
    ";
    page_tail();
}
?>
