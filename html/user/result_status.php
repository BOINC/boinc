<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2016 University of California
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

// RPC handler for getting the status of one or more results
// args:
// ids=id1,id2...
// or
// names = name1,name2,...

require_once("../inc/util.inc");
require_once("../inc/xml.inc");

function result_xml($r) {
    echo "
    <result>
        <id>$r->id</id>
        <create_time>$r->create_time</create_time>
        <workunitid>$r->workunitid</workunitid>
        <server_state>$r->server_state</server_state>
        <outcome>$r->outcome</outcome>
        <client_state>$r->client_state</client_state>
        <hostid>$r->hostid</hostid>
        <userid>$r->userid</userid>
        <report_deadline>$r->report_deadline</report_deadline>
        <sent_time>$r->sent_time</sent_time>
        <received_time>$r->received_time</received_time>
        <name>$r->name</name>
        <cpu_time>$r->cpu_time</cpu_time>
        <batch>$r->batch</batch>
        <file_delete_state>$r->file_delete_state</file_delete_state>
        <validate_state>$r->validate_state</validate_state>
        <granted_credit>$r->granted_credit</granted_credit>
        <app_version_num>$r->app_version_num</app_version_num>
        <appid>$r->appid</appid>
        <exit_status>$r->exit_status</exit_status>
        <elapsed_time>$r->elapsed_time</elapsed_time>
        <flops_estimate>$r->flops_estimate</flops_estimate>
        <peak_working_set_size>$r->peak_working_set_size</peak_working_set_size>
        <peak_swap_size>$r->peak_swap_size</peak_swap_size>
        <peak_disk_usage>$r->peak_disk_usage</peak_disk_usage>
    </result>
";
}

BoincDb::get(true); // read-only; use replica DB if possible

xml_header();
echo "<results>\n";
$ids = get_str("ids", true);
if ($ids) {
    $ids = explode(",", $ids);
    foreach ($ids as $id) {
        $result = BoincResult::lookup_id($id);
        if ($result) {
            result_xml($result);
        } else {
            echo "<error>ID $id unknown</error>\n";
        }
    }
} else {
    $names = get_str("names", true);
    $names = explode(",", $names);
    foreach ($names as $name) {
        $result = BoincResult::lookup_name($name);
        if ($result) {
            result_xml($result);
        } else {
            echo "<error>name $name unknown</error>\n";
        }
    }
}
echo "</results>\n";

?>
