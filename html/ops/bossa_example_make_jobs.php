<?php

// Given a directory of images, make a batch of jobs
// Usage:
// bossa_example_make_jobs.php options
// --app_name name
// --dir dir
// [ --calibration ]

require_once("../inc/bossa.inc");

function make_job($path, $batchid, $appid, $calibration) {
    $info = null;
    $info->path = $path;

    // if it's a calibration job, get the answer and store in job record
    //
    if ($calibration) {
        $path2 = str_replace(".png", ".ans", "../user/$path");
        $info->answer = unserialize(file_get_contents($path2));
    }

    if (!bossa_job_create($appid, $batchid, $info, $calibration)) {
        exit("bossa_create_job() failed\n");
    }
    echo "created job for $path\n";
}

function make_jobs($dir, $appid, $calibration) {
    $batchid = bossa_batch_create($appid, date(DATE_RFC822), $calibration);
    if (!$batchid) {
        exit("bossa_create_batch() failed\n");
    }

    $d = opendir("../user/$dir");
    while ($file = readdir($d)) {
        if (!strstr($file, ".png")) continue;
        make_job("$dir/$file", $batchid, $appid, $calibration);
    }
    closedir($d);
}

function usage() {
    exit("Usage: php bossa_example_make_jobs.php --app_name x --dir d [--calibration]]\n");
}

$calibration = false;
for ($i=1; $i<$argc; $i++) {
    if ($argv[$i] == '--app_name') $app_name = $argv[++$i];
    elseif ($argv[$i] == '--dir') $dir = $argv[++$i];
    elseif ($argv[$i] == '--calibration') $calibration = true;
    else usage();
}

if (!$app_name || !$dir) usage();
$appid = bossa_app_lookup($app_name);
if (!$appid) {
    exit("Application '$app_name' not found\n");
}

if (!is_dir("../user/$dir")) {
    exit("../user/$dir is not a directory\n");
}

make_jobs($dir, $appid, $calibration);

?>
