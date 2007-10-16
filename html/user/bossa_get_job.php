<?php

require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../bossa_inc/bossa_db.inc");

db_init();

$user = get_logged_in_user();

$bossa_app_id = get_int('bossa_app_id');
$app = Bossa::app_lookup_id($bossa_app_id);

if (!$app) {
    error_page("no such app: $bossa_app_id");
}

$ji = Bossa::assign_job($app, $user);
if ($ji) {
    $url = $app->start_url."&job_info=".$job->job_info;
    Header("Location: $url");
} else {
    page_head("No jobs available");
    echo "
        Sorry, no jobs are available right not.
        Please try again later.
    ";
    page_tail();
}
?>
