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

require_once("../inc/util.inc");
require_once("../inc/countries.inc");

check_get_args(array("tnow", "ttok"));

$user = get_logged_in_user();
check_tokens($user->authenticator);
$project_rain = get_project_rain_details();

page_head(tra("Edit project rain information"));

echo "<form method=post action=edit_project_rain_action.php>";
echo form_tokens($user->authenticator);
start_table();
row1(tra("Cryptocurrency addresses/accounts"));
row2(tra("<img src=\"img/bitshares.png\" alt=\"BTS\" /> Bitshares %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=bitshares type=text size=20 value='$project_rain->bitshares'>"
);
row2(tra("<img src=\"img/gridcoin.png\" alt=\"GRC\" /> Gridcoin %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=gridcoin type=text size=20 value='$project_rain->gridcoin'>"
);
row2(tra("<img src=\"img/ethereum.png\" alt=\"ETH\" /> Ethereum %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=ethereum type=text size=20 value='$project_rain->ethereum'>"
);
row2(tra("<img src=\"img/ethereum-classic.png\" alt=\"ETC\" /> Ethereum Classic %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=ethereum_classic type=text size=20 value='$project_rain->ethereum_classic'>"
);
row2(tra("<img src=\"img/golem.png\" alt=\"Gol\" /> Golem %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=golem type=text size=20 value='$project_rain->golem'>"
);
row2(tra("<img src=\"img/nxt.png\" alt=\"NXT\" /> NXT %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=nxt type=text size=20 value='$project_rain->nxt'>"
);
row2(tra("<img src=\"img/ardor.png\" alt=\"Ardor\" /> Ardor %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=ardor type=text size=20 value='$project_rain->ardor'>"
);
row2(tra("<img src=\"img/hyperledger.png\" alt=\"HLSL\" /> Hyperledger Sawtooth Lake %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=hyperledger_sawtooth_lake type=text size=20 value='$project_rain->hyperledger_sawtooth_lake'>"
);
row2(tra("<img src=\"img/hyperledger.png\" alt=\"HLF\" /> Hyperledger Fabric %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=hyperledger_fabric type=text size=20 value='$project_rain->hyperledger_fabric'>"
);
row2(tra("<img src=\"img/hyperledger.png\" alt=\"HLM\" /> Hyperledger MISC %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=hyperledger_misc type=text size=20 value='$project_rain->hyperledger_misc'>"
);
row2(tra("<img src=\"img/waves.png\" alt=\"Waves\" /> Waves %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=waves type=text size=20 value='$project_rain->waves'>"
);
row2(tra("<img src=\"img/peercoin.png\" alt=\"PPC\" /> Peershares %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=peershares type=text size=20 value='$project_rain->peershares'>"
);
row2(tra("<img src=\"img/omni.png\" alt=\"OMNI\" /> Omnilayer %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=omnilayer type=text size=20 value='$project_rain->omnilayer'>"
);
row2(tra("<img src=\"img/counterparty.png\" alt=\"CP\" /> CounterParty %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=counterparty type=text size=20 value='$project_rain->counterparty'>"
);
row2(tra("<img src=\"img/heat_ledger.png\" alt=\"Heat\" /> Heat Ledger %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=heat_ledger type=text size=20 value='$project_rain->heat_ledger'>"
);
row2(tra("<img src=\"img/peerplays.png\" alt=\"ppl\" /> Peerplays %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=peerplays type=text size=20 value='$project_rain->peerplays'>"
);
row2(tra("<img src=\"img/storjcoin-x.png\" alt=\"STRJ\" /> Storj %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=storj type=text size=20 value='$project_rain->storj'>"
);
row2(tra("<img src=\"img/nem.png\" alt=\"NEM\" /> NEM %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=nem type=text size=20 value='$project_rain->nem'>"
);
row2(tra("<img src=\"img/IBMBM.png\" alt=\"\" /> IBM Bluemix blockchain %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=ibm_bluemix_blockchain type=text size=20 value='$project_rain->ibm_bluemix_blockchain'>"
);
row2(tra("<img src=\"img/coloredcoins.png\" alt=\"CC\" /> Coloredcoins %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=coloredcoins type=text size=20 value='$project_rain->coloredcoins'>"
);
row2(tra("<img src=\"img/antshares.png\" alt=\"ANT\" /> Antshares %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=antshares type=text size=20 value='$project_rain->antshares'>"
);
row2(tra("<img src=\"img/lisk.png\" alt=\"LSK\" /> Lisk %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=lisk type=text size=20 value='$project_rain->lisk'>"
);
row2(tra("<img src=\"img/expanse.png\" alt=\"EXP\" /> Expanse %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=expanse type=text size=20 value='$project_rain->expanse'>"
);
row2(tra("Cosmos %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=cosmos type=text size=20 value='$project_rain->cosmos'>"
);
row2(tra("<img src=\"img/metaverse.png\" alt=\"Meta\" /> Metaverse %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=metaverse type=text size=20 value='$project_rain->metaverse'>"
);
row2(tra("<img src=\"img/zcash.png\" alt=\"ZC\" /> Zcash %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=zcash type=text size=20 value='$project_rain->zcash'>"
);
row2(tra("<img src=\"img/stratis.png\" alt=\"STRA\" /> Stratis %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=stratis type=text size=20 value='$project_rain->stratis'>"
);
row1(tra("Crypto Media"));
row2(tra("<img src=\"img/akasha.png\" alt=\"aka\" /> Akasha %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=akasha type=text size=20 value='$project_rain->akasha'>"
);
row2(tra("<img src=\"img/decent.png\" alt=\"DC\" /> Decent %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=decent type=text size=20 value='$project_rain->decent'>"
);
row2(tra("<img src=\"img/synereo.png\" alt=\"SYN\" /> Synereo %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=synereo type=text size=20 value='$project_rain->synereo'>"
);
row2(tra("<img src=\"img/lbry.png\" alt=\"LBRY\" /> LBRY %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=lbry type=text size=20 value='$project_rain->lbry'>"
);
row2(tra("<img src=\"img/steem.png\" alt=\"STEEM\" /> Steem %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=steem type=text size=20 value='$project_rain->steem'>"
);
row1(tra("DAC/DAO platforms"));
row2(tra("<img src=\"img/wings.png\" alt=\"Wings\" /> Wings %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=wings type=text size=20 value='$project_rain->wings'>"
);
row2(tra("<img src=\"img/hong.png\" alt=\"hong\" /> Hong %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=hong type=text size=20 value='$project_rain->hong'>"
);
row2(tra("<img src=\"img/boardroom.png\" alt=\"\" /> Boardroom %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=boardroom type=text size=20 value='$project_rain->boardroom'>"
);
row1(tra("Secure messaging"));
row2(tra("<img src=\"img/echo.png\" alt=\"Echo\" /> Echo %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=echo type=text size=20 value='$project_rain->echo'>"
);
row2(tra("<img src=\"img/tox.png\" alt=\"TOX\" /> TOX %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=tox type=text size=20 value='$project_rain->tox'>"
);
row2(tra("<img src=\"img/retroshare.png\" alt=\"RS\" /> Retroshare %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=retroshare type=text size=20 value='$project_rain->retroshare'>"
);
row2(tra("<img src=\"img/wickr.png\" alt=\"Wickr\" /> Wickr %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=wickr type=text size=20 value='$project_rain->wickr'>"
);
row2(tra("<img src=\"img/ring.png\" alt=\"Ring\" /> Ring %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=ring type=text size=20 value='$project_rain->ring'>"
);
row2(tra("<img src=\"img/pgp.png\" alt=\"PGP\" /> PGP %1 Optional%2", "<br><p class=\"text-muted\">", "</p>"),
    "<input name=pgp type=text size=20 value='$project_rain->pgp'>"
);
row2("", "<input class=\"btn btn-default\" type=submit value='".tra("Update info")."'>");
end_table();
echo "</form>\n";
page_tail();

?>
