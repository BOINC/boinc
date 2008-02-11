<?php

// PHP versions of functions in sched/sched_util.C

function filename_hash($filename, $fanout) {
    $m = md5($filename);
    $s = substr($m, 1, 7);
    sscanf($s, "%x", $n);
    return sprintf("%x", $n%$fanout);
}

function dir_hier_path($filename, $root, $fanout) {
    $dir = filename_hash($filename, $fanout);
    $dirpath = "$root/$dir";
    if (!is_dir($dirpath)) {
        mkdir($dirpath);
    }
    return "$dirpath/$filename";
}

function dir_hier_url($filename, $base, $fanout) {
    $dir = filename_hash($filename, $fanout);
    return "$base/$dir/$filename";
}

?>
