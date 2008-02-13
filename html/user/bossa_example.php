<?php

require_once("../inc/bossa.inc");

// Bossa example.
// Show the user an image and ask them to click on the ellipse

function show_job($bj, $bji) {
    $info = json_decode($bj->info);
    $img_url = $info->url;
    echo "
        <h2>Find the Ellipse!</h2>
        <form method=get action=bossa_example.php>
        Click on the center of the ellipse.
        If you don't see one, click here:
        <input type=submit name=submit value=None>
        <br><br>
        <input type=hidden name=bji value=$bji->id>
        <input type=hidden name=completion value=1>
        <input type=image name=pic src=$img_url>
        </form>
    ";
}

function handle_job_completion($bj, $bji) {
    $response = null;
    if (get_str('submit', true)) {
        $response->have_ellipse = 0;
    } else {
        $response->have_ellipse = 1;
        $response->cx = get_int('pic_x');
        $response->cy = get_int('pic_y');
    }
    $bji->info = json_encode($response);
    $bji->completed($bj);
    Bossa::show_next_job($bj);    // show another job immediately
}

Bossa::script_init($user, $bj, $bji);

if (isset($_GET['completion'])) {
    handle_job_completion($bj, $bji);
} else {
    show_job($bj, $bji);
}

?>
