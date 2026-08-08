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

// code to get list of app types (CPU/GPU) supported by project

require_once("../inc/boinc_db.inc");

// return a structure indicating whether project can use
// various resource types, and a count of apps.
// Include both non-deprecated app versions and BUDA app variants.
//
function get_app_types() {
    $t = new StdClass;
    $t->cpu = false;
    $t->cuda = false;
    $t->ati = false;
    $t->intel_gpu = false;
    $t->apple_gpu = false;
    $t->count = 0;

    $avs = BoincAppVersion::enum("deprecated=0");
    foreach ($avs as $av) {
        do_plan_class($av->plan_class, $t);
    }

    $fname = '../../buda_plan_classes';
    if (file_exists($fname)) {
        $pcs = file($fname);
        foreach ($pcs as $pc) {
            do_plan_class($pc, $t);
        }
    }
    return $t;
}

function do_plan_class($plan_class, &$t) {
    if (strstr($plan_class, "ati")) {
        $t->ati = true;
        $t->count++;
    } else if (strstr($plan_class, "amd")) {
        $t->ati = true;
        $t->count++;
    } else if (strstr($plan_class, "cuda")) {
        $t->cuda = true;
        $t->count++;
    } else if (strstr($plan_class, "nvidia")) {
        $t->cuda = true;
        $t->count++;
    } else if (strstr($plan_class, "intel_gpu")) {
        $t->intel_gpu = true;
        $t->count++;
    } else if (strstr($plan_class, "apple_gpu")) {
        $t->apple_gpu = true;
        $t->count++;
    } else {
        $t->cpu = true;
        $t->count++;
    }
}

?>
