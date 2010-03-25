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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");

function db_init_xml() {
    if (web_stopped()) {
        return -183;
    }
    $db = BoincDb::get();
    if (!$db) return -138;
    return 0;
}

function xml_header() {
    global $generating_xml;
    header('Content-type: text/xml');
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
    $generating_xml = true;
}

function show_host_xml($host) {
    echo "    <host>
        <id>$host->id</id>
        <create_time>$host->create_time</create_time>
        <rpc_seqno>$host->rpc_seqno</rpc_seqno>
        <host_cpid>$host->host_cpid</host_cpid>
        <total_credit>$host->total_credit</total_credit>
        <expavg_credit>$host->expavg_credit</expavg_credit>
        <expavg_time>$host->expavg_time</expavg_time>
        <domain_name>$host->domain_name</domain_name>
        <p_ncpus>$host->p_ncpus</p_ncpus>
        <p_vendor>$host->p_vendor</p_vendor>
        <p_model>$host->p_model</p_model>
        <p_fpops>$host->p_fpops</p_fpops>
        <p_iops>$host->p_iops</p_iops>
        <os_name>$host->os_name</os_name>
        <os_version>$host->os_version</os_version>
        <m_nbytes>$host->m_nbytes</m_nbytes>
        <d_free>$host->d_free</d_free>
        <d_total>$host->d_total</d_total>
        <venue>$host->venue</venue>
    </host>
";
}

function show_user_xml($user, $show_hosts) {
    $cpid = md5($user->cross_project_id.$user->email_addr);
    echo "<user>
    <id>$user->id</id>
    <cpid>$cpid</cpid>
    <create_time>$user->create_time</create_time>
    <name>".htmlspecialchars($user->name)."</name>
    <country>$user->country</country>
    <total_credit>$user->total_credit</total_credit>
    <expavg_credit>$user->expavg_credit</expavg_credit>
    <expavg_time>$user->expavg_time</expavg_time>
    <teamid>$user->teamid</teamid>
    <url>".htmlspecialchars($user->url)."</url>
    <has_profile>$user->has_profile</has_profile>
";
    if ($show_hosts) {
        $hosts = BoincHost::enum("userid=$user->id");
        echo "   <venue>$user->venue</venue>\n";
        foreach ($hosts as $host) {
            show_host_xml($host);
        }
    }
echo"</user>
";
}

function show_team_member($user, $show_email) {
    $cpid = md5($user->cross_project_id.$user->email_addr);
    echo "<user>
    <id>$user->id</id>
";
    if ($show_email && $user->send_email) {
        echo "<email_addr>$user->email_addr</email_addr>
";
    }
    echo "<cpid>$cpid</cpid>
    <create_time>$user->create_time</create_time>
    <name>".htmlspecialchars($user->name)."</name>
    <country>$user->country</country>
    <total_credit>$user->total_credit</total_credit>
    <expavg_credit>$user->expavg_credit</expavg_credit>
    <expavg_time>$user->expavg_time</expavg_time>
    <url>".htmlspecialchars($user->url)."</url>
    <has_profile>$user->has_profile</has_profile>
</user>
";
}

function show_team_xml($team) {
    echo "<team>
    <id>$team->id</id>
    <create_time>$team->create_time</create_time>
    <userid>$team->userid</userid>
    <name>".htmlspecialchars($team->name)."</name>
    <url>$team->url</url>
    <type>$team->type</type>
    <country>$team->country</country>
    <total_credit>$team->total_credit</total_credit>
    <expavg_credit>$team->expavg_credit</expavg_credit>
    <expavg_time>$team->expavg_time</expavg_time>
</team>
";
}

?>
