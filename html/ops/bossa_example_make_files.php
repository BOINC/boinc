<?php

// Generate image files and answer files for Bossa example
// Usage:
//
// bossa_example_make_files.php options
// --nfiles N           how many files to create
// --dir dir            where to put them (e.g., ../user/example)
// --ellipse_frac X     fraction of images with ellipses

require_once("../inc/util_basic.inc");

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

function make_test_case($ellipse_frac) {
    $case = null;
    $case->have_ellipse = drand() < $ellipse_frac;
    if ($case->have_ellipse) {
        $case->cx = rand(50, 550);
        $case->cy = rand(50, 350);
        $case->w = rand(50, 100);
        $case->h = rand(50, 100);
    }
    return $case;
}

function usage() {
    global $argv;
    exit("Usage: ".$argv[0]." --nfiles N --dir dir [--ellipse_frac x]\n");
}

$nfiles = 0;
$dir = null;
$ellipse_frac = 0.5;
for ($i=1; $i<$argc; $i++) {
    if ($argv[$i] == '--nfiles') $nfiles = $argv[++$i];
    elseif ($argv[$i] == '--dir') $dir = $argv[++$i];
    elseif ($argv[$i] == '--ellipse_frac') $ellipse_frac = $argv[++$i];
    else usage();
}

if (!$nfiles || !$dir) usage();

if (!is_dir($dir)) {
    exit("$dir is not a directory\n");
}

for ($i=0; $i<$nfiles; $i++) {
    $path = "$dir/$i.png";
    $anspath = "$dir/$i.ans";
    $case = make_test_case($ellipse_frac);
    $f = fopen($anspath, 'w');
    fwrite($f, serialize($case));
    fclose(f);
    imagepng(make_image($case), $path);
}

?>
