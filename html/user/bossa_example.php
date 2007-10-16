<?php

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");

db_init();

// Bossa example.
// Show the user an image and ask them whether it's a zero or one.

function show_job($bj, $bji) {
    if ($bji->finish_time) {
        error_page("You already finished this job");
    }
    $i = $bj->info;
    $img_url = "http://boinc.berkeley.edu/images/number_$i.jpg";
    echo "
        <form method=get action=bossa_example.php>
        <input type=hidden name=bji value=$bji->id>
        <img src=$img_url>
        <br>
        The picture shows a
        <br><input type=radio name=response value=0> zero
        <br><input type=radio name=response value=1> one
        <br><input type=radio name=response value=2 checked> not sure
        <br><br><input type=submit name=submit value=OK>
        </form>
    ";
}

function handle_job_completion($bj, $bji) {
    $response = get_int('response');
    print_r($bji);
    $bji->info = "response=$response";
    $bji->completed($bj);

    // show another job immediately
    //
    $url = "bossa_get_job.php?bossa_app_id=$bj->app_id";
    Header("Location: $url");
}

$user = get_logged_in_user();
$bji = BossaJobInst::lookup_id(get_int('bji'));
if (!$bji) {
    error_page("No such job instance");
}
if ($bji->user_id != $user->id) {
    error_page("Bad user ID");
}
$bj = BossaJob::lookup_id($bji->job_id);
if (!$bj) {
    error_page("No such job");
}

if ($_GET['submit']) {
    handle_job_completion($bj, $bji);
} else {
    show_job($bj, $bji);
}

?>
