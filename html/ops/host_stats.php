<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// show counts of Linux hosts broken down by libc and vbox version

require_once("../inc/boinc_db.inc");

$max_days = 30;
    // include only hosts who have done an RPC within this many days

function is_linux($host) {
    return strstr($host->os_name, 'Linux');
}

function libc_version($host) {
    $p = $host->os_version;
    $n = strpos($p, 'libc ');
    if ($n === false) return '';
    $m = strpos($p, ']', $n);
    if ($m === false) return '';
    $x = substr($p, $n+5, $m-$n-5);

    $n = strpos($x, ' ');
    if ($n !== false) return substr($x, 0, $n);
    return $x;

}

function vbox_version($host) {
    $p = $host->serialnum;
    $n = strpos($p, 'vbox|');
    if ($n === false) return '';
    $m = strpos($p, ']', $n);
    if ($m === false) return '';
    $x = substr($p, $n+5, $m-$n-5);
    $n = strpos($x, '_');
    if ($n !== false) return substr($x, 0, $n);
    $n = strpos($x, 'r');
    if ($n !== false) return substr($x, 0, $n);
    return $x;
}

function main() {
    global $max_days;
    $t = time()-$max_days*86400;
    $hosts = BoincHost::enum("rpc_time>$t");
    $descs = [];
    foreach ($hosts as $host) {
        if (!is_linux($host)) continue;
        $d = new StdClass;
        $d->libc = libc_version($host);
        $d->vbox = vbox_version($host);
        $descs[] = $d;
    }
    show($descs);
}

// show:
// counts by libc version
// counts by libc version for hosts w/ vbox
// counts by vbox version

function show($descs) {
    global $max_days;
    $libc = [];
    $libc2 = [];
    $vbox = [];
    $nv = 0;
    foreach ($descs as $d) {
        if (array_key_exists($d->libc, $libc)) {
            $libc[$d->libc] += 1;
        } else {
            $libc[$d->libc] = 1;
        }
        if (!$d->vbox) continue;
        $nv++;
        if (array_key_exists($d->vbox, $vbox)) {
            $vbox[$d->vbox] += 1;
        } else {
            $vbox[$d->vbox] = 1;
        }
        if (array_key_exists($d->libc, $libc2)) {
            $libc2[$d->libc] += 1;
        } else {
            $libc2[$d->libc] = 1;
        }
    }

    $n = count($descs);
    echo "$n Linux hosts active in last $max_days days\n";
    ksort($libc);
    echo "libc versions:\n";
    foreach ($libc as $x=>$count) {
        if (!$x) continue;
        echo "$x: $count\n";
    }
    ksort($vbox);
    echo "vbox versions ($nv hosts):\n";
    foreach ($vbox as $x=>$count) {
        if (!$x) continue;
        echo "$x: $count\n";
    }
    ksort($libc2);
    echo "libc versions of hosts with vbox:\n";
    foreach ($libc2 as $x=>$count) {
        if (!$x) continue;
        echo "$x: $count\n";
    }
}

main();
?>
