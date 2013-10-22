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

// size_census
// for each multi-size app,
// find the N quantiles of its effective speed,
// and write them to a file.
// See http://boinc.berkeley.edu/trac/wiki/JobSizeMatching

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/boinc_db.inc");

function do_app($app) {
    // enumerate the host_app_versions for this app,
    // joined to the host

    $db = BoincDb::get();
    $dbn = $db->db_name;
    $query = "select et_avg, host.on_frac, host.active_frac " .
        " from $dbn.host_app_version, $dbn.host, $dbn.app_version " .
        " where host_app_version.app_version_id = app_version.id " .
        " and app_version.appid = 1 " .
        " and et_n > 0 " .
        " and host.id = host_app_version.host_id";
    $result = $db->do_query($query);
    $a = array();
    while ($x = mysql_fetch_object($result)) {
        $a[] = (1/$x->et_avg) * $x->on_frac * $x->active_frac;
    }
    mysql_free_result($result);
    sort($a);
    $n = count($a);
    $f = fopen("../../size_census_".$app->name, "w");
    for ($i=1; $i<$app->n_size_classes; $i++) {
        $k = (int)(($i*$n)/$app->n_size_classes);
        fprintf($f, "%e\n", $a[$k]);
    }
    fclose($f);
}

$apps = BoincApp::enum("deprecated=0 and n_size_classes>1");
foreach ($apps as $app) {
    do_app($app);
}

?>
