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
require_once("../inc/xml.inc");
require_once("../inc/team.inc");
require_once("../inc/email.inc");

// do a very cursory check that the given text is valid;
// for now, just make sure it has the given start and end tags,
// and at least one \n in the middle.
// Ideally, we'd like to check that it's valid XML
//
function bad_xml($text, $start, $end) {
    $text = trim($text);
    if (strstr($text, $start) != $text) {
        return "No start tag";
    }
    if (strstr($text, $end) != $end) {
        return "No end tag";
    }
    if (!strstr($text, "\n")) {
        return "No CR";
    }
    return "";
}

function success($x) {
    echo "<am_set_info_reply>
    <success/>
    $x
</am_set_info_reply>
";
}

xml_header();
$retval = db_init_xml();
if ($retval) xml_error($retval);

$auth = post_str("account_key", true);
if ($auth) {
    $name = post_str("name", true);
    $country = post_str("country", true);
    $postal_code = post_str("postal_code", true);
    //
    $bitshares = post_str("bitshares", true);
    $steem = post_str("steem", true);
    $gridcoin = post_str("gridcoin", true);
    $ethereum = post_str("ethereum", true);
    $golem = post_str("golem", true);
    $nxt = post_str("nxt", true);
    $ardor = post_str("ardor", true);
    $hyperledger_sawtooth_lake = post_str("hyperledger_sawtooth_lake", true);
    $hyperledger_fabric = post_str("hyperledger_fabric", true);
    $hyperledger_misc = post_str("hyperledger_misc", true);
    $waves = post_str("waves", true);
    $peershares = post_str("peershares", true);
    $omnilayer = post_str("omnilayer", true);
    $counterparty = post_str("counterparty", true);
    $heat_ledger = post_str("heat_ledger", true);
    $peerplays = post_str("peerplays", true);
    $storj = post_str("storj", true);
    $nem = post_str("nem", true);
    $ibm_bluemix_blockchain = post_str("ibm_bluemix_blockchain", true);
    $coloredcoins = post_str("coloredcoins", true);
    $antshares = post_str("antshares", true);
    $lisk = post_str("lisk", true);
    $decent = post_str("decent", true);
    $synereo = post_str("synereo", true);
    $lbry = post_str("lbry", true);
    $expanse = post_str("expanse", true);
    $akasha = post_str("akasha", true);
    $cosmos = post_str("cosmos", true);
    $metaverse = post_str("metaverse", true);
    $zcash = post_str("zcash", true);
    $stratis = post_str("stratis", true);
    //
    $wings = post_str("wings", true);
    $hong = post_str("hong", true);
    $boardroom = post_str("boardroom", true);
    //
    $echo = post_str("echo", true);
    $tox = post_str("tox", true);
    $retroshare = post_str("retroshare", true);
    $wickr = post_str("wickr", true);
    $ring = post_str("ring", true);
    $pgp = post_str("pgp", true);
    //
    $global_prefs = post_str("global_prefs", true);
    $project_prefs = post_str("project_prefs", true);
    $url = post_str("url", true);
    $send_email = post_str("send_email", true);
    $show_hosts = post_str("show_hosts", true);
    $teamid = post_int("teamid", true);
    $venue = post_str("venue", true);
    $email_addr = post_str("email_addr", true);
    $password_hash = post_str("password_hash", true);
} else {
    $auth = get_str("account_key");
    $name = get_str("name", true);
    $country = get_str("country", true);
    $postal_code = get_str("postal_code", true);
    //
    $bitshares = get_str("bitshares", true);
    $steem = get_str("steem", true);
    $gridcoin = get_str("gridcoin", true);
    $ethereum = get_str("ethereum", true);
    $ethereum_classic = get_str("ethereum_classic", true);
    $golem = get_str("golem", true);
    $nxt = get_str("nxt", true);
    $ardor = get_str("ardor", true);
    $hyperledger_sawtooth_lake = get_str("hyperledger_sawtooth_lake", true);
    $hyperledger_fabric = get_str("hyperledger_fabric", true);
    $hyperledger_misc = get_str("hyperledger_misc", true);
    $waves = get_str("waves", true);
    $peershares = get_str("peershares", true);
    $omnilayer = get_str("omnilayer", true);
    $counterparty = get_str("counterparty", true);
    $heat_ledger = get_str("heat_ledger", true);
    $peerplays = get_str("peerplays", true);
    $storj = get_str("storj", true);
    $nem = get_str("nem", true);
    $ibm_bluemix_blockchain = get_str("ibm_bluemix_blockchain", true);
    $coloredcoins = get_str("coloredcoins", true);
    $antshares = get_str("antshares", true);
    $lisk = get_str("lisk", true);
    $decent = get_str("decent", true);
    $synereo = get_str("synereo", true);
    $lbry = get_str("lbry", true);
    $wings = get_str("wings", true);
    $hong = get_str("hong", true);
    $boardroom = get_str("boardroom", true);
    $expanse = get_str("expanse", true);
    $akasha = get_str("akasha", true);
    $cosmos = get_str("cosmos", true);
    $metaverse = get_str("metaverse", true);
    $zcash = get_str("zcash", true);
    $stratis = get_str("stratis", true);
    //
    $echo = get_str("echo", true);
    $tox = get_str("tox", true);
    $retroshare = get_str("retroshare", true);
    $wickr = get_str("wickr", true);
    $ring = get_str("ring", true);
    $pgp = get_str("pgp", true);
    //
    $global_prefs = get_str("global_prefs", true);
    $project_prefs = get_str("project_prefs", true);
    $url = get_str("url", true);
    $send_email = get_str("send_email", true);
    $show_hosts = get_str("show_hosts", true);
    $teamid = get_int("teamid", true);
    $venue = get_str("venue", true);
    $email_addr = get_str("email_addr", true);
    $password_hash = get_str("password_hash", true);
}

$user = BoincUser::lookup_auth($auth);
if (!$user) {
    xml_error(ERR_DB_NOT_FOUND);
}

$name = BoincDb::escape_string($name);
if ($country && !is_valid_country($country)) {
    xml_error(-1, "invalid country");
}
$country = BoincDb::escape_string($country);
$postal_code = BoincDb::escape_string($postal_code);
//
$bitshares = BoincDb::escape_string($bitshares);
$steem = BoincDb::escape_string($steem);
$gridcoin = BoincDb::escape_string($gridcoin);
$ethereum = BoincDb::escape_string($ethereum);
$ethereum_classic = BoincDb::escape_string($ethereum_classic);
$golem = BoincDb::escape_string($golem);
$nxt = BoincDb::escape_string($nxt);
$ardor = BoincDb::escape_string($ardor);
$hyperledger_sawtooth_lake = BoincDb::escape_string($hyperledger_sawtooth_lake);
$hyperledger_fabric = BoincDb::escape_string($hyperledger_fabric);
$hyperledger_misc = BoincDb::escape_string($hyperledger_misc);
$waves = BoincDb::escape_string($waves);
$peershares = BoincDb::escape_string($peershares);
$omnilayer = BoincDb::escape_string($omnilayer);
$counterparty = BoincDb::escape_string($counterparty);
$heat_ledger = BoincDb::escape_string($heat_ledger);
$peerplays = BoincDb::escape_string($peerplays);
$storj = BoincDb::escape_string($storj);
$nem = BoincDb::escape_string($nem);
$ibm_bluemix_blockchain = BoincDb::escape_string($ibm_bluemix_blockchain);
$coloredcoins = BoincDb::escape_string($coloredcoins);
$antshares = BoincDb::escape_string($antshares);
$lisk = BoincDb::escape_string($lisk);
$decent = BoincDb::escape_string($decent);
$synereo = BoincDb::escape_string($synereo);
$lbry = BoincDb::escape_string($lbry);
$wings = BoincDb::escape_string($wings);
$hong = BoincDb::escape_string($hong);
$boardroom = BoincDb::escape_string($boardroom);
$expanse = BoincDb::escape_string($expanse);
$akasha = BoincDb::escape_string($akasha);
$cosmos = BoincDb::escape_string($cosmos);
$metaverse = BoincDb::escape_string($metaverse);
$zcash = BoincDb::escape_string($zcash);
$stratis = BoincDb::escape_string($stratis);
//
$echo = BoincDb::escape_string($echo);
$tox = BoincDb::escape_string($tox);
$retroshare = BoincDb::escape_string($retroshare);
$wickr = BoincDb::escape_string($wickr);
$ring = BoincDb::escape_string($ring);
$pgp = BoincDb::escape_string($pgp);
//
$global_prefs = BoincDb::escape_string($global_prefs);
$project_prefs = BoincDb::escape_string($project_prefs);

// Do processing on project prefs so that we don't overwrite project-specific
// settings if AMS has no idea about them

if (stripos($project_prefs, "<project_specific>") === false) {
    // AMS request does not contain project specific prefs, preserve original
    $orig_project_specific = stristr($user->project_prefs, "<project_specific>");
    $orig_project_specific = substr($orig_project_specific, 0, stripos($orig_project_specific, "</project_specific>") + 19)."\n";
    $project_prefs = str_ireplace("<project_preferences>", "<project_preferences>\n".$orig_project_specific, $project_prefs);
}

$url = BoincDb::escape_string($url);
$send_email = BoincDb::escape_string($send_email);
$show_hosts = BoincDb::escape_string($show_hosts);
$venue = BoincDb::escape_string($venue);
if ($email_addr) {
    if (!is_valid_email_addr($email_addr)) {
        xml_error(ERR_BAD_EMAIL_ADDR, "Invalid email address");
    }
    if (is_banned_email_addr($email_addr)) {
        xml_error(ERR_BAD_EMAIL_ADDR, "Invalid email address");
    }
    $email_addr = strtolower(BoincDb::escape_string($email_addr));
}
$password_hash = BoincDb::escape_string($password_hash);

$query = "";
if ($name) {
    $query .= " name='$name', ";
}
if ($country) {
    $query .= " country='$country', ";
}
if ($postal_code) {
    $query .= " postal_code='$postal_code', ";
}
if ($bitshares) {
    $query .= " bitshares='$bitshares', ";
}
if ($steem) {
    $query .= " steem='$steem', ";
}
if ($gridcoin) {
    $query .= " gridcoin='$gridcoin', ";
}
if ($ethereum) {
    $query .= " ethereum='$ethereum', ";
}
if ($ethereum_classic) {
    $query .= " ethereum_classic='$ethereum_classic', ";
}
if ($golem) {
    $query .= " golem='$golem', ";
}
if ($nxt) {
    $query .= " nxt='$nxt', ";
}
if ($ardor) {
    $query .= " ardor='$ardor', ";
}
if ($hyperledger_sawtooth_lake) {
    $query .= " hyperledger_sawtooth_lake='$hyperledger_sawtooth_lake', ";
}
if ($hyperledger_fabric) {
    $query .= " hyperledger_fabric='$hyperledger_fabric', ";
}
if ($hyperledger_misc) {
    $query .= " hyperledger_misc='$hyperledger_misc', ";
}
if ($waves) {
    $query .= " waves='$waves', ";
}
if ($peershares) {
    $query .= " peershares='$peershares', ";
}
if ($omnilayer) {
    $query .= " omnilayer='$omnilayer', ";
}
if ($counterparty) {
    $query .= " counterparty='$counterparty', ";
}
if ($heat_ledger) {
    $query .= " heat_ledger='$heat_ledger', ";
}
if ($peerplays) {
    $query .= " peerplays='$peerplays', ";
}
if ($storj) {
    $query .= " storj='$storj', ";
}
if ($nem) {
    $query .= " nem='$nem', ";
}
if ($ibm_bluemix_blockchain) {
    $query .= " ibm_bluemix_blockchain='$ibm_bluemix_blockchain', ";
}
if ($coloredcoins) {
    $query .= " coloredcoins='$coloredcoins', ";
}
if ($antshares) {
    $query .= " antshares='$antshares', ";
}
if ($lisk) {
    $query .= " lisk='$lisk', ";
}
if ($decent) {
    $query .= " decent='$decent', ";
}
if ($synereo) {
    $query .= " synereo='$synereo', ";
}
if ($lbry) {
    $query .= " lbry='$lbry', ";
}
if ($wings) {
    $query .= " wings='$wings', ";
}
if ($hong) {
    $query .= " hong='$hong', ";
}
if ($boardroom) {
    $query .= " boardroom='$boardroom', ";
}
if ($expanse) {
    $query .= " expanse='$expanse', ";
}
if ($akasha) {
    $query .= " akasha='$akasha', ";
}
if ($cosmos) {
    $query .= " cosmos='$cosmos', ";
}
if ($metaverse) {
    $query .= " metaverse='$metaverse', ";
}
if ($zcash) {
    $query .= " zcash='$zcash', ";
}
if ($stratis) {
    $query .= " stratis='$stratis', ";
}
if ($echo) {
    $query .= " echo='$echo', ";
}
if ($tox) {
    $query .= " tox='$tox', ";
}
if ($retroshare) {
    $query .= " retroshare='$retroshare', ";
}
if ($wickr) {
    $query .= " wickr='$wickr', ";
}
if ($ring) {
    $query .= " ring='$ring', ";
}
if ($pgp) {
    $query .= " pgp='$pgp', ";
}
if ($global_prefs) {
    $global_prefs = str_replace("\\r\\n", "\n", $global_prefs);
    $x = bad_xml($global_prefs, "<global_preferences>", "</global_preferences>");
    if ($x) {
        error("Invalid global preferences: $x");
    }
    $query .= " global_prefs='$global_prefs', ";
}
if ($project_prefs) {
    $project_prefs = str_replace("\\r\\n", "\n", $project_prefs);
    $x = bad_xml($project_prefs, "<project_preferences>", "</project_preferences>");
    if ($x) {
        xml_error(ERR_XML_PARSE, "Invalid project preferences: $x");
    }
    $query .= " project_prefs='$project_prefs', ";
}
if ($url) {
    $query .= " url='$url', ";
}
if ($send_email != null) {
    $query .= " send_email='$send_email', ";
}
if ($show_hosts != null) {
    $query .= " show_hosts='$show_hosts', ";
}

if (!is_null($teamid)) {
    if ($teamid==0) {
        user_quit_team($user);
    } else {
        $team = BoincTeam::lookup_id_nocache($teamid);
        if ($team && $team->joinable) {
            user_join_team($team, $user);
        }
    }
}

if ($venue) {
    $query .= " venue='$venue', ";
}
if ($email_addr && $email_addr!=$user->email_addr) {
    $old_email_addr = $user->email_addr;
    $query .= " email_addr='$email_addr', ";
}
if ($password_hash) {
    $query .= " passwd_hash='$password_hash', ";
}

if (strlen($query)) {
    // the seti_id=seti_id is to make the query valid,
    // since $query ends with a comma at this point
    //
    $query = "$query seti_id=seti_id";
    $result = $user->update($query);
    if ($result) {
        success("");
    } else {
        xml_error(-1, "database error: ".BoincDb::error());
    }
} else {
    success("");
}

?>
