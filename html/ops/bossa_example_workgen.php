<?php

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");
require_once("../inc/dir_hier.inc");

function rand_color($im, $range) {
    $mid = 200;
    $lo = $mid - $range;
    $hi = $mid + $range;
    return imagecolorallocate($im, rand($lo,$hi), rand($lo,$hi), rand($lo,$hi));
}

function add_ellipse($im, $case) {
    imagefilledellipse(
        $im, $case->cx, $case->cy, $case->w, $case->h, rand_color($im, 50)
    );
}

function add_rect($im) {
    $cx = rand(-100, 600);
    $cy = rand(-100, 400);
    $w = rand(50, 100);
    $h = rand(50, 100);
    imagefilledrectangle($im, $cx, $cy, $cx+$w, $cy+$h, rand_color($im, 50));
}


function make_image($case) {
    $im = imagecreatetruecolor(600, 400);
    imagefill($im, 0, 0, imagecolorallocate($im, 255, 255, 255));
    for ($i=0; $i<400; $i++) {
        add_rect($im);
    }
    $im2 = imagecreatetruecolor(600, 400);
    imagefill($im2, 0, 0, rand_color($im2, 0));
    if ($case->have_ellipse) {
        add_ellipse($im2, $case);
    }
    imagecopymerge($im, $im2, 0, 0, 0, 0, 600, 400, 30);
    return $im;
}

function make_test_case() {
    $case = null;
    $case->have_ellipse = rand(0, 1);
    if ($case->have_ellipse) {
        $case->cx = rand(50, 550);
        $case->cy = rand(50, 350);
        $case->w = rand(50, 100);
        $case->h = rand(50, 100);
    }
    return $case;
}

function make_job($app, $batch, $i, $config) {
    // create the image file;
    // store it in the download directory hierarchy
    //
    $jobname = "job_$batch_$i";
    $filename = "$jobname.png";
    $path = dir_hier_path(
        $filename, $config->download_dir, $config->uldl_dir_fanout
    );
    $url = dir_hier_url(
        $filename, $config->download_url, $config->uldl_dir_fanout
    );
    $case = make_test_case();
    imagepng(make_image($case), $path);
    $case->url = $url;

    // make a job record in the Bossa database
    //
    $job = new BossaJob;
    $job->app_id = $app->id;
    $job->batch = $batch;
    $job->time_estimate = 30;
    $job->time_limit = 600;
    $job->name = $jobname;
    $job->info = json_encode($case);
    $job->conf_needed = $app->min_conf_sum;

    if (!$job->insert()) {
        echo "BossaJob::insert failed: ", mysql_error(), "\n";
        exit(1);
    }
}

function make_jobs($njobs) {
    $c = get_config();
    $config = null;
    $config->download_dir = parse_config($c, "<download_dir>");
    $config->download_url = parse_config($c, "<download_url>");
    $config->uldl_dir_fanout = parse_config($c, "<uldl_dir_fanout>");
    $appname = "bossa_example";
    $app = BossaApp::lookup_short_name($appname);
    if (!$app) {
        echo "Application $appname not found\n";
        exit(1);
    }
    $batch = time();
    for ($i=0; $i<$njobs; $i++) {
        make_job($app, $batch, $i, $config);
    }
    echo "Created $njobs jobs
        <p>
        <a href=bossa_admin.php>Bossa Admin</a>
    ";
}

$njobs = get_int('njobs', true);
if ($njobs) {
    make_jobs($njobs);
} else {
    header ("Content-type: image/png");
    imagepng(make_image(make_test_case()));
}

?>
