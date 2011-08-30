<?php

require_once("../inc/util.inc");

function get_gpu_model($x, $vendor) {
    $descs = explode("]", $x);
    array_pop($descs);
    foreach ($descs as $desc) {
        $desc = trim($desc, "[");
        $d = explode("|", $desc);
        if ($d[0] == "BOINC") continue;
        if ($d[0] != $vendor) continue;
        return $d[1];
    }
    return null;
}

function add_model($model, &$models) {
    if (array_key_exists($model, $models)) {
        $models[$model]++;
    } else {
        $models[$model] = 1;
    }
}

// return a data structure containing GPU usage info for a vendor
// $x->total: combined list
// $x->windows
// $x->linux
// $x->mac
//
function get_gpu_list($vendor) {
    $avs = BoincAppVersion::enum("plan_class like '%$vendor%'");

    $av_ids = "";
    foreach($avs as $av) {
        $av_ids .= "$av->id, ";
    }
    $av_ids .= "0";

    $t = time() - 30*86400;
    $results = BoincResult::enum("app_version_id in ($av_ids) and create_time > $t limit 1000");
    $total = array();
    $win = array();
    $linux = array();
    $mac = array();
    foreach ($results as $r) {
        $h = BoincHost::lookup_id($r->hostid);
        if (!$h) continue;
        $v = $vendor=="cuda"?"CUDA":"ATI";
        $model = get_gpu_model($h->serialnum, $v);
        if (!$model) continue;
        add_model($model, $total);
        if (strstr($h->os_name, "Windows")) {
            add_model($model, $win);
        }
        if (strstr($h->os_name, "Linux")) {
            add_model($model, $linux);
        }
        if (strstr($h->os_name, "Darwin")) {
            add_model($model, $mac);
        }

    }
    $x = null;
    $x->total = $total;
    $x->win = $win;
    $x->linux = $linux;
    $x->mac = $mac;
    return $x;
}

function get_gpu_lists() {
    $x = null;
    $x->cuda = get_gpu_list("cuda");
    $x->ati = get_gpu_list("ati");
    return $x;
}

function show_list($models, $name) {
    echo "<td><h2>$name</h2>\n";
    if (!count($models)) {
        echo tra("No GPU tasks reported")."</td>\n";
        return;
    }
    arsort($models);
    echo "<ol>\n";
    $first = true;
    foreach ($models as $model=>$n) {
        if ($first) {
            $max = $n;
            $first = false;
        }
        if ($n < $max/10) break;
        echo "<li>$model\n";
    }
    echo "</ol></td>\n";
}

function show_vendor($vendor, $x) {
    echo "<h2>$vendor</h2>\n";
    if (!count($x->total)) {
        echo tra("No GPU tasks reported");
        return;
    }
    $have_win = count($x->win)>0;
    $have_mac = count($x->mac)>0;
    $have_linux = count($x->linux)>0;
    $n = 0;
    if ($have_win) $n++;
    if ($have_mac) $n++;
    if ($have_linux) $n++;
    $show_total = $n>1;
    start_table();
    echo "<tr>";
    if ($show_total) {
        show_list($x->total, "Total");
    }
    show_list($x->win, "Windows");
    show_list($x->linux, "Linux");
    show_list($x->mac, "Mac");
    echo "</tr></table>\n";
}

$d = get_cached_data(86400);
if ($d) {
    $data = unserialize($d);
} else {
    $data = get_gpu_lists();
    set_cached_data(86400, serialize($data));
}

page_head(tra("Top GPU models"));
echo tra("The following lists show the most productive GPU models on different platforms.");
show_vendor("NVIDIA", $data->cuda);
show_vendor("ATI/AMD", $data->ati);
page_tail();
$x = get_gpu_list("cuda");


?>
