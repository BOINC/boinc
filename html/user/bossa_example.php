<?php

require_once("../inc/bossa.inc");

// Bossa example.
// Show the user an image and ask them whether it's a zero or one.

function show_job($bj, $bji) {
    if ($bji->finish_time) {
        error_page("You already finished this job");
    }
    $info = json_decode($bj->info);
    $img_url = "http://boinc.berkeley.edu/images/number_".$info->number.".jpg";
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
    $response = null;
    $response->number = get_int('response');
    $bji->info = json_encode($response);
    $bji->completed($bj);

    // show another job immediately
    //
    Bossa::show_next_job($bj);
}

Bossa::script_init($user, $bj, $bji);

if (isset($_GET['submit'])) {
    handle_job_completion($bj, $bji);
} else {
    show_job($bj, $bji);
}

?>
