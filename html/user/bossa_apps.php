<?php

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");

function show_app($app) {
    row2(
        "$app->name<span class=note><br>$app->description</span>",
        "<a href=bossa_get_job.php?bossa_app_id=$app->id>Get job</a>"
    );
}

function show_apps() {
    $apps = BossaApp::enum();
    foreach ($apps as $app) {
        if ($app->hidden) continue;
        show_app($app);
    }
}

function main() {
    page_head("Bossa apps");
    start_table();
    show_apps();
    end_table();
    page_tail();
}

main();

?>
