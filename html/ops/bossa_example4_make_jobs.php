<?php

// Make some jobs for Bossa example 4
// Usage:
// bossa_example4_make_jobs.php
//   --dir dir

require_once("../inc/bossa.inc");

function make_job($path, $batchid, $appid) {
    $info = null;
    $info->path = $path;

    if (!bossa_job_create($appid, $batchid, $info, false)) {
        exit("bossa_create_job() failed\n");
    }
    echo "created job for $path\n";
}

function make_jobs($dir, $appid) {
    $batchid = bossa_batch_create($appid, date(DATE_RFC822), false);
    if (!$batchid) {
        exit("bossa_create_batch() failed\n");
    }

    $d = opendir("../user/$dir");
    while ($file = readdir($d)) {
        if (!strstr($file, ".png") && !strstr($file, ".jpg")) continue;
        make_job("$dir/$file", $batchid, $appid);
    }
    closedir($d);
}

function usage() {
    exit("Usage: bossa_example4_make_jobs.php --dir d\n");
}

for ($i=1; $i<$argc; $i++) {
    if ($argv[$i] == '--dir') $dir = $argv[++$i];
    else usage();
}

if (!$dir) usage();

if (!is_dir("../user/$dir")) {
    exit("../user/$dir is not a directory\n");
}

$appid = bossa_app_lookup("bossa_example4");
if (!$appid) exit("No application 'bossa_example4'\n");

make_jobs($dir, $appid);
