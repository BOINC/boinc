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

require_once("../inc/credit.inc");
require_once("../inc/email.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/friend.inc");
require_once("../inc/forum_db.inc");
require_once("../inc/notify.inc");
require_once("../inc/ldap.inc");

if (!defined('REMOTE_PROJECTS_TTL')) {
    define('REMOTE_PROJECTS_TTL', 86400);
}

function get_other_projects($user) {
    $cpid = md5($user->cross_project_id . $user->email_addr);
    $url = "http://boinc.netsoft-online.com/get_user.php?cpid=".$cpid;

    // Check the cache for that URL
    //
    $cacheddata = get_cached_data(REMOTE_PROJECTS_TTL, $url);
    if ($cacheddata){
        $remote = unserialize($cacheddata);
    } else {
        $xml_object = null;
        $remote = false;
        // Fetch the XML, use curl if fopen() is disallowed
        //
        if (ini_get('allow_url_fopen')) {
            $timeout = 3;
            $old_timeout = ini_set('default_socket_timeout', $timeout);
            $xml_object = null;
            $f = @file_get_contents($url);
            if ($f) {
                $xml_object = @simplexml_load_string($f);
            }
            ini_set('default_socket_timeout', $old_timeout);
            if (!$xml_object) {
                return $user;
            }
        } else {
            $ch = curl_init($url);
            curl_setopt($ch, CURLOPT_HEADER, false);
            curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
            curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
            curl_setopt($ch, CURLOPT_MAXREDIRS, 3);
            curl_setopt($ch, CURLOPT_TIMEOUT, 3);
            $rawxml = @curl_exec($ch);
            $xml_object = null;
            if ($rawxml) {
                $xml_object = @simplexml_load_string($rawxml);
            }
            curl_close($ch);
            if (!xml_object) {
                return $user;
            }
        }

        // auto-cast the project list to an array of stdClass projects
        //
        $remote = @json_decode(json_encode((array)$xml_object))->project;
        
        if (!$remote) {
            return $user;
        } else {
            // Cache the results
            set_cached_data(REMOTE_PROJECTS_TTL, serialize($remote), $url);
        }
    }
    
    $user->projects = $remote;
    return $user;
}

function show_project($project) {
    if ($project->url == "http://www.worldcommunitygrid.org/") {
        $x = $project->name;
    } else {
        $x = "<a href=\"$project->url"."show_user.php?userid=$project->id\">$project->name</a>";
    }
    echo "<tr>
        <td>$x</td>
        <td align=\"right\">".number_format($project->total_credit, 0)."</td>
        <td align=\"right\">".number_format($project->expavg_credit, 0)."</td>
        <td align=\"right\">".date_str($project->create_time)."</td>
        </tr>
    ";
}

function cmp($a, $b) {
    if ($a->expavg_credit == $b->expavg_credit) return 0;
    return ($a->expavg_credit < $b->expavg_credit)? 1 : -1;
}

function show_other_projects($user, $personal) {
    if (!isset($user->projects)) return;
    if (count($user->projects) < 2) return;

    usort($user->projects, "cmp");
    if ($personal) {
        echo "<h3>".tra("Projects in which you are participating")."</h3>";
    } else {
        echo "<h3>".tra("Projects in which %1 is participating", $user->name)."</h3>";
    }
    start_table('table-striped');
    row_heading_array(
        array(
            tra("Project")."<br/><small>".tra("Click for user page")."</small>",
            tra("Total credit"),
            tra("Average credit"),
            tra("Since")
        ),
        array("", ALIGN_RIGHT, ALIGN_RIGHT, ALIGN_RIGHT)
    );
    foreach($user->projects as $project) {
        show_project($project);
    }
    end_table();
}

function total_posts($user) {
    return BoincPost::count("user=$user->id");
}

function show_credit($user) {
    row2(tra("Total credit"), format_credit_large($user->total_credit));
    row2(tra("Recent average credit"), format_credit($user->expavg_credit));
    if (function_exists("project_user_credit")) {
        project_user_credit($user);
    }
}

require_once("../inc/stats_sites.inc");
// show dynamic user info (private)
//
function show_user_stats_private($user) {
    global $cpid_stats_sites;
    row1(tra("Computing and credit"));
    show_credit($user);
    $config = get_config();
    row2(tra("Computers on this account"), "<a href=\"hosts_user.php\">".tra("View")."</a>");
    row2(tra("Tasks"), "<a href=\"results.php?userid=$user->id\">".tra("View")."</a>");
    $cpid = md5($user->cross_project_id . $user->email_addr);
    $x = "";
    shuffle($cpid_stats_sites);
    foreach ($cpid_stats_sites as $site) {
        $name = $site[0];
        $y = sprintf($site[1], $cpid);
        $x .= "<a href=\"$y\">$name</a><br/>\n";
    }
    $x .= "<br/><small>".tra("Cross-project ID").": $cpid</small>\n";
    row2(tra("Cross-project statistics"), $x);
    $x = '<a href="cert1.php">'.tra("Account").'</a>';
    if ($user->teamid) {
        $x .= ' &middot; <a href="cert_team.php">'.tra("Team").'</a>';
    }
    $x .= ' &middot; <a href="cert_all.php">'.tra("Cross-project").'</a>';
    row2(tra("Certificate"), $x);
    row2(tra("Stats on your cell phone"), url_base()."userw.php?id=$user->id");
}

function notify_description($notify) {
    switch ($notify->type) {
    case NOTIFY_FRIEND_REQ:
        return friend_notify_req_web_line($notify);
    case NOTIFY_FRIEND_ACCEPT:
        return friend_notify_accept_web_line($notify);
    case NOTIFY_PM:
        return pm_web_line($notify);
    case NOTIFY_SUBSCRIBED_POST:
        return subscribed_post_web_line($notify);
    }
    return null;
}

function weak_auth($user) {
    $x = md5($user->authenticator.$user->passwd_hash);
    return "{$user->id}_$x";
}

// show static user info (private)
//
function show_user_info_private($user) {
    row1(tra("Account information"), 2, 'heading');
    row2(tra("Name"), $user->name);
    if (LDAP_HOST && is_ldap_email($user->email_addr)) {
        row2("LDAP ID", ldap_email_to_uid($user->email_addr));
    } else {
        $email_text = $user->email_addr;
        if (defined("SHOW_NONVALIDATED_EMAIL_ADDR") && !$user->email_validated) {
            $email_text .= " (<a href=validate_email_addr.php>must be validated</a>)";
        }
        row2(tra("Email address"), $email_text);
    }
    if (strlen($user->url)) {
        $x = "<a href=http://$user->url>http://$user->url</a>";
        row2(tra("URL"), $x);
    }
    row2(tra("Country"), $user->country);
    row2(tra("Postal code"), $user->postal_code);
    row2(tra("%1 member since", PROJECT), date_str($user->create_time));
    $url_tokens = url_tokens($user->authenticator);
    if (LDAP_HOST && is_ldap_email($user->email_addr)) {
        // LDAP accounts can't change email or password
        //
        row2(tra("Change"),
            "<a href=\"edit_user_info_form.php?$url_tokens\">Account info</a>"
        );
    } else {
        row2(tra("Change"),
            "<a href=\"edit_email_form.php\">".tra("email address")."</a>
            &middot; <a href=\"".secure_url_base()."/edit_passwd_form.php\">".tra("password")."</a>
            &middot; <a href=\"edit_user_info_form.php?$url_tokens\">".tra("other account info")."</a>"
        );
    }
    row2(tra("User ID")."<br/><p class=\"small\">".tra("Used in community functions")."</p>", $user->id);
    if (!no_computing()) {
        row2(
            tra("Account keys"),
            "<a href=\"weak_auth.php\">".tra("View")."</a>"
        );
    }

    row1("<a name=\"prefs\"></a>".tra("Preferences"));
    if (!no_computing()) {
        row2(
            tra("When and how BOINC uses your computer"),
            "<a href=\"prefs.php?subset=global\">".tra("Computing preferences")."</a>"
        );
    }
    row2(tra("Message boards and private messages"),
        "<a href=\"edit_forum_preferences_form.php\">".tra("Community preferences")."</a>"
    );
    if (!no_computing()) {
        row2(tra("Preferences for this project"),
            "<a href=\"prefs.php?subset=project\">".tra("%1 preferences", PROJECT)."</a>"
        );
    }
}

function show_project_rain_crypto_private($project_rain) {
    
    if (($project_rain->gridcoin) || ($project_rain->bitshares) || ($project_rain->peerplays) || ($project_rain->nxt) || ($project_rain->ardor) || ($project_rain->golem) || ($project_rain->ethereum) || ($project_rain->ethereum_classic) || ($project_rain->expanse) || ($project_rain->ethereum_classic) || ($project_rain->expanse) || ($project_rain->storj) || ($project_rain->nem) || ($project_rain->hyperledger_sawtooth_lake) || ($project_rain->hyperledger_fabric) || ($project_rain->ibm_bluemix_blockchain) || ($project_rain->hyperledger_misc) || ($project_rain->waves) || ($project_rain->peershares) || ($project_rain->omnilayer) || ($project_rain->counterparty) || ($project_rain->coloredcoins) || ($project_rain->antshares) || ($project_rain->lisk) || ($project_rain->heat_ledger) || ($project_rain->metaverse) || ($project_rain->cosmos) || ($project_rain->stratis) || ($project_rain->zcash)) {
        row1(tra("Cryptocurrency addresses/accounts"));
        if ($project_rain->gridcoin) {
            row2(tra("<img src=\"img/gridcoin.png\" alt=\"GRC\" /> <a href='https://gridcoin.us/'>Gridcoin</a>"), $project_rain->gridcoin);
        }
        if ($project_rain->bitshares) {
            row2(tra("<img src=\"img/bitshares.png\" alt=\"BTS\" /> <a href='https://bitshares.org'>Bitshares</a>"), $project_rain->bitshares);
        }
        if ($project_rain->peerplays) {
            row2(tra("<img src=\"img/peerplays.png\" alt=\"ppl\" /> <a href='http://www.peerplays.com/'>Peerplays</a>"), $project_rain->peerplays);
        }
        if ($project_rain->nxt) {
            row2(tra("<img src=\"img/nxt.png\" alt=\"NXT\" /> <a href='https://nxt.org/'>NXT</a>"), $project_rain->nxt);
        }
        if ($project_rain->ardor) {
            row2(tra("<img src=\"img/ardor.png\" alt=\"Ardor\" /> <a href='https://ardorplatform.org/'>Ardor</a>"), $project_rain->ardor);
        }
        if ($project_rain->golem) {
            row2(tra("<img src=\"img/golem.png\" alt=\"Gol\" /> <a href='http://golemproject.net/'>Golem</a>"), $project_rain->golem);
        }
        if ($project_rain->ethereum) {
            row2(tra("<img src=\"img/ethereum.png\" alt=\"ETH\" /> <a href='https://www.ethereum.org/'>Ethereum</a>"), $project_rain->ethereum);
        }
        if ($project_rain->ethereum_classic) {
            row2(tra("<img src=\"img/ethereum-classic.png\" alt=\"ETC\" /> <a href='https://ethereumclassic.github.io/'>Ethereum Classic</a>"), $project_rain->ethereum_classic);
        }
        if ($project_rain->expanse) {
            row2(tra("<img src=\"img/expanse.png\" alt=\"EXP\" /> <a href='http://www.expanse.tech/'>Expanse</a>"), $project_rain->expanse);
        }
        if ($project_rain->storj) {
            row2(tra("<img src=\"img/storjcoin-x.png\" alt=\"STRJ\" /> <a href='https://storj.io/'>Storj</a>"), $project_rain->storj);
        }
        if ($project_rain->nem) {
            row2(tra("<img src=\"img/nem.png\" alt=\"NEM\" /> <a href='http://nem.io/'>NEM</a>"), $project_rain->nem);
        }
        if ($project_rain->hyperledger_sawtooth_lake) {
            row2(tra("<img src=\"img/hyperledger.png\" alt=\"HLSL\" /> <a href='https://github.com/intelledger'>Hyperledger Sawtooth Lake</a>"), $project_rain->hyperledger_sawtooth_lake);
        }
        if ($project_rain->hyperledger_fabric) {
            row2(tra("<img src=\"img/hyperledger.png\" alt=\"HLSL\" /> <a href='https://github.com/hyperledger/fabric'>Hyperledger Fabric</a>"), $project_rain->hyperledger_fabric);
        }
        if ($project_rain->ibm_bluemix_blockchain) {
            row2(tra("<img src=\"img/IBMBM.png\" alt=\"\" /> <a href='https://www.ibm.com/blockchain/bluemix.html'>IBM Bluemix Blockchain</a>"), $project_rain->ibm_bluemix_blockchain);
        }
        if ($project_rain->hyperledger_misc) {
            row2(tra("<img src=\"img/hyperledger.png\" alt=\"HLSL\" /> <a href='https://www.hyperledger.org/'>Hylerledger (MISC)</a>"), $project_rain->hyperledger_misc);
        }
        if ($project_rain->waves) {
            row2(tra("<img src=\"img/waves.png\" alt=\"Waves\" /> <a href='https://wavesplatform.com/'>Waves</a>"), $project_rain->waves);
        }
        if ($project_rain->peershares) {
            row2(tra("<img src=\"img/peercoin.png\" alt=\"PPC\" /> <a href='http://peershares.net/'>Peershares</a>"), $project_rain->peershares);
        }
        if ($project_rain->omnilayer) {
            row2(tra("<img src=\"img/omni.png\" alt=\"OMNI\" /> <a href='http://www.omnilayer.org/'>Omnilayer</a>"), $project_rain->omnilayer);
        }
        if ($project_rain->counterparty) {
            row2(tra("<img src=\"img/counterparty.png\" alt=\"CP\" /> <a href='http://counterparty.io/'>Counterparty</a>"), $project_rain->counterparty);
        }
        if ($project_rain->coloredcoins) {
            row2(tra("<img src=\"img/coloredcoins.png\" alt=\"CC\" /> <a href='http://coloredcoins.org/'>ColoredCoins</a>"), $project_rain->coloredcoins);
        }
        if ($project_rain->antshares) {
            row2(tra("<img src=\"img/antshares.png\" alt=\"ANT\" /> <a href='https://www.antshares.org/'>Antshares</a>"), $project_rain->antshares);
        }
        if ($project_rain->lisk) {
            row2(tra("<img src=\"img/lisk.png\" alt=\"LSK\" /> <a href='https://lisk.io/'>Lisk</a>"), $project_rain->lisk);
        }
        if ($project_rain->heat_ledger) {
            row2(tra("<img src=\"img/heat_ledger.png\" alt=\"Heat\" /> <a href='http://heatledger.com/'>Heat Ledger</a>"), $project_rain->heat_ledger);
        }
        if ($project_rain->metaverse) {
            row2(tra("<img src=\"img/metaverse.png\" alt=\"Meta\" /> <a href='http://viewfin.com/'>Metaverse</a>"), $project_rain->metaverse);
        }
        if ($project_rain->cosmos) {
            row2(tra("<a href='http://cosmos.network/'>Cosmos</a>"), $project_rain->cosmos);
        }
        if ($project_rain->stratis) {
            row2(tra("<img src=\"img/stratis.png\" alt=\"STRA\" /> <a href='http://stratisplatform.com/'>Stratis</a>"), $project_rain->stratis);
        }
        if ($project_rain->zcash) {
            row2(tra("<img src=\"img/zcash.png\" alt=\"ZC\" /> <a href='https://z.cash/'>Zcash</a>"), $project_rain->zcash);
        }
    }

    $url_tokens = url_tokens($project_rain->authenticator);
    row2(tra("Change"),
        "<a href=\"edit_project_rain_form.php?$url_tokens\">".tra("Edit your cryptocurrency addresses/accounts")."</a>"
    );
}

function show_project_rain_crypto_public($project_rain) {
    if (($project_rain->gridcoin) || ($project_rain->bitshares) || ($project_rain->peerplays) || ($project_rain->nxt) || ($project_rain->ardor) || ($project_rain->golem) || ($project_rain->ethereum) || ($project_rain->ethereum_classic) || ($project_rain->expanse) || ($project_rain->ethereum_classic) || ($project_rain->expanse) || ($project_rain->storj) || ($project_rain->nem) || ($project_rain->hyperledger_sawtooth_lake) || ($project_rain->hyperledger_fabric) || ($project_rain->ibm_bluemix_blockchain) || ($project_rain->hyperledger_misc) || ($project_rain->waves) || ($project_rain->peershares) || ($project_rain->omnilayer) || ($project_rain->counterparty) || ($project_rain->coloredcoins) || ($project_rain->antshares) || ($project_rain->lisk) || ($project_rain->heat_ledger) || ($project_rain->metaverse) || ($project_rain->cosmos) || ($project_rain->stratis) || ($project_rain->zcash)) {
        row1(tra("Cryptocurrency addresses/accounts"));
        if ($project_rain->gridcoin) {
            row2(tra("<img src=\"img/gridcoin.png\" alt=\"GRC\" /> <a href='https://gridcoin.us/'>Gridcoin</a>"), $project_rain->gridcoin);
        }
        if ($project_rain->bitshares) {
            row2(tra("<img src=\"img/bitshares.png\" alt=\"BTS\" /> <a href='https://bitshares.org'>Bitshares</a>"), $project_rain->bitshares);
        }
        if ($project_rain->peerplays) {
            row2(tra("<img src=\"img/peerplays.png\" alt=\"ppl\" /> <a href='http://www.peerplays.com/'>Peerplays</a>"), $project_rain->peerplays);
        }
        if ($project_rain->nxt) {
            row2(tra("<img src=\"img/nxt.png\" alt=\"NXT\" /> <a href='https://nxt.org/'>NXT</a>"), $project_rain->nxt);
        }
        if ($project_rain->ardor) {
            row2(tra("<img src=\"img/ardor.png\" alt=\"Ardor\" /> <a href='https://ardorplatform.org/'>Ardor</a>"), $project_rain->ardor);
        }
        if ($project_rain->golem) {
            row2(tra("<img src=\"img/golem.png\" alt=\"Gol\" /> <a href='http://golemproject.net/'>Golem</a>"), $project_rain->golem);
        }
        if ($project_rain->ethereum) {
            row2(tra("<img src=\"img/ethereum.png\" alt=\"ETH\" /> <a href='https://www.ethereum.org/'>Ethereum</a>"), $project_rain->ethereum);
        }
        if ($project_rain->ethereum_classic) {
            row2(tra("<img src=\"img/ethereum-classic.png\" alt=\"ETC\" /> <a href='https://ethereumclassic.github.io/'>Ethereum Classic</a>"), $project_rain->ethereum_classic);
        }
        if ($project_rain->expanse) {
            row2(tra("<img src=\"img/expanse.png\" alt=\"EXP\" /> <a href='http://www.expanse.tech/'>Expanse</a>"), $project_rain->expanse);
        }
        if ($project_rain->storj) {
            row2(tra("<img src=\"img/storjcoin-x.png\" alt=\"STRJ\" /> <a href='https://storj.io/'>Storj</a>"), $project_rain->storj);
        }
        if ($project_rain->nem) {
            row2(tra("<img src=\"img/nem.png\" alt=\"NEM\" /> <a href='http://nem.io/'>NEM</a>"), $project_rain->nem);
        }
        if ($project_rain->hyperledger_sawtooth_lake) {
            row2(tra("<img src=\"img/hyperledger.png\" alt=\"HLSL\" /> <a href='https://github.com/intelledger'>Hyperledger Sawtooth Lake</a>"), $project_rain->hyperledger_sawtooth_lake);
        }
        if ($project_rain->hyperledger_fabric) {
            row2(tra("<img src=\"img/hyperledger.png\" alt=\"HLSL\" /> <a href='https://github.com/hyperledger/fabric'>Hyperledger Fabric</a>"), $project_rain->hyperledger_fabric);
        }
        if ($project_rain->ibm_bluemix_blockchain) {
            row2(tra("<img src=\"img/IBMBM.png\" alt=\"\" /> <a href='https://www.ibm.com/blockchain/bluemix.html'>IBM Bluemix Blockchain</a>"), $project_rain->ibm_bluemix_blockchain);
        }
        if ($project_rain->hyperledger_misc) {
            row2(tra("<img src=\"img/hyperledger.png\" alt=\"HLSL\" /> <a href='https://www.hyperledger.org/'>Hylerledger (MISC)</a>"), $project_rain->hyperledger_misc);
        }
        if ($project_rain->waves) {
            row2(tra("<img src=\"img/waves.png\" alt=\"Waves\" /> <a href='https://wavesplatform.com/'>Waves</a>"), $project_rain->waves);
        }
        if ($project_rain->peershares) {
            row2(tra("<img src=\"img/peercoin.png\" alt=\"PPC\" /> <a href='http://peershares.net/'>Peershares</a>"), $project_rain->peershares);
        }
        if ($project_rain->omnilayer) {
            row2(tra("<img src=\"img/omni.png\" alt=\"OMNI\" /> <a href='http://www.omnilayer.org/'>Omnilayer</a>"), $project_rain->omnilayer);
        }
        if ($project_rain->counterparty) {
            row2(tra("<img src=\"img/counterparty.png\" alt=\"CP\" /> <a href='http://counterparty.io/'>Counterparty</a>"), $project_rain->counterparty);
        }
        if ($project_rain->coloredcoins) {
            row2(tra("<img src=\"img/coloredcoins.png\" alt=\"CC\" /> <a href='http://coloredcoins.org/'>ColoredCoins</a>"), $project_rain->coloredcoins);
        }
        if ($project_rain->antshares) {
            row2(tra("<img src=\"img/antshares.png\" alt=\"ANT\" /> <a href='https://www.antshares.org/'>Antshares</a>"), $project_rain->antshares);
        }
        if ($project_rain->lisk) {
            row2(tra("<img src=\"img/lisk.png\" alt=\"LSK\" /> <a href='https://lisk.io/'>Lisk</a>"), $project_rain->lisk);
        }
        if ($project_rain->heat_ledger) {
            row2(tra("<img src=\"img/heat_ledger.png\" alt=\"Heat\" /> <a href='http://heatledger.com/'>Heat Ledger</a>"), $project_rain->heat_ledger);
        }
        if ($project_rain->metaverse) {
            row2(tra("<img src=\"img/metaverse.png\" alt=\"Meta\" /> <a href='http://viewfin.com/'>Metaverse</a>"), $project_rain->metaverse);
        }
        if ($project_rain->cosmos) {
            row2(tra("<a href='http://cosmos.network/'>Cosmos</a>"), $project_rain->cosmos);
        }
        if ($project_rain->stratis) {
            row2(tra("<img src=\"img/stratis.png\" alt=\"STRA\" /> <a href='http://stratisplatform.com/'>Stratis</a>"), $project_rain->stratis);
        }
        if ($project_rain->zcash) {
            row2(tra("<img src=\"img/zcash.png\" alt=\"ZC\" /> <a href='https://z.cash/'>Zcash</a>"), $project_rain->zcash);
        }
    }
}

function show_project_rain_dao_message_private($project_rain) {
    if (($project_rain->wings) || ($project_rain->hong) || ($project_rain->boardroom)) {
        row1(tra("DAC/DAO platforms"));
        if ($project_rain->wings) {
            row2(tra("<img src=\"img/wings.png\" alt=\"Wings\" /> <a href='https://wings.ai/'>Wings</a>"), $project_rain->wings);
        }
        if ($project_rain->hong) {
            row2(tra("<img src=\"img/hong.png\" alt=\"hong\" /> <a href='http://www.hongcoin.org/'>Hong</a>"), $project_rain->hong);
        }
        if ($project_rain->boardroom) {
            row2(tra("<img src=\"img/boardroom.png\" alt=\"\" /> <a href='http://boardroom.to/'>BoardRoom</a>"), $project_rain->boardroom);
        }
    }

    if (($project_rain->steem) || ($project_rain->decent) || ($project_rain->synereo) || ($project_rain->akasha) || ($project_rain->lbry)) {
        row1(tra("Crypto media"));
        if ($project_rain->steem) {
            row2(tra("<img src=\"img/steem.png\" alt=\"STEEM\" /> <a href='https://steemit.com/?r=cm-steem'>Steem</a>"), $project_rain->steem);
        }
        if ($project_rain->decent) {
            row2(tra("<img src=\"img/decent.png\" alt=\"DC\" /> <a href='http://decent.ch/'>Decent</a>"), $project_rain->decent);
        }
        if ($project_rain->synereo) {
            row2(tra("<img src=\"img/synereo.png\" alt=\"SYN\" /> <a href='https://www.synereo.com/'>Synereo</a>"), $project_rain->synereo);
        }
        if ($project_rain->akasha) {
            row2(tra("<img src=\"img/akasha.png\" alt=\"aka\" /> <a href='http://akasha.world/'>Akasha</a>"), $project_rain->akasha);
        }
        if ($project_rain->lbry) {
            row2(tra("<img src=\"img/lbry.png\" alt=\"LBRY\" /> <a href='https://lbry.io/'>LBRY</a>"), $project_rain->lbry);
        }
    }

    if (($project_rain->echo) || ($project_rain->tox) || ($project_rain->retroshare) || ($project_rain->wickr) || ($project_rain->ring) || ($project_rain->pgp)) {
        row1(tra("Secure messaging"));
        if ($project_rain->echo) {
            row2(tra("<img src=\"img/echo.png\" alt=\"Echo\" /> <a href='https://my-echo.com/'>Echo</a>"), $project_rain->echo);
        }
        if ($project_rain->tox) {
            row2(tra("<img src=\"img/tox.png\" alt=\"TOX\" /> <a href='https://tox.chat/'>TOX</a>"), $project_rain->tox);
        }
        if ($project_rain->retroshare) {
            row2(tra("<img src=\"img/retroshare.png\" alt=\"RS\" /> <a href='https://retroshare.github.io/index.html'>RetroShare</a>"), $project_rain->retroshare);
        }
        if ($project_rain->wickr) {
            row2(tra("<img src=\"img/wickr.png\" alt=\"Wickr\" /> <a href='https://www.wickr.com/'>Wickr</a>"), $project_rain->wickr);
        }
        if ($project_rain->ring) {
            row2(tra("<img src=\"img/ring.png\" alt=\"Ring\" /> <a href='https://ring.cx/'>Ring</a>"), $project_rain->ring);
        }
        if ($project_rain->pgp) {
            pgpfield(tra("<img src=\"img/pgp.png\" alt=\"PGP\" /> <a href='https://ssd.eff.org/en/module/introduction-public-key-cryptography-and-pgp'>PGP</a>"), $project_rain->pgp);
        }
    }


    $url_tokens = url_tokens($project_rain->authenticator);
    row2(tra("Change"),
        "<a href=\"edit_project_rain_form.php?$url_tokens\">".tra("Edit your cryptocurrency addresses/accounts")."</a>"
    );
}

function show_project_rain_dao_message_public($project_rain) {
    if (($project_rain->wings) || ($project_rain->hong) || ($project_rain->boardroom)) {
        row1(tra("DAC/DAO platforms"));
        if ($project_rain->wings) {
            row2(tra("<img src=\"img/wings.png\" alt=\"Wings\" /> <a href='https://wings.ai/'>Wings</a>"), $project_rain->wings);
        }
        if ($project_rain->hong) {
            row2(tra("<img src=\"img/hong.png\" alt=\"hong\" /> <a href='http://www.hongcoin.org/'>Hong</a>"), $project_rain->hong);
        }
        if ($project_rain->boardroom) {
            row2(tra("<img src=\"img/boardroom.png\" alt=\"\" /> <a href='http://boardroom.to/'>BoardRoom</a>"), $project_rain->boardroom);
        }
    }

    if (($project_rain->steem) || ($project_rain->decent) || ($project_rain->synereo) || ($project_rain->akasha) || ($project_rain->lbry)) {
        row1(tra("Crypto media"));
        if ($project_rain->steem) {
            row2(tra("<img src=\"img/steem.png\" alt=\"STEEM\" /> <a href='https://steemit.com/?r=cm-steem'>Steem</a>"), $project_rain->steem);
        }
        if ($project_rain->decent) {
            row2(tra("<img src=\"img/decent.png\" alt=\"DC\" /> <a href='http://decent.ch/'>Decent</a>"), $project_rain->decent);
        }
        if ($project_rain->synereo) {
            row2(tra("<img src=\"img/synereo.png\" alt=\"SYN\" /> <a href='https://www.synereo.com/'>Synereo</a>"), $project_rain->synereo);
        }
        if ($project_rain->akasha) {
            row2(tra("<img src=\"img/akasha.png\" alt=\"aka\" /> <a href='http://akasha.world/'>Akasha</a>"), $project_rain->akasha);
        }
        if ($project_rain->lbry) {
            row2(tra("<img src=\"img/lbry.png\" alt=\"LBRY\" /> <a href='https://lbry.io/'>LBRY</a>"), $project_rain->lbry);
        }
    }
    
    if (($project_rain->echo) || ($project_rain->tox) || ($project_rain->retroshare) || ($project_rain->wickr) || ($project_rain->ring) || ($project_rain->pgp)) {
            row1(tra("Secure messaging"));
            if ($project_rain->echo) {
                row2(tra("<img src=\"img/echo.png\" alt=\"Echo\" /> <a href='https://my-echo.com/'>Echo</a>"), $project_rain->echo);
            }
            if ($project_rain->tox) {
                row2(tra("<img src=\"img/tox.png\" alt=\"TOX\" /> <a href='https://tox.chat/'>TOX</a>"), $project_rain->tox);
            }
            if ($project_rain->retroshare) {
                row2(tra("<img src=\"img/retroshare.png\" alt=\"RS\" /> <a href='https://retroshare.github.io/index.html'>RetroShare</a>"), $project_rain->retroshare);
            }
            if ($project_rain->wickr) {
                row2(tra("<img src=\"img/wickr.png\" alt=\"Wickr\" /> <a href='https://www.wickr.com/'>Wickr</a>"), $project_rain->wickr);
            }
            if ($project_rain->ring) {
                row2(tra("<img src=\"img/ring.png\" alt=\"Ring\" /> <a href='https://ring.cx/'>Ring</a>"), $project_rain->ring);
            }
            if ($project_rain->pgp) {
                pgpfield(tra("<img src=\"img/pgp.png\" alt=\"PGP\" /> <a href='https://ssd.eff.org/en/module/introduction-public-key-cryptography-and-pgp'>PGP</a>"), $project_rain->pgp);
            }
        }
}

function friend_links($user) {
    if (is_banished($user)) {
        return "";
    }
    $x = "<table height=\"100\" width=\"150\" border=\"0\" cellpadding=\"4\"><tr><td class=\"friend\">";
    if ($user->has_profile) {
        $profile = BoincProfile::lookup_fields("has_picture", "userid=$user->id");
        if ($profile && $profile->has_picture) {
            $img_url = profile_thumb_url($user->id);
        } else {
            $img_url = url_base()."img/head_20.png";
        }
        $title = tra("View the profile of %1", $user->name);
        $alt = tra("Profile");
        $x .= ' <a href="'.url_base().'view_profile.php?userid='.$user->id.'"><img title="'.$title.'" src="'.$img_url.'" alt="'.$alt.'"></a><br>';
    }
    $x .= " <a href=\"".url_base()."show_user.php?userid=".$user->id."\">".$user->name."</a>";
    if ($user->donated == 1) {
        require_once("../project/donations.inc");
        $x .= DONATION_LINK;
    }
    if ($user->donated == 2) {
        require_once("../project/donations.inc");
        $x .= DONATION_LINK_TWO;
    }
    $x .= "</td></tr></table>\n";
    return $x;
}

// show user name, with links to profile if present.
// if $badge_height is > 0, show badges
//
function user_links($user, $badge_height=0) {
    BoincForumPrefs::lookup($user);
    if (is_banished($user)) {
        return "(banished: ID $user->id)";
    }
    $x = "";
    if ($user->has_profile) {
        $img_url = url_base()."img/head_20.png";
        $x .= ' <a href="'.url_base().'view_profile.php?userid='.$user->id.'"><img title="View the profile of '.$user->name.'" src="'.$img_url.'" alt="Profile"></a>';
    }
    $x .= " <a href=\"".url_base()."show_user.php?userid=".$user->id."\">".$user->name."</a>";
    if ($user->donated == 1) {
        require_once("../project/donations.inc");
        $x .= DONATION_LINK;
    }
    if ($user->donated == 2) {
        require_once("../project/donations.inc");
        $x .= DONATION_LINK_TWO;
    }
    if ($badge_height) {
        $x .= badges_string(true, $user, $badge_height);
    }
    if (function_exists("project_user_links")){
        $x .= project_user_links($user);
    }
    return $x;
}

function show_community_private($user) {
    start_table();
    row1(tra("Community"));

    show_badges_row(true, $user);
    if (!DISABLE_PROFILES) {
        if ($user->has_profile) {
            $x = "<a href=\"view_profile.php?userid=$user->id\">".tra("View")."</a> &middot; <a href=\"delete_profile.php\">".tra("Delete")."</a>";
        } else {
            $x = "<a href=\"create_profile.php\">".tra("Create")."</a>";
        }
        row2(tra("Profile"), $x);
    }
    if (!DISABLE_FORUMS) {
        $tot = total_posts($user);
        if ($tot) {
            row2(tra("Message boards"), "<a href=\"".url_base()."forum_user_posts.php?userid=$user->id\">".tra("%1 posts", $tot)."</a>");
        }
    }
    
    row2(tra("Private messages"), pm_notification($user).pm_email_remind($user));

    $notifies = BoincNotify::enum("userid=$user->id");
    if (count($notifies)) {
        $x = "";
        foreach ($notifies as $notify) {
            $y = notify_description($notify);
            if ($y) {
                $x .= "&bull; $y<br>";
            } else {
                $notify->delete();
            }
        }
        $x .= "<a href=\"".notify_rss_url($user)."\"><img vspace=\"4\" border=\"0\" src=\"img/rss_icon.gif\" alt=\"RSS\" /></a>";
        row2(tra("Notifications"), $x);
    }

    if (!DISABLE_TEAMS) {
        if ($user->teamid && ($team = BoincTeam::lookup_id($user->teamid))) {
            $x = "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>
                &middot; <a href=\"team_quit_form.php\">".tra("Quit team")."</a>";
            if (is_team_admin($user, $team)) {
                $x .= " &middot; <a href=\"team_manage.php?teamid=$user->teamid\">".tra("Administer")."</a>";
            }

            // if there's a foundership request, notify the founder
            //
            if ($user->id==$team->userid && $team->ping_user >0) {
                $x .= "<p class=\"text-danger\">".tra("(foundership change request pending)")."</p>";
            }
            row2(tra("Member of team"), $x);
        } else {
            row2(tra("Team"), tra("None")." &middot; <a href=\"team_search.php\">".tra("find a team")."</a>");
        }
        
        $teams_founded = BoincTeam::enum("userid=$user->id");
        foreach ($teams_founded as $team) {
            if ($team->id != $user->teamid) {
                $x = "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>";
                $x .= " | <a href=\"team_manage.php?teamid=".$team->id."\">".tra("Administer")."</a>";
                if ($team->ping_user > 0) {
                    $x .= "<p class=\"text-danger\">".tra("(foundership change request pending)")."</span>";
                }
                row2(tra("Founder but not member of"), $x);
            }
        }
    }

    $friends = BoincFriend::enum("user_src=$user->id and reciprocated=1");
    $x = "<a href=\"user_search.php\">".tra("Find friends")."</a><br/>\n";
    $n = count($friends);
    if ($n) {
        foreach($friends as $friend) {
            $fuser = BoincUser::lookup_id($friend->user_dest);
            if (!$fuser) continue;
            $x .= friend_links($fuser);
        }
        row2(tra("Friends")." ($n)", $x);
    } else {
        row2(tra("Friends"), $x);
    }
    end_table();
}

// show summary of dynamic and static info (public)
//
function show_user_summary_public($user) {
    global $g_logged_in_user;
    row2(tra("User ID"), $user->id);
    row2(tra("%1 member since", PROJECT), date_str($user->create_time));
    row2(tra("Country"), $user->country);
    // don't show URL if user has no recent credit (spam suppression)
    //
    if (strlen($user->url)) {
        if (no_computing() || $user->expavg_credit > 1) {
            row2(tra("URL"), "<a href=\"http://$user->url\">http://$user->url</a>");
        }
    }
    if (!no_computing()) {
        show_credit($user);

        if ($user->show_hosts) {
            row2(tra("Computers"), "<a href=\"".url_base()."hosts_user.php?userid=$user->id\">".tra("View")."</a>");
        } else {
            row2(tra("Computers"), tra("hidden"));
        }
    }
    if ($user->donated == 1) { 
        if (file_exists("../project/donations.inc")) {
            require_once("../project/donations.inc");
            row2(tra("Donor"), DONATION_LINK);
        }
    }
    if ($user->donated == 2) {
        if (file_exists("../project/donations.inc")) {
            require_once("../project/donations.inc");
            row2(tra("Donor"), DONATION_LINK_TWO);
        }
    }
}

// Returns a cacheable community links data object
// @param user The user to produce a community links object for

function get_community_links_object($user){
    $cache_object = new StdClass;
    $cache_object->post_count = total_posts($user);
    $cache_object->user = $user;
    $cache_object->team = BoincTeam::lookup_id($user->teamid);
    $cache_object->friends = array();

    $friends = BoincFriend::enum("user_src=$user->id and reciprocated=1");
    foreach($friends as $friend) {
        $fuser = BoincUser::lookup_id($friend->user_dest);
        if (!$fuser) continue;
        $cache_object->friends[] = $fuser;
    }
    return $cache_object;
}

function community_links($clo, $logged_in_user){
    $user = $clo->user;
    $team = $clo->team;
    $friends = $clo->friends;
    $tot = $clo->post_count;
    
    if (!DISABLE_TEAMS) {
        if ($user->teamid && $team) {
            row2(tra("Team"), "<a href=\"".url_base()."team_display.php?teamid=$team->id\">$team->name</a>");
        } else {
            row2(tra("Team"), tra("None"));
        }
    }
    if (!DISABLE_FORUMS) {
        if ($tot) {
            row2(tra("Message boards"), "<a href=\"".url_base()."forum_user_posts.php?userid=$user->id\">".tra("%1 posts", $tot)."</a>");
        }
    }
    if ($logged_in_user && $logged_in_user->id != $user->id) {
        row2(tra("Contact"), "<a href=\"pm.php?action=new&userid=".$user->id."\">".tra("Send private message")."</a>");
        $friend = BoincFriend::lookup($logged_in_user->id, $user->id);
        if ($friend && $friend->reciprocated) {
            row2(tra("This person is a friend"), 
                "<a href=\"friend.php?action=cancel_confirm&userid=$user->id\">".tra("Cancel friendship")."</a>"
            );
        } else if ($friend) {
            row2(tra("Friends"),  "<a href=\"friend.php?action=add&userid=$user->id\">".tra("Request pending")."</a>");
        } else {
            row2(tra("Friends"),  "<a href=\"friend.php?action=add&userid=$user->id\">".tra("Add as friend")."</a>");
        }
    }
    
    if ($friends) {
        $x = "";
        foreach($friends as $friend) {
            $x .= friend_links($friend);
        }
        row2(tra("Friends")." (".sizeof($friends).")", $x);
    }
}

function show_profile_link($user) {
    if ($user->has_profile) {
        row2(tra("Profile"), "<a href=\"view_profile.php?userid=$user->id\">".tra("View")."</a>");
    }
}

function show_account_private($user) {
    grid(
        false,
        function() use ($user) {
            start_table();
            show_user_info_private($user);
            if (!no_computing()) {
                show_user_stats_private($user);
            }

            if (file_exists("../project/donations.inc")) {
                require_once("../project/donations.inc");
                if (function_exists('show_user_donations_private')) {
                    show_user_donations_private($user);
                }
            }
            end_table();
            show_other_projects($user, true);
            project_user_page_private($user);
            //echo "<a href=delete_account.php>Delete account</a>\n";
        },
        function() use ($user) {
            show_community_private($user);
        }
    );
}

function show_rain_private($project_rain) {
    start_table();
    echo "<tr><td valign=top>";
    start_table();
    show_project_rain_crypto_private($project_rain);
    end_table();
    echo "</td><td valign=top>";
    start_table();
    show_project_rain_dao_message_private($project_rain);
    end_table();
    echo "</td></tr></table>";
}

function show_rain_public($project_rain) {
    start_table();
    echo "<tr><td valign=top>";
    start_table();
    show_project_rain_crypto_public($project_rain);
    end_table();
    echo "</td><td valign=top>";
    start_table();
    show_project_rain_dao_message_public($project_rain);
    end_table();
    echo "</td></tr></table>";
}

function is_banned_email_addr($email_addr) {
    global $banned_email_domains;
    if (isset($banned_email_domains)) {
        foreach($banned_email_domains as $d) {
            $x = strstr($email_addr, $d);
            if ($x == $d) return true;
        }
    }
    return false;
}

function is_valid_user_name($name, &$reason) {
    if (trim($name) !== $name) {
        $reason = tra("user name cannot have leading or trailing white space");
        return false;
    }
    if (strlen($name) == 0) {
        $reason = tra("user name must be nonempty");
        return false;
    }
    if (sanitize_tags($name) !== $name) {
        $reason = tra("user name may not contain HTML tags");
        return false;
    }
    return true;
}

// the following DB-escapes its args
//
function make_user(
    $email_addr, $name, $passwd_hash,
    $country=null, $postal_code=null, $project_prefs=null, $teamid=0
) {
    if (!is_valid_email_addr($email_addr)) return null;
    if (is_banned_email_addr($email_addr)) return null;

    $authenticator = random_string();
    $cross_project_id = random_string();
    $now = time();
    if (!is_valid_country($country)) return null;

    $email_addr = BoincDb::escape_string($email_addr);
    $name = sanitize_tags($name);
    $name = BoincDb::escape_string($name);
    $passwd_hash = BoincDb::escape_string($passwd_hash);

    $country = BoincDb::escape_string($country);
    $postal_code = sanitize_tags(BoincDb::escape_string($postal_code));

    $uid = BoincUser::insert("(create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid,  venue, send_email, show_hosts, posts, seti_id, seti_nresults, seti_last_result_time, seti_total_cpu, has_profile, cross_project_id, passwd_hash, email_validated, donated) values($now, '$email_addr', '$name', '$authenticator', '$country', '$postal_code', 0, 0, unix_timestamp(), '$project_prefs', $teamid, '', 1, 1, 0, 0, 0, 0, 0, 0, '$cross_project_id', '$passwd_hash', 0, 0)");

    $rainid = BoincRain::insert("(authenticator, bitshares, steem, peerplays, storj, nem, ibm_bluemix_blockchain, coloredcoins, antshares, lisk, decent, synereo, lbry, wings, hong, boardroom, gridcoin, ethereum, ethereum_classic, expanse, golem, nxt, ardor, hyperledger_sawtooth_lake, hyperledger_fabric, hyperledger_misc, waves, akasha, cosmos, metaverse, zcash, stratis, peershares, omnilayer, counterparty, heat_ledger, echo, tox, retroshare, wickr, ring, pgp) values('$authenticator', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '', '')");

    if (!$uid) {
        return null;
    }
    $user = BoincUser::lookup_id($uid);
    if (defined('RECORD_USER_IP')) {
        $ip = $_SERVER['REMOTE_ADDR'];
        $ip = BoincDb::escape_string($ip);
        $user->update("venue='$ip'");
    }
    return $user;
}

function make_user_ldap($email_addr, $name) {
    $email_addr = BoincDb::escape_string($email_addr);
    $name = sanitize_tags($name);
    $name = BoincDb::escape_string($name);
    $authenticator = random_string();
    $cross_project_id = random_string();
    $passwd_hash = random_string();
    $now = time();
    $uid = BoincUser::insert("(create_time, email_addr, name, authenticator, country, postal_code, total_credit, expavg_credit, expavg_time, project_prefs, teamid,  send_email, show_hosts, cross_project_id, passwd_hash) values($now, '$email_addr', '$name', '$authenticator', '', '', 0, 0, unix_timestamp(), '', 0, 1, 1, '$cross_project_id', '$passwd_hash')");

    if ($uid) {
        return BoincUser::lookup_id($uid);
    } else {
        return null;
    }
}

// delete a user and all associated records except
// result
// host
// batch
// team
// user_submit
// user_submit_app
// credited_job
// donation_paypal
// sent_email
//
function delete_user($user) {
    delete_profile($user);
    forum_delete_user($user);
        // deletes post, thread, subscription, forum_preferences, forum_logging
    BoincPrivateMessage::delete_aux("userid=$user->id or senderid=$user->id");
    BoincNotify::delete_aux("userid=$user->id");
    BoincCreditUser::delete_user($user);
    BoincBadgeUser::delete("user_id=$user->id");
    BoincFriend::delete_aux("user_src=$user->id or user_dest=$user->id");
    $user->delete();
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

?>
