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
require_once("../inc/user.inc");
require_once("../inc/util.inc");
require_once("../inc/countries.inc");

check_get_args(array("tnow", "ttok"));

$user = get_logged_in_user();
check_tokens($user->authenticator);
$project_rain = get_project_rain_details();

$bitshares = sanitize_tags(post_str("bitshares", true));
$steem = sanitize_tags(post_str("steem", true));
$peerplays = sanitize_tags(post_str("peerplays", true));
$gridcoin = strip_non_alphanumeric(post_str("gridcoin", true));
$ethereum = strip_non_alphanumeric(post_str("ethereum", true));
$ethereum_classic = strip_non_alphanumeric(post_str("ethereum_classic", true));
$golem = strip_non_alphanumeric(post_str("golem", true));
$nxt = strip_non_alphanumeric(post_str("nxt", true));
$ardor = strip_non_alphanumeric(post_str("ardor", true));
$hyperledger_sawtooth_lake = strip_non_alphanumeric(post_str("hyperledger_sawtooth_lake", true));
$hyperledger_fabric = strip_non_alphanumeric(post_str("hyperledger_fabric", true));
$hyperledger_misc = strip_non_alphanumeric(post_str("hyperledger_misc", true));
$waves = strip_non_alphanumeric(post_str("waves", true));
$peershares = strip_non_alphanumeric(post_str("peershares", true));
$omnilayer = strip_non_alphanumeric(post_str("omnilayer", true));
$counterparty = strip_non_alphanumeric(post_str("counterparty", true));
$heat_ledger = strip_non_alphanumeric(post_str("heat_ledger", true));
$storj = strip_non_alphanumeric(post_str("storj", true));
$nem = strip_non_alphanumeric(post_str("nem", true));
$ibm_bluemix_blockchain = strip_non_alphanumeric(post_str("ibm_bluemix_blockchain", true));
$coloredcoins = strip_non_alphanumeric(post_str("coloredcoins", true));
$antshares = strip_non_alphanumeric(post_str("antshares", true));
$lisk = strip_non_alphanumeric(post_str("lisk", true));
$decent = strip_non_alphanumeric(post_str("decent", true));
$synereo = strip_non_alphanumeric(post_str("synereo", true));
$lbry = strip_non_alphanumeric(post_str("lbry", true));
$wings = strip_non_alphanumeric(post_str("wings", true));
$hong = strip_non_alphanumeric(post_str("hong", true));
$boardroom = strip_non_alphanumeric(post_str("boardroom", true));
$expanse = strip_non_alphanumeric(post_str("expanse", true));
$akasha = strip_non_alphanumeric(post_str("akasha", true));
$cosmos = strip_non_alphanumeric(post_str("cosmos", true));
$metaverse = strip_non_alphanumeric(post_str("metaverse", true));
$zcash = strip_non_alphanumeric(post_str("zcash", true));
$stratis = strip_non_alphanumeric(post_str("stratis", true));
$echo = sanitize_tags(post_str("echo", true));
$tox = sanitize_tags(post_str("tox", true));
$retroshare = sanitize_tags(post_str("retroshare", true));
$wickr = sanitize_tags(post_str("wickr", true));
$ring = sanitize_tags(post_str("ring", true));
$pgp = sanitize_tags(post_str("pgp", true));

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
$echo = BoincDb::escape_string($echo);
$tox = BoincDb::escape_string($tox);
$retroshare = BoincDb::escape_string($retroshare);
$wickr = BoincDb::escape_string($wickr);
$ring = BoincDb::escape_string($ring);
$pgp = BoincDb::escape_string($pgp);

$result = $project_rain->update(
   "bitshares='$bitshares',
    steem='$steem',
    gridcoin='$gridcoin',
    ethereum='$ethereum',
    ethereum_classic='$ethereum_classic',
    golem='$golem',
    nxt='$nxt',
    ardor='$ardor',
    hyperledger_sawtooth_lake='$hyperledger_sawtooth_lake',
    hyperledger_fabric='$hyperledger_fabric',
    hyperledger_misc='$hyperledger_misc',
    waves='$waves',
    peershares='$peershares',
    omnilayer='$omnilayer',
    counterparty='$counterparty',
    heat_ledger='$heat_ledger',
    peerplays='$peerplays',
    storj='$storj',
    nem='$nem',
    ibm_bluemix_blockchain='$ibm_bluemix_blockchain',
    coloredcoins='$coloredcoins',
    antshares='$antshares',
    lisk='$lisk',
    decent='$decent',
    synereo='$synereo',
    lbry='$lbry',
    wings='$wings',
    hong='$hong',
    boardroom='$boardroom',
    expanse='$expanse',
    akasha='$akasha',
    cosmos='$cosmos',
    metaverse='$metaverse',
    zcash='$zcash',
    stratis='$stratis',
    echo='$echo',
    tox='$tox',
    retroshare='$retroshare',
    wickr='$wickr',
    ring='$ring',
   pgp='$pgp' "
);
if ($result) {
    Header("Location: home.php");
} else {
    error_page(tra("Couldn't update 'project Rain' info."));
}

?>
