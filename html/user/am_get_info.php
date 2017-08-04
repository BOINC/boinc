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

BoincDb::get(true);
xml_header();

$retval = db_init_xml();
if ($retval) xml_error($retval);

check_get_args(array("account_key"));
$auth = get_str("account_key");

$user = BoincUser::lookup_auth($auth);
if (!$user) {
    xml_error(ERR_DB_NOT_FOUND);
}

// init the project_rain var for the following lines
$project_rain = BoincRain::lookup_auth($auth);

$name = urlencode($user->name);
$country = urlencode($user->country);
$postal_code = urlencode($user->postal_code);
$bitshares = urlencode($project_rain->bitshares);
$steem = urlencode($project_rain->steem);
$gridcoin = urlencode($project_rain->gridcoin);
$ethereum = urlencode($project_rain->ethereum);
$ethereum_classic = urlencode($project_rain->ethereum_classic);
$golem = urlencode($project_rain->golem);
$nxt = urlencode($project_rain->nxt);
$ardor = urlencode($project_rain->ardor);
$hyperledger_sawtooth_lake = urlencode($project_rain->hyperledger_sawtooth_lake);
$hyperledger_fabric = urlencode($project_rain->hyperledger_fabric);
$hyperledger_misc = urlencode($project_rain->hyperledger_misc);
$waves = urlencode($project_rain->waves);
$peershares = urlencode($project_rain->peershares);
$omnilayer = urlencode($project_rain->omnilayer);
$counterparty = urlencode($project_rain->counterparty);
$heat_ledger = urlencode($project_rain->heat_ledger);
$peerplays = urlencode($project_rain->peerplays);
$storj = urlencode($project_rain->storj);
$nem = urlencode($project_rain->nem);
$ibm_bluemix_blockchain = urlencode($project_rain->ibm_bluemix_blockchain);
$coloredcoins = urlencode($project_rain->coloredcoins);
$antshares = urlencode($project_rain->antshares);
$lisk = urlencode($project_rain->lisk);
$decent = urlencode($project_rain->decent);
$synereo = urlencode($project_rain->synereo);
$lbry = urlencode($project_rain->lbry);
$wings = urlencode($project_rain->wings);
$hong = urlencode($project_rain->hong);
$boardroom = urlencode($project_rain->boardroom);
$expanse = urlencode($project_rain->expanse);
$akasha = urlencode($project_rain->akasha);
$cosmos = urlencode($project_rain->cosmos);
$metaverse = urlencode($project_rain->metaverse);
$zcash = urlencode($project_rain->zcash);
$stratis = urlencode($project_rain->stratis);
$echo = urlencode($project_rain->echo);
$tox = urlencode($project_rain->tox);
$retroshare = urlencode($project_rain->retroshare);
$wickr = urlencode($project_rain->wickr);
$ring = urlencode($project_rain->ring);
$pgp = urlencode($project_rain->pgp);
$url = urlencode($user->url);
$weak_auth = weak_auth($user);
$cpid = md5($user->cross_project_id.$user->email_addr);

$ret = "<id>$user->id</id>
<name>$name</name>
<country>$country</country>
<weak_auth>$weak_auth</weak_auth>
<postal_code>$postal_code</postal_code>
    <bitshares>$bitshares</bitshares>
    <steem>$steem</steem>
    <gridcoin>$gridcoin</gridcoin>
    <ethereum>$ethereum</ethereum>
    <ethereum_classic>$ethereum_classic</ethereum_classic>    
    <golem>$golem</golem>
    <nxt>$nxt</nxt>
    <ardor>$ardor</ardor>
    <hyperledger_sawtooth_lake>$hyperledger_sawtooth_lake</hyperledger_sawtooth_lake>
    <hyperledger_fabric>$hyperledger_fabric</hyperledger_fabric>
    <hyperledger_misc>$hyperledger_misc</hyperledger_misc>
    <waves>$waves</waves>
    <peershares>$peershares</peershares>
    <omnilayer>$omnilayer</omnilayer>
    <counterparty>$counterparty</counterparty>
    <heat_ledger>$heat_ledger</heat_ledger>
    <peerplays>$peerplays</peerplays>
    <storj>$storj</storj>
    <nem>$nem</nem>
    <ibm_bluemix_blockchain>$ibm_bluemix_blockchain</ibm_bluemix_blockchain>
    <coloredcoins>$coloredcoins</coloredcoins>
    <antshares>$antshares</antshares>
    <lisk>$lisk</lisk>
    <decent>$decent</decent>
    <synereo>$synereo</synereo>
    <lbry>$lbry</lbry>
    <wings>$wings</wings>
    <hong>$hong</hong>
    <boardroom>$boardroom</boardroom>
    <expanse>$expanse</expanse>
    <akasha>$akasha</akasha>
    <cosmos>$cosmos</cosmos>
    <metaverse>$metaverse</metaverse>
    <zcash>$zcash</zcash>
    <stratis>$stratis</stratis>
    <echo>$echo</echo>
    <tox>$tox</tox>
    <retroshare>$retroshare</retroshare>
    <wickr>$wickr</wickr>
    <ring>$ring</ring>
    <pgp>$pgp</pgp>
<cpid>$cpid</cpid>
<has_profile>$user->has_profile</has_profile>
<create_time>$user->create_time</create_time>
<global_prefs>
$user->global_prefs
</global_prefs>
<project_prefs>
$user->project_prefs
</project_prefs>
<url>$url</url>
<send_email>$user->send_email</send_email>
<show_hosts>$user->show_hosts</show_hosts>
<teamid>$user->teamid</teamid>
<venue>$user->venue</venue>";

if ($user->teamid) {
    $team = BoincTeam::lookup_id_nocache($user->teamid);
    if ($team->userid == $user->id) {
        $ret = $ret . "<teamfounder/>\n";
    }
}

echo "<am_get_info_reply>
    <success/>
    $ret
</am_get_info_reply>
";

?>
