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
require_once("../inc/common_defs.inc");
require_once("../project/project.inc");

function db_init_xml() {
    if (web_stopped()) {
        return ERR_PROJECT_DOWN;
    }
    $db = BoincDb::get();
    if (!$db) return ERR_DB_CANT_CONNECT;
    return 0;
}

// write PHP warnings as XML, so that the reply can be parsed by client
//
function error_handler($errno, $errstr, $errfile, $errline) {
    if ($errno == E_WARNING) $errno = "Warning";
    if ($errno == E_NOTICE) $errno = "Notice";
    echo "<error>
    <error_msg><![CDATA[$errstr]]></error_msg>
    <type>$errno</type>
    <file>$errfile</file>
    <line>$errline</line>
</error>
";
}


function xml_header() {
    global $generating_xml;
    if (defined("RPC_DEBUG") && RPC_DEBUG) {
        set_error_handler('error_handler', E_WARNING|E_NOTICE);
    }
    header('Content-type: text/xml');
    echo "<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n";
    $generating_xml = true;
}

$xml_outer_tag = "";

function xml_start_tag($tag) {
    global $xml_outer_tag;
    echo "<$tag>\n";
    $xml_outer_tag = $tag;
}

// used only if host owner is the requesting user
//
function show_host_xml($host) {
    echo "    <host>
        <id>$host->id</id>
        <create_time>$host->create_time</create_time>
        <rpc_seqno>$host->rpc_seqno</rpc_seqno>
        <rpc_time>$host->rpc_time</rpc_time>
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

// $show_hosts is true only if $user is the logged-in user
//
function show_user_xml($user, $show_hosts) {
    $cpid = md5($user->cross_project_id.$user->email_addr);
    echo "<user>
    <id>$user->id</id>
    <cpid>$cpid</cpid>
    <create_time>$user->create_time</create_time>
    <name>".htmlspecialchars($user->name)."</name>
    <total_credit>$user->total_credit</total_credit>
    <expavg_credit>$user->expavg_credit</expavg_credit>
    <expavg_time>$user->expavg_time</expavg_time>
    <teamid>$user->teamid</teamid>
    <has_profile>$user->has_profile</has_profile>
";
    if ($show_hosts) {
        $hosts = BoincHost::enum("userid=$user->id");
        echo "   <venue>$user->venue</venue>\n";
        foreach ($hosts as $host) {
            show_host_xml($host);
        }
    }
    if (USER_COUNTRY) {
        echo "     <country>$user->country</country>\n";
    }
    if (USER_URL && $user->url) {
        $url = normalize_user_url($user->url);
        echo "    <url>".htmlspecialchars($url)."</url>\n";
    }
    echo "</user>\n";
}

function show_team_member($user, $creditonly = false) {
    if ($creditonly && !$user->total_credit) { return; }
    $cpid = md5($user->cross_project_id.$user->email_addr);
    echo "<user>
    <id>$user->id</id>
    <cpid>$cpid</cpid>
    <total_credit>$user->total_credit</total_credit>";
    if (!$creditonly) {
        echo "    <create_time>$user->create_time</create_time>
    <name>".htmlspecialchars($user->name)."</name>
    <expavg_credit>$user->expavg_credit</expavg_credit>
    <expavg_time>$user->expavg_time</expavg_time>
    <has_profile>$user->has_profile</has_profile>
";
        if (USER_COUNTRY) {
            echo "     <country>$user->country</country>\n";
        }
        if (USER_URL && $user->url) {
            $url = normalize_user_url($user->url);
            echo "    <url>".htmlspecialchars($url)."</url>\n";
        }
    }
    echo "</user>
";
}

function show_team_xml($team) {
    echo "<team>
    <id>$team->id</id>
    <create_time>$team->create_time</create_time>
    <userid>$team->userid</userid>
    <name>".htmlspecialchars($team->name)."</name>
    <url>".htmlspecialchars($team->url)."</url>
    <type>$team->type</type>
    <country>$team->country</country>
    <total_credit>$team->total_credit</total_credit>
    <expavg_credit>$team->expavg_credit</expavg_credit>
    <expavg_time>$team->expavg_time</expavg_time>
</team>
";
}

?>
