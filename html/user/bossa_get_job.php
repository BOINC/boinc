<?php

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");
require_once("../inc/bossa_impl.inc");

$user = get_logged_in_user();
BossaUser::lookup($user);

$bossa_app_id = get_int('bossa_app_id');
$app = BossaApp::lookup_id($bossa_app_id);

if (!$app) {
    error_page("no such app: $bossa_app_id");
}

{
    $trans = new BossaTransaction();
    show_next_job($app, $user);
}

?>
