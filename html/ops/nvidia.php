<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// script for studying NVIDIA GPUs on hosts

$cli_only = true;
require_once("../inc/util_ops.inc");

ini_set ("memory_limit", "8000M");
set_time_limit(0);

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

$hosts_total = 0;     // number of hosts 6.2 or better
$hosts_gpu = 0;     // number with an nvidia gpu
$rac_total =0;
$rac_gpu = 0;
$linux_total = 0;
$linux_gpus = 0;
$windows_total = 0;
$windows_gpus = 0;
$model = array();   // name -> count
$ram = array();     // size -> count
$driver = array();  // vers -> count
$ngpus = array();   // ngpus -> count

function inc(&$ar, $ind) {
    if (array_key_exists($ind, $ar)) {
        $ar[$ind]++;
    } else {
        $ar[$ind] = 1;
    }
}

function parse_vers($x) {
    $y = strstr($x, 'BOINC');
    if (!$y) return '';
    $y = substr($y, 6);
    $z = explode("]", $y, 2);
    $y = explode(".", $z[0]);
    $v->major = $y[0];
    $v->minor = $y[1];
    return $v;
}

function parse_cuda($x) {
    $y = strstr($x, 'CUDA');
    if (!$y) return '';
    $y = substr($y, 5);
    $z = explode("]", $y, 2);
    $y = explode("|", $z[0]);
    $g->model = $y[0];
    $g->ngpus = $y[1];
    $ram = (int)$y[2];
    $ram += 63;
    $ram /= 64;
    $ram = (int)$ram;
    $ram *= 64;
    $g->ram = (int)($ram);
    $d = $y[3];
    $d /= 100;
    $g->driver = (int)$d;
    return $g;
}

$hosts = BoincHost::enum("expavg_credit > 10 and serialnum<>''");
foreach($hosts as $host) {
    $boinc_vers = parse_vers($host->serialnum);
    if (!$boinc_vers) continue;
    if ($boinc_vers->major < 6) continue;
    $is_linux = false;
    if (strstr($host->os_name, "Linux")) {
        $linux_total++;
        $is_linux = true;
    } else if (strstr($host->os_name, "Windows")) {
        $windows_total++;
    } else {
        continue;
    }
    $hosts_total++;
    $rac_total += $host->expavg_credit;
    $gpu = parse_cuda($host->serialnum);
    if (!$gpu) {
        continue;
    }
    $hosts_gpu++;
    $rac_gpu += $host->expavg_credit;
    if ($is_linux) {
        $linux_gpus++;
    } else {
        $windows_gpus++;
    }
    inc($model, $gpu->model);
    inc($ram, $gpu->ram);
    inc($driver, $gpu->driver);
    inc($ngpus, $gpu->ngpus);
}

$pct = 100*($hosts_gpu/$hosts_total);
echo "ntotal: $hosts_total   ngpus: $hosts_gpu ($pct %)\n";
$pct = 100*($windows_gpus/$windows_total);
echo "Windows: total $windows_total gpus $windows_gpus ($pct %)\n";
$pct = 100*($linux_gpus/$linux_total);
echo "Linux: total $linux_total gpus $linux_gpus ($pct %)\n";

$rac_non_gpu = $rac_total - $rac_gpu;
$hosts_non_gpu = $hosts_total - $hosts_gpu;
$a = $rac_gpu/$hosts_gpu;
$b = $rac_non_gpu/$hosts_non_gpu;
echo "Avg RAC: GPU: $a non-GPU: $b\n";

arsort($model);
foreach($model as $m=>$c) {
    echo "$m $c\n";
}
print_r($ram);
print_r($driver);
print_r($ngpus);

?>
