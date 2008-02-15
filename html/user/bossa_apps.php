<?php

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");
require_once("../inc/bolt_db.inc");

function show_app($app) {
    global $user;
    if ($app->bolt_course_id) {
        if ($user) {
            switch (bolt_course_status($app->bolt_course_id, $user->id)) {
            case BOLT_COURSE_NOT_STARTED:
                $x = "<a href=bolt_sched.php?action=start&course_id=$app->bolt_course_id>Take training course</a>";
                break;
            case BOLT_COURSE_STARTED:
                $x = "<a href=bolt_sched.php?action=resume&course_id=$app->bolt_course_id>Finish training course</a>";
                break;
            case BOLT_COURSE_FINISHED:
                $x = "<a href=bossa_get_job.php?bossa_app_id=$app->id>Get job</a>";
                break;
            }
        } else {
            $x = "<a href=bolt_sched.php?action=start&course_id=$app->bolt_course_id>Take training course</a>";
        }
    } else {
        $x = "<a href=bossa_get_job.php?bossa_app_id=$app->id>Get job</a>";
    }
    row2("$app->name<span class=note><br>$app->description</span>", $x);
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

$user = get_logged_in_user();

main();

?>
