<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// Convert miscellaneous host info from old format to JSON
// Should have to run this just once.

require_once('../inc/boinc_db.inc');
require_once('../inc/host.inc');

function make_gpu($type, $x) {
    $g = new StdClass();
    $g->type = $type;
    $g->model = $x[1];
    if (!empty($x[2])) {
        $g->count = intval($x[2]);
    }
    if (!empty($x[3])) {
        $g->ram_mb = intval($x[3]);
    }
    if (!empty($x[4])) {
        $g->driver_version = $x[4];
    }
    if (!empty($x[5])) {
        $g->opencl_version = $x[5];
    }
    return $g;
}

function make_vbox($x) {
    $v = new StdClass;
    $v->version = $x[1];
    if (count($x) >= 4) {
        $v->hw_accel = $x[2]?true:false;
        $v->hw_accel_enabled = $x[3]?true:false;
    }
    return $v;
}

function make_docker($x) {
    $d = new StdClass;
    $d->version = $x[1];
    $d->type = intval($x[2]);
    return $d;
}

function to_struct($p) {
    $s = new StdClass;
    $gpus = [];
    $config = [];
    foreach ($p as $x) {
        switch ($x[0]) {
        case 'BOINC':
            $s->client_version = $x[1];
            if (!empty($x[2])) {
                $s->client_brand = $x[2];
            }
            break;
        case 'CUDA':
            $g = make_gpu('nvidia', $x);
            if ($g) $gpus[] = $g;
            break;
        case 'CAL':
            $g = make_gpu('amd', $x);
            if ($g) $gpus[] = $g;
            break;
        case 'INTEL':
            $g = make_gpu('intel', $x);
            if ($g) $gpus[] = $g;
            break;
        case 'apple_gpu':
            $g = make_gpu('apple', $x);
            if ($g) $gpus[] = $g;
            break;
        case 'opencl_gpu':
            if (stristr($x[1], 'apple')) {
                break;
            }
            if (stristr($x[1], 'amd')) {
                break;
            }
            $g = make_gpu('opencl', $x);
            if ($g) $gpus[] = $g;
            break;
        case 'vbox':
            $s->vbox = make_vbox($x);
            break;
        case 'docker':
            $s->docker = make_docker($x);
            break;
        case 'dont_use_docker':
            $config['dont_use_docker'] = true;
            break;
        case 'dont_use_wsl':
            $config['dont_use_wsl'] = true;
            break;
        default:
            echo "unrecognized: ".$x[0]."\n";
            continue;
        }
    }
    if ($gpus) $s->gpus = $gpus;
    if ($config) $s->config = $config;
    return $s;
}

function parse_serialnum($serialnum) {
    $parts = explode('[', $serialnum);
    $ret = [];
    foreach ($parts as $part) {
        if (!$part) continue;
        $part = substr($part, 0, -1);
        $f = explode('|', $part);
        if (!$f) continue;
        $ret[$f[0]] = $f;
    }
    return $ret;
}

function main() {
    $hosts = BoincHost::enum('', '');
    foreach ($hosts as $h) {
        //echo "$h->serialnum\n";
        $p = parse_serialnum($h->serialnum);
        //print_r($p);
        $s = to_struct($p);
        //print_r($s);
        $s = json_encode($s, JSON_PRETTY_PRINT);
        $s = BoincDb::escape_string($s);
        $h->update("misc='$s'");
        echo "$h->id\n";
    }
}

main();

?>
