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
function show_user_xml($user, $show_hosts, $project_rain) {
    $cpid = md5($user->cross_project_id.$user->email_addr);
    echo "<user>
    <id>$user->id</id>
    <cpid>$cpid</cpid>
    <create_time>$user->create_time</create_time>
    <name>".htmlspecialchars($user->name)."</name>
    <country>$user->country</country>
    <bitshares>$project_rain->bitshares</bitshares>
    <steem>$project_rain->steem</steem>
    <gridcoin>$project_rain->gridcoin</gridcoin>
    <ethereum>$project_rain->ethereum</ethereum>
    <ethereum_classic>$project_rain->ethereum_classic</ethereum_classic>    
    <golem>$project_rain->golem</golem>
    <nxt>$project_rain->nxt</nxt>
    <ardor>$project_rain->ardor</ardor>
    <hyperledger_sawtooth_lake>$project_rain->hyperledger_sawtooth_lake</hyperledger_sawtooth_lake>
    <hyperledger_fabric>$project_rain->hyperledger_fabric</hyperledger_fabric>
    <hyperledger_misc>$project_rain->hyperledger_misc</hyperledger_misc>
    <waves>$project_rain->waves</waves>
    <peershares>$project_rain->peershares</peershares>
    <omnilayer>$project_rain->omnilayer</omnilayer>
    <counterparty>$project_rain->counterparty</counterparty>
    <peerplays>$project_rain->peerplays</peerplays>
    <storj>$project_rain->storj</storj>
    <nem>$project_rain->nem</nem>
    <ibm_bluemix_blockchain>$project_rain->ibm_bluemix_blockchain</ibm_bluemix_blockchain>
    <coloredcoins>$project_rain->coloredcoins</coloredcoins>
    <heat_ledger>$project_rain->heat_ledger</heat_ledger>
    <antshares>$project_rain->antshares</antshares>
    <lisk>$project_rain->lisk</lisk>
    <decent>$project_rain->decent</decent>
    <synereo>$project_rain->synereo</synereo>
    <lbry>$project_rain->lbry</lbry>
    <wings>$project_rain->wings</wings>
    <hong>$project_rain->hong</hong>
    <boardroom>$project_rain->boardroom</boardroom>
    <expanse>$project_rain->expanse</expanse>
    <akasha>$project_rain->akasha</akasha>
    <cosmos>$project_rain->cosmos</cosmos>
    <metaverse>$project_rain->metaverse</metaverse>
    <zcash>$project_rain->zcash</zcash>
    <stratis>$project_rain->stratis</stratis>
    <echo>$project_rain->echo</echo>
    <tox>$project_rain->tox</tox>
    <retroshare>$project_rain->retroshare</retroshare>
    <wickr>$project_rain->wickr</wickr>
    <ring>$project_rain->ring</ring>
    <pgp>$project_rain->pgp</pgp>
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

function show_team_member($user, $project_rain, $creditonly = false) {
    if ($creditonly && !$user->total_credit) { return; }
    $cpid = md5($user->cross_project_id.$user->email_addr);
    echo "<user>
    <id>$user->id</id>
    <cpid>$cpid</cpid>
    <total_credit>$user->total_credit</total_credit>";
    if (!$creditonly) {
        echo "    <create_time>$user->create_time</create_time>
    <name>".htmlspecialchars($user->name)."</name>
    <country>$user->country</country>
    <bitshares>$project_rain->bitshares</bitshares>
    <steem>$project_rain->steem</steem>
    <gridcoin>$project_rain->gridcoin</gridcoin>
    <ethereum>$project_rain->ethereum</ethereum>
    <ethereum_classic>$project_rain->ethereum_classic</ethereum_classic>    
    <golem>$project_rain->golem</golem>
    <nxt>$project_rain->nxt</nxt>
    <ardor>$project_rain->ardor</ardor>
    <hyperledger_sawtooth_lake>$project_rain->hyperledger_sawtooth_lake</hyperledger_sawtooth_lake>
    <hyperledger_fabric>$project_rain->hyperledger_fabric</hyperledger_fabric>
    <hyperledger_misc>$project_rain->hyperledger_misc</hyperledger_misc>
    <waves>$project_rain->waves</waves>
    <peershares>$project_rain->peershares</peershares>
    <omnilayer>$project_rain->omnilayer</omnilayer>
    <counterparty>$project_rain->counterparty</counterparty>
    <heat_ledger>$project_rain->heat_ledger</heat_ledger>
    <peerplays>$project_rain->peerplays</peerplays>
    <storj>$project_rain->storj</storj>
    <nem>$project_rain->nem</nem>
    <ibm_bluemix_blockchain>$project_rain->ibm_bluemix_blockchain</ibm_bluemix_blockchain>
    <coloredcoins>$project_rain->coloredcoins</coloredcoins>
    <antshares>$project_rain->antshares</antshares>
    <lisk>$project_rain->lisk</lisk>
    <decent>$project_rain->decent</decent>
    <synereo>$project_rain->synereo</synereo>
    <lbry>$project_rain->lbry</lbry>
    <wings>$project_rain->wings</wings>
    <hong>$project_rain->hong</hong>
    <boardroom>$project_rain->boardroom</boardroom>
    <expanse>$project_rain->expanse</expanse>
    <akasha>$project_rain->akasha</akasha>
    <cosmos>$project_rain->cosmos</cosmos>
    <metaverse>$project_rain->metaverse</metaverse>
    <zcash>$project_rain->zcash</zcash>
    <stratis>$project_rain->stratis</stratis>
    <echo>$project_rain->echo</echo>
    <tox>$project_rain->tox</tox>
    <retroshare>$project_rain->retroshare</retroshare>
    <wickr>$project_rain->wickr</wickr>
    <ring>$project_rain->ring</ring>
    <pgp>$project_rain->pgp</pgp>
    <expavg_credit>$user->expavg_credit</expavg_credit>
    <expavg_time>$user->expavg_time</expavg_time>
    <url>".htmlspecialchars($user->url)."</url>
    <has_profile>$user->has_profile</has_profile>
";
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
