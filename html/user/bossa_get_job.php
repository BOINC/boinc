<?php

require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/bossa_db.inc");

$user = get_logged_in_user();
BossaUser::lookup($user);

$bossa_app_id = get_int('bossa_app_id');
$app = BossaApp::lookup_id($bossa_app_id);

if (!$app) {
    error_page("no such app: $bossa_app_id");
}

// TODO: call app-specific function to get confidence

$user->conf = 1;
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
