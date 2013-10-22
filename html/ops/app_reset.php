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

// script for resetting an app's credit and runtime estimation statistics;
// use this if these got messed up because of bad FLOPs estimates
//

require_once("../inc/util_ops.inc");

if (!file_exists("../../stop_daemons")) {
    admin_error_page("Project must be stopped");
}

// PFC is based on workunit.rsc_fpops_est.
// If this was bad, all PFC info is bad.
// So we need to zero it out everywhere
//
function reset_app($app) {
    $avs = BoincAppVersion::enum("appid=$app->id");
    foreach ($avs as $av) {
        $av->update("pfc_n=0, pfc_avg=0, pfc_scale=0, expavg_credit=0, expavg_time=0");
        BoincHostAppVersion::update_aux("pfc_n=0, pfc_avg=0, et_n=0, et_avg=0, et_var=0, et_q=0 where app_version_id=$av->id");
    }
    $app->update("min_avg_pfc = 0");
}

$appid = get_int("appid");
$app = BoincApp::lookup_id($appid);
if (!$app) admin_error_page("no such app");

$confirmed = get_int("confirmed", true);

if ($confirmed) {
    reset_app($app);
    admin_page_head("Application reset completed");
    admin_page_tail();
} else {
    admin_page_head("Confirm: reset $app->name");
    echo "
        This operation will zero out the statistics used to calculate credit.
        It may take a while to regenerate these statistics.
        Are you sure you want to do this?
        <p>
        <a href=app_reset.php?appid=$appid&confirmed=1>Yes</a>
    ";
    admin_page_tail();
}
