<?php

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");

class Bossa {
    static function script_init(&$user, &$bj, &$bji) {
        db_init();

        $user = get_logged_in_user();
        $bji = BossaJobInst::lookup_id(get_int('bji'));
        if (!$bji) {
            error_page("No such job instance");
        }
        if ($bji->user_id != $user->id) {
            error_page("Bad user ID");
        }
        if ($bji->finish_time) {
            error_page("You already finished this job");
        }
        $bj = BossaJob::lookup_id($bji->job_id);
        if (!$bj) {
            error_page("No such job");
        }
    }

    static function show_next_job($bj) {
        $url = "bossa_get_job.php?bossa_app_id=$bj->app_id";
        Header("Location: $url");
    }
}

?>
