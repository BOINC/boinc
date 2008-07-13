<?php

// This script generates some random image files
// Usage:
//
// bossa_example_make_files.php nfiles dir

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

if ($argc < 3) {
    echo "Usage: bossa_example_make_files nfiles dir\n";
    exit;
}

$n = $argv[1];
$dir = $argv[2];

if (!is_dir($dir)) {
    echo "$dir is not a directory\n";
    exit;
}

for ($i=0; $i<$n; $i++) {
    $path = "$dir/$i.png";
    $case = make_test_case();
    imagepng(make_image($case), $path);
}

?>
