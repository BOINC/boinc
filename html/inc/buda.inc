<?php

$buda_root = "../../buda_apps";

function get_buda_apps() {
    global $buda_root;
    $apps = [];
    $x = scandir($buda_root);
    foreach ($x as $app) {
        if ($app[0] == '.') continue;
        if (!is_dir("$buda_root/$app")) continue;
        $apps[] = $app;
    }
    return $apps;
}

function get_buda_variants($app_name) {
    global $buda_root;
    $x = [];
    $dir = "$buda_root/$app_name";
    $pcs = scandir($dir);
    foreach ($pcs as $pc) {
        if ($pc[0] == '.') continue;
        if (!is_dir("$dir/$pc")) continue;
        $x[] = $pc;
    }
    return $x;
}

function get_buda_desc($app) {
    global $buda_root;
    $path = "$buda_root/$app/desc.json";
    return json_decode(file_get_contents($path));
}

?>
