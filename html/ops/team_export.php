#!/usr/bin/env php

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

// This script for use ONLY by the BOINC-teams project.
// It generates an XML file with team and user info

$cli_only = true;
require_once("../inc/util_ops.inc");

function escape2($strin) {
    $strout = null;

    for ($i = 0; $i < strlen($strin); $i++) {
        $ord = ord($strin[$i]);

        if (($ord > 0 && $ord < 32) || ($ord >= 127)) {
            $strout .= "&amp;#{$ord};";
        } else {
            switch ($strin[$i]) {
            case '<': $strout .= '&lt;'; break;
            case '>': $strout .= '&gt;'; break;
            case '&': $strout .= '&amp;'; break;
            case '"': $strout .= '&quot;'; break;
            default: $strout .= $strin[$i]; }
        }
    }
    return $strout;
}

function escape($strin) {
    $dom = new DOMDocument('1.0');
    $element = $dom->createElement('Element');
    $element->appendChild(
        $dom->createTextNode($strin)
    );

    $dom->appendChild($element);
    $x = $dom->saveXml();
    $x = substr($x, 31);
    $x = substr($x, 0, -11);
    return $x;
}

function handle_team($team, $f) {
    echo "Team: $team->name\n";
    $user = BoincUser::lookup_id($team->userid);
    if (!$user) {
        echo "no user for team $team->id\n";
        return;
    }
    if ($user->teamid != $team->id) {
        echo "Founder is not member of $team->name\n";
        return;
    }
    if (!$user->email_validated) {
        echo "the founder of $team->name, $user->email_addr, is not validated\n";
        return;
    }
    $user_email_munged = str_rot13($user->email_addr);
    fwrite($f, 
"<team>
   <name>".escape($team->name)."</name>
   <url>".escape($team->url)."</url>
   <type>$team->type</type>
   <name_html>".escape($team->name_html)."</name_html>
   <description>
".escape($team->description)."
    </description>
   <country>$team->country</country>
   <id>$team->id</id>
   <user_email_munged>$user_email_munged</user_email_munged>
   <user_name>".escape($user->name)."</user_name>
   <user_country>".escape($user->country)."</user_country>
   <user_postal_code>".escape($user->postal_code)."</user_postal_code>
   <user_bitshares>".escape($project_rain->bitshares)."</user_bitshares>
   <user_steem>".escape($project_rain->steem)."</user_steem>
   <user_gridcoin>".escape($project_rain->gridcoin)."</user_gridcoin>
   <user_ethereum>".escape($project_rain->ethereum)."</user_ethereum>
   <user_ethereum_classic>".escape($project_rain->ethereum_classic)."</user_ethereum_classic>    
   <user_golem>".escape($project_rain->golem)."</user_golem>
   <user_nxt>".escape($project_rain->nxt)."</user_nxt>
   <user_ardor>".escape($project_rain->ardor)."</user_ardor>
   <user_hyperledger_sawtooth_lake>".escape($project_rain->hyperledger_sawtooth_lake)."</user_hyperledger_sawtooth_lake>
   <user_hyperledger_fabric>".escape($project_rain->hyperledger_fabric)."</user_hyperledger_fabric>
   <user_hyperledger_misc>".escape($project_rain->hyperledger_misc)."</user_hyperledger_misc>
   <user_waves>".escape($project_rain->waves)."</user_waves>
   <user_peershares>".escape($project_rain->peershares)."</user_peershares>
   <user_omnilayer>".escape($project_rain->omnilayer)."</user_omnilayer>
   <user_counterparty>".escape($project_rain->counterparty)."</user_counterparty>
   <user_heat_ledger>".escape($project_rain->heat_ledger)."</user_heat_ledger>
   <user_peerplays>".escape($project_rain->peerplays)."</user_peerplays>
   <user_storj>".escape($project_rain->storj)."</user_storj>
   <user_nem>".escape($project_rain->nem)."</user_nem>
   <user_ibm_bluemix_blockchain>".escape($project_rain->ibm_bluemix_blockchain)."</user_ibm_bluemix_blockchain>
   <user_coloredcoins>".escape($project_rain->coloredcoins)."</user_coloredcoins>
   <user_antshares>".escape($project_rain->antshares)."</user_antshares>
   <user_lisk>".escape($project_rain->lisk)."</user_lisk>
   <user_decent>".escape($project_rain->decent)."</user_decent>
   <user_synereo>".escape($project_rain->synereo)."</user_synereo>
   <user_lbry>".escape($project_rain->lbry)."</user_lbry>
   <user_wings>".escape($project_rain->wings)."</user_wings>
   <user_hong>".escape($project_rain->hong)."</user_hong>
   <user_boardroom>".escape($project_rain->boardroom)."</user_boardroom>
   <user_expanse>".escape($project_rain->expanse)."</user_expanse>
   <user_akasha>".escape($project_rain->akasha)."</user_akasha>
   <user_cosmos>".escape($project_rain->cosmos)."</user_cosmos>
   <user_metaverse>".escape($project_rain->metaverse)."</user_metaverse>
   <user_zcash>".escape($project_rain->zcash)."</user_zcash>
   <user_stratis>".escape($project_rain->stratis)."</user_stratis>
   <user_echo>".escape($project_rain->echo)."</user_echo>
   <user_tox>".escape($project_rain->tox)."</user_tox>
   <user_retroshare>".escape($project_rain->retroshare)."</user_retroshare>
   <user_wickr>".escape($project_rain->wickr)."</user_wickr>
   <user_ring>".escape($project_rain->ring)."</user_ring>
   <user_pgp>".escape($project_rain->pgp)."</user_pgp>    
   <user_url>".escape($user->url)."</user_url>
</team>
"
    );
}

function main() {
    echo "------------ Starting at ".time_str(time())."-------\n";
    $f = fopen("temp.xml", "w");
    $teams = BoincTeam::enum(null);
    fwrite($f, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<teams>\n");
    foreach($teams as $team) {
        handle_team($team, $f);
    }
    fwrite($f, "</teams>\n");
    fclose($f);
    if (!rename("temp.xml", "/home/boincadm/boinc/doc/boinc_teams.xml")) {
        echo "Rename failed\n";
    }
    echo "------------ Finished at ".time_str(time())."-------\n";
}

main();

?>
