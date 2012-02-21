<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

// output XML summary of app versions

require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");

BoincDb::get(true);
xml_header();
echo "<app_versions>\n";
$app_versions = BoincAppVersion::enum("deprecated=0 and pfc_n>0");
foreach ($app_versions as $av) {
    $platform = BoincPlatform::lookup_id($av->platformid);
    $app = BoincApp::lookup_id($av->appid);
    echo "
    <app_version>
        <id>$av->id</id>
        <create_time>$av->create_time</create_time>
        <platform>$platform->name</platform>
        <app_name>$app->name</app_name>
        <version_num>$av->version_num</version_num>
        <plan_class>$av->plan_class</plan_class>
        <pfc_n>$av->pfc_n</pfc_n>
        <pfc_avg>$av->pfc_avg</pfc_avg>
        <pfc_scale>$av->pfc_scale</pfc_scale>
        <expavg_credit>$av->expavg_credit</expavg_credit>
        <expavg_time>$av->expavg_time</expavg_time>
    </app_version>
";
}
echo "</app_versions>\n";
?>
