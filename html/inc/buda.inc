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

// return list of variant dir names
//
function get_buda_variants($app_name) {
    global $buda_root;
    $x = [];
    $app_dir = "$buda_root/$app_name";
    $dirs = scandir($app_dir);
    foreach ($dirs as $dir) {
        if ($dir[0] == '.') continue;
        if (!is_dir("$app_dir/$dir")) continue;
        if (!file_exists("$app_dir/$dir/variant.json")) continue;
        $x[] = $dir;
    }
    return $x;
}

function get_buda_app_desc($app) {
    global $buda_root;
    $path = "$buda_root/$app/desc.json";
    return json_decode(file_get_contents($path));
}

function get_buda_var_desc($app, $var) {
    global $buda_root;
    $path = "$buda_root/$app/$var/variant.json";
    return json_decode(file_get_contents($path));
}

function variant_name($desc) {
    if ($desc->plan_class) {
        return "$desc->cpu_type ($desc->plan_class)";
    } else {
        return "$desc->cpu_type (CPU)";
    }
}

?>
