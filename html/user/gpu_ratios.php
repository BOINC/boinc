<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// generate XML showing the average PFC of GPU versions
// relative to CPU versions.
// This can be used to scale GPU credit
// for projects that have only GPU versions

require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");

$cpu_scale_sum = 0;
$cpu_credit_sum = 0;
$ati_scale_sum = 0;
$ati_credit_sum = 0;
$nvidia_scale_sum = 0;
$nvidia_credit_sum = 0;
$intel_gpu_scale_sum = 0;
$intel_gpu_credit_sum = 0;
$apple_gpu_scale_sum = 0;
$apple_gpu_credit_sum = 0;
$total_credit_sum= 0;

$apps = BoincApp::enum("deprecated=0");
foreach ($apps as $app) {
    $avs = BoincAppVersion::enum("appid=$app->id and deprecated=0");
    foreach ($avs as $av) {
        if (strstr($av->plan_class, "ati")) {
            $ati_scale_sum += $av->pfc_scale * $av->expavg_credit;
            $ati_credit_sum += $av->expavg_credit;
        } else if (strstr($av->plan_class, "nvidia") || strstr($av->plan_class, "cuda")) {
            $nvidia_scale_sum += $av->pfc_scale * $av->expavg_credit;
            $nvidia_credit_sum += $av->expavg_credit;
        } else if (strstr($av->plan_class, "intel_gpu")) {
            $intel_gpu_scale_sum += $av->pfc_scale * $av->expavg_credit;
            $intel_gpu_credit_sum += $av->expavg_credit;
        } else if (strstr($av->plan_class, "apple_gpu")) {
            $apple_gpu_scale_sum += $av->pfc_scale * $av->expavg_credit;
            $apple_gpu_credit_sum += $av->expavg_credit;
        } else {
            $cpu_scale_sum += $av->pfc_scale * $av->expavg_credit;
            $cpu_credit_sum += $av->expavg_credit;
        }
        $total_credit_sum += $av->expavg_credit;
    }
}

xml_header();
echo "<scale_factors>
   <total_credit>$total_credit_sum</total_credit>
";
if ($cpu_credit_sum) {
    echo "   <cpu>", $cpu_scale_sum/$cpu_credit_sum, "</cpu>\n";
}
if ($ati_credit_sum) {
    echo "   <ati>", $ati_scale_sum/$ati_credit_sum, "</ati>\n";
}
if ($nvidia_credit_sum) {
    echo "   <nvidia>", $nvidia_scale_sum/$nvidia_credit_sum, "</nvidia>\n";
}
if ($intel_gpu_credit_sum) {
    echo "   <intel_gpu>", $intel_gpu_scale_sum/$intel_gpu_credit_sum, "</intel_gpu>\n";
}
if ($apple_gpu_credit_sum) {
    echo "   <apple_gpu>", $apple_gpu_scale_sum/$apple_gpu_credit_sum, "</apple_gpu>\n";
}
echo "</scale_factors>\n";
?>
