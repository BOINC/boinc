#! /usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2013 University of California
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

// size_census [--all_apps]
// for each multi-size app,
// find the N quantiles of its effective speed,
// and write them to a file.
// See https://github.com/BOINC/boinc/wiki/JobSizeMatching
//
// --all_apps: compute quantiles for all apps;
// use this during setup and testing.

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/util.inc");

function do_app($app) {
    // enumerate the host_app_versions for this app,
    // joined to the host

    $db = BoincDb::get();
    $query = "select et_avg, host.on_frac, host.active_frac, host.gpu_active_frac, app_version.plan_class " .
        " from DBNAME.host_app_version, DBNAME.host, DBNAME.app_version " .
        " where host_app_version.app_version_id = app_version.id " .
        " and app_version.appid = $app->id " .
        " and et_n > 0  and et_avg > 0 " .
        " and host.id = host_app_version.host_id " .
        " and host.expavg_credit > 10";
    $result = $db->do_query($query);
    $a = array();
    while ($x = _mysql_fetch_object($result)) {
        if (is_gpu($x->plan_class)) {
            $av = $x->on_frac;
            if ($x->gpu_active_frac) {
                $av *= $x->gpu_active_frac;
            } else {
                $av *= $x->active_frac;
            }
        } else {
            $av = $x->on_frac * $x->active_frac;
        }
        $a[] = (1/$x->et_avg) * $av;
    }
    _mysql_free_result($result);
    sort($a);
    $n = count($a);
    $f = fopen("../../size_census_".$app->name, "w");
    for ($i=1; $i<$app->n_size_classes; $i++) {
        $k = (int)(($i*$n)/$app->n_size_classes);
        fprintf($f, "%e\n", $a[$k]);
    }
    fclose($f);
}

echo "Starting: ", time_str(time()), "\n";

if ($argc == 2 && $argv[1]=="--all_apps") {
    $apps = BoincApp::enum("deprecated=0");
} else {
    $apps = BoincApp::enum("deprecated=0 and n_size_classes>1");
}

foreach ($apps as $app) {
    do_app($app);
}
echo "Finished: ", time_str(time()), "\n";

?>
