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

check_get_args(array());

if (!defined("PAYPAL_ADDRESS")) {
    error_page(tra("This project is not accepting donations."));
}

db_init();

$logged_in_user = get_logged_in_user(false);
if ($logged_in_user) {
    $user_id = $logged_in_user->id;
}

page_head(tra("%1 donations", PROJECT));

if (function_exists("donations_intro")) {
    donations_intro();
} else {
    echo "<div>".tra("This project is accepting donations via
%1.", "<a href=\"http://www.paypal.com/\">PayPal</a>")."</div>
        ".tra("To donate, fill in the amount you want to donate using the field below.
        PayPal is accepting multiple currencies
        (Canadian Dollars, Euros, Pounds Sterling, U.S. Dollars,
         Yen, Australian Dollars, New Zealand Dollars,
        Swiss Francs, Hong Kong Dollars, Singapore Dollars, Swedish Kronor,
        Danish Kroner, Polish Zloty, Norwegian Kroner,
        Hungarian Forint, Czech Koruna).
        You can use included currency converter
        to see the donation amount equivalent in different currencies
        (please note that the rates are only estimates
         and the actual amount may differ).")."
    ";
}

echo "<script src=\"currency.js\" type=\"text/javascript\"></script>\n";

$query = _mysql_query("SELECT * FROM donation_items");

echo "<form name=\"calcForm\" action=\"donate.php\" method=\"post\">\n";
start_table();

// If donation_items table is filled, a list of donation targets is shown

$first_row = true;
while ($row = _mysql_fetch_object($query)) {
    $col1 = $row->title."<br><small>".$row->description."</small>";

    $query_amount = _mysql_query("SELECT SUM(payment_amount-payment_fee) AS amount FROM donation_paypal WHERE processed = 1 AND item_number = '".$row->id."'");
    $acquired = _mysql_fetch_object($query_amount)->amount;
    $required = $row->required;

    if ($acquired > $required) {
        $acquired = $acquired - $required;
        $progress = 100;
    } else {
        $progress = round($acquired / ($required) * 100);
        $progress2 = round($acquired / ($required) * 100,1);
    }

    $col2 = "<input style=\"float: left\" type=\"radio\" name=\"item_id\" value=\"".$row->id."\"";
    if ($first_row) {
        $first_row = false;
        $col2 .= " checked=\"checked\"";
    }
    $col2 .= ">";

    if ($progress == 0) {
        $col2 .= "<div style=\"float: left; width: 300px; height: 18px; background-color: red; border: 1px solid #000000\">";
    } elseif ($progress == 100) {
        $col2 .= "<div style=\"float: left; width: 300px; height: 18px; background-color: green; border: 1px solid #000000\">";
    } else {
        $col2 .= "<div style=\"float: left; width: ".($progress*3 - 1)."px; height: 18px; background-color: green; border: 1px solid #000000;\" align=\"right\"><b>".$progress2."</b>%</div><div style=\"float: left; width: ".((100-$progress)*3 - 1)."px; height: 18px; background-color: red; border: 1px solid #000000;\">";
    }

    row2($col1, $col2);
}

$amount = "<select class=\"form-control\" name=\"from\" onchange=\"Cvalue();numberFormat();\" style=\"width: 200px;\">\n";
if (DONATION_CURRENCY == "EUR") {
    $amount .= "<option value=\"51\">Euros</option>\n";
}
elseif (DONATION_CURRENCY == "CAD") {
    $amount .= "<option value=\"30\">Canadian Dollars</option>\n";
}
elseif (DONATION_CURRENCY == "GBP") {
    $amount .= "<option value=\"24\">Pounds Sterling</option>\n";
}
elseif (DONATION_CURRENCY == "USD") {
    $amount .= "<option value=\"3\">U.S. Dollars</option>\n";
}
elseif (DONATION_CURRENCY == "JPY") {
    $amount .= "<option value=\"72\">Yen</option>\n";
}
elseif (DONATION_CURRENCY == "AUD") {
    $amount .= "<option value=\"9\">Australian Dollars</option>\n";
}
elseif (DONATION_CURRENCY == "NZD") {
    $amount .= "<option value=\"102\">New Zealand Dollars</option>\n";
}
elseif (DONATION_CURRENCY == "CHF") {
    $amount .= "<option value=\"139\">Swiss Francs</option>\n";
}
elseif (DONATION_CURRENCY == "HKD") {
    $amount .= "<option value=\"63\">Hong Kong Dollars</option>\n";
}
elseif (DONATION_CURRENCY == "SGD") {
    $amount .= "<option value=\"126\">Singapore Dollars</option>\n";
}
elseif (DONATION_CURRENCY == "SEK") {
    $amount .= "<option value=\"138\">Swedish Kronor</option>\n";
}
elseif (DONATION_CURRENCY == "DKK") {
    $amount .= "<option value=\"42\">Danish Kroner</option>\n";
}
elseif (DONATION_CURRENCY == "PLN") {
    $amount .= "<option value=\"114\">Polish Zloty</option>\n";
}
elseif (DONATION_CURRENCY == "NOK") {
    $amount .= "<option value=\"106\">Norwegian Kroner</option>\n";
}
elseif (DONATION_CURRENCY == "HUF") {
    $amount .= "<option value=\"64\">Hungarian Forint</option>\n";
}
elseif (DONATION_CURRENCY == "CZK") {
    $amount .= "<option value=\"41\">Czech Koruna</option>\n";
}
$amount .= "</select>\n";
$amount .= "<input type=\"hidden\" name=\"currency\" value=\"".DONATION_CURRENCY."\">\n";
$amount .= "<input type=\"text\" name=\"inV\" id=\"inV\" value=\"10.00\" style=\"text-align: right; width: 80px\" onchange=\"Cvalue();numberFormat();\">\n";

$estimated = "<select class=\"form-control\" name=\"to\" onchange=\"Cvalue();numberFormat();\" style=\"width: 200px;\">";
$estimated .= "<option value=0 selected>Select currency</option>\n";
$estimated .= "<option value=1>Afghanistan Afghani</option>\n";
$estimated .= "<option value=2>Algerian Dinar</option>\n";
$estimated .= "<option value=3>American Dollar</option>\n";
$estimated .= "<option value=4>Angolan New Kwanza</option>\n";
$estimated .= "<option value=5>Antilles Guilder</option>\n";
$estimated .= "<option value=6>Argentine Peso</option>\n";
$estimated .= "<option value=7>Armenia Dram</option>\n";
$estimated .= "<option value=8>Aruba Guilder</option>\n";
$estimated .= "<option value=9>Australian Dollar</option>\n";
$estimated .= "<option value=10>Azerbaijan Manat</option>\n";
$estimated .= "<option value=11>BCEAO Franc</option>\n";
$estimated .= "<option value=12>BEAC Franc</option>\n";
$estimated .= "<option value=13>Bahamanian Dollar</option>\n";
$estimated .= "<option value=14>Bahraini Dinar</option>\n";
$estimated .= "<option value=15>Barbados Dollar</option>\n";
$estimated .= "<option value=16>Belarus Ruble</option>\n";
$estimated .= "<option value=17>Belize Dollar</option>\n";
$estimated .= "<option value=18>Bermuda Dollar</option>\n";
$estimated .= "<option value=19>Bhutan Ngultrum</option>\n";
$estimated .= "<option value=20>Bolivian Boliviano</option>\n";
$estimated .= "<option value=21>Bosnia Marka</option>\n";
$estimated .= "<option value=22>Botswana Pula</option>\n";
$estimated .= "<option value=23>Brazilian Real</option>\n";
$estimated .= "<option value=24>British Pound</option>\n";
$estimated .= "<option value=25>Brunei Dollar</option>\n";
$estimated .= "<option value=26>Bulgarian Lev</option>\n";
$estimated .= "<option value=27>Burundi Franc</option>\n";
$estimated .= "<option value=28>CFP Franc</option>\n";
$estimated .= "<option value=29>Cambodian Riel</option>\n";
$estimated .= "<option value=30>Canadian Dollar</option>\n";
$estimated .= "<option value=31>Cayman Dollar</option>\n";
$estimated .= "<option value=32>Chilean Peso</option>\n";
$estimated .= "<option value=33>Chinese Yuan</option>\n";
$estimated .= "<option value=34>Colombian Peso</option>\n";
$estimated .= "<option value=35>Comoros Franc</option>\n";
$estimated .= "<option value=36>Congolese Franc</option>\n";
$estimated .= "<option value=37>Costa Rican Colon</option>\n";
$estimated .= "<option value=38>Croatian Kuna</option>\n";
$estimated .= "<option value=39>Cuban Peso</option>\n";
$estimated .= "<option value=40>Cyprus Pound</option>\n";
$estimated .= "<option value=41>Czech Koruna</option>\n";
$estimated .= "<option value=42>Danish Krone</option>\n";
$estimated .= "<option value=43>Djibouti Franc</option>\n";
$estimated .= "<option value=44>Dominican R. Peso</option>\n";
$estimated .= "<option value=45>East Caribbean Dollar</option>\n";
$estimated .= "<option value=46>Egyptian Pound</option>\n";
$estimated .= "<option value=47>El Salvador Colon</option>\n";
$estimated .= "<option value=48>Eritrea Nakfa</option>\n";
$estimated .= "<option value=49>Estonian Kroon</option>\n";
$estimated .= "<option value=50>Ethiopian Birr</option>\n";
$estimated .= "<option value=51>Euro</option>\n";
$estimated .= "<option value=52>Falkland Pound</option>\n";
$estimated .= "<option value=53>Fiji Dollar</option>\n";
$estimated .= "<option value=54>Gambia Dalasi</option>\n";
$estimated .= "<option value=55>Georgia Lari</option>\n";
$estimated .= "<option value=56>Ghanaian Cedi</option>\n";
$estimated .= "<option value=57>Gibraltar Pound</option>\n";
$estimated .= "<option value=58>Guatemalan Quetzal</option>\n";
$estimated .= "<option value=59>Guinea Franc</option>\n";
$estimated .= "<option value=60>Guyanese Dollar</option>\n";
$estimated .= "<option value=61>Haitian Gourde</option>\n";
$estimated .= "<option value=62>Honduran Lempira</option>\n";
$estimated .= "<option value=63>Hong Kong Dollar</option>\n";
$estimated .= "<option value=64>Hungarian Forint</option>\n";
$estimated .= "<option value=65>IRR Rial</option>\n";
$estimated .= "<option value=66>Iceland Krona</option>\n";
$estimated .= "<option value=67>Indian Rupee</option>\n";
$estimated .= "<option value=68>Indonesian Rupiah</option>\n";
$estimated .= "<option value=69>Iraqi Dinar</option>\n";
$estimated .= "<option value=70>Israeli New Sheqel</option>\n";
$estimated .= "<option value=71>Jamaican Dollar</option>\n";
$estimated .= "<option value=72>Japanese Yen</option>\n";
$estimated .= "<option value=73>Jordanian Dinar</option>\n";
$estimated .= "<option value=74>Kazakhstan Tenge</option>\n";
$estimated .= "<option value=75>Kenyan Shilling</option>\n";
$estimated .= "<option value=76>Krygyzstan Som</option>\n";
$estimated .= "<option value=77>Kuwaiti Dinar</option>\n";
$estimated .= "<option value=78>Lao Kip</option>\n";
$estimated .= "<option value=79>Latvian Lat</option>\n";
$estimated .= "<option value=80>Lebanese Pound</option>\n";
$estimated .= "<option value=81>Lesotho Maloti</option>\n";
$estimated .= "<option value=82>Liberian Dollar</option>\n";
$estimated .= "<option value=83>Libyan Dinar</option>\n";
$estimated .= "<option value=84>Lithuanian Litas</option>\n";
$estimated .= "<option value=85>Macao Patacas</option>\n";
$estimated .= "<option value=86>Macedonian Denar</option>\n";
$estimated .= "<option value=87>Madagascar Ariary</option>\n";
$estimated .= "<option value=88>Malawi Kwacha</option>\n";
$estimated .= "<option value=89>Malaysian Ringgit</option>\n";
$estimated .= "<option value=90>Maldive Rufiyaa</option>\n";
$estimated .= "<option value=91>Maltese Lira</option>\n";
$estimated .= "<option value=92>Mauritanian Ouguiya</option>\n";
$estimated .= "<option value=93>Mauritian Rupee</option>\n";
$estimated .= "<option value=94>Mexican Peso</option>\n";
$estimated .= "<option value=95>Moldova Leu</option>\n";
$estimated .= "<option value=96>Mongolian Tugrik</option>\n";
$estimated .= "<option value=97>Moroccan Dirham</option>\n";
$estimated .= "<option value=98>Mozambique Metical</option>\n";
$estimated .= "<option value=99>Myanmar Kyat</option>\n";
$estimated .= "<option value=100>Namibia Dollar</option>\n";
$estimated .= "<option value=101>Nepalese Rupee</option>\n";
$estimated .= "<option value=102>New Zealand Dollar</option>\n";
$estimated .= "<option value=103>Nicaraguan Cordoba Oro</option>\n";
$estimated .= "<option value=104>Nigerian Naira</option>\n";
$estimated .= "<option value=105>North Korean Won</option>\n";
$estimated .= "<option value=106>Norwegian Kroner</option>\n";
$estimated .= "<option value=107>Omani Rial</option>\n";
$estimated .= "<option value=108>Pakistan Rupee</option>\n";
$estimated .= "<option value=109>Panamanian Balboa</option>\n";
$estimated .= "<option value=110>Papua New Guinea Kina</option>\n";
$estimated .= "<option value=111>Paraguay Guarani</option>\n";
$estimated .= "<option value=112>Peruvian Nuevo Sol</option>\n";
$estimated .= "<option value=113>Philippine Peso</option>\n";
$estimated .= "<option value=114>Polish Zloty</option>\n";
$estimated .= "<option value=115>Portuguese Escudo</option>\n";
$estimated .= "<option value=116>Qatari Rial</option>\n";
$estimated .= "<option value=117>Romanian Leu</option>\n";
$estimated .= "<option value=118>Russian Ruble</option>\n";
$estimated .= "<option value=119>Rwandan Franc</option>\n";
$estimated .= "<option value=120>Saint Helena Pound</option>\n";
$estimated .= "<option value=121>Sao Tome/Principe Dobra</option>\n";
$estimated .= "<option value=122>Saudi Riyal</option>\n";
$estimated .= "<option value=123>Serbia Dinar</option>\n";
$estimated .= "<option value=124>Seychelles Rupee</option>\n";
$estimated .= "<option value=125>Sierra Leone Leone</option>\n";
$estimated .= "<option value=126>Singapore Dollar</option>\n";
$estimated .= "<option value=127>Slovak Koruna</option>\n";
$estimated .= "<option value=128>Slovenian Tolar</option>\n";
$estimated .= "<option value=129>Solomon Dollar</option>\n";
$estimated .= "<option value=130>Somalia Shiling</option>\n";
$estimated .= "<option value=131>Somoa Tala</option>\n";
$estimated .= "<option value=132>South African Rand</option>\n";
$estimated .= "<option value=133>South Korean Won</option>\n";
$estimated .= "<option value=134>Sri Lanka Rupee</option>\n";
$estimated .= "<option value=135>Sudanese Dinar</option>\n";
$estimated .= "<option value=136>Suriname Dollar</option>\n";
$estimated .= "<option value=137>Swaziland Emalangeni</option>\n";
$estimated .= "<option value=138>Swedish Krona</option>\n";
$estimated .= "<option value=139>Swiss Franc</option>\n";
$estimated .= "<option value=140>Syrian Pound</option>\n";
$estimated .= "<option value=141>Taiwan Dollar</option>\n";
$estimated .= "<option value=142>Tanzania Shiling</option>\n";
$estimated .= "<option value=143>Thai Baht</option>\n";
$estimated .= "<option value=144>Tonga Pa\anga</option>\n";
$estimated .= "<option value=145>Trinidad Dollar</option>\n";
$estimated .= "<option value=146>Tunisia Dinar</option>\n";
$estimated .= "<option value=147>Turkish Lira</option>\n";
$estimated .= "<option value=148>Turkmenistan Manat</option>\n";
$estimated .= "<option value=149>Uganda Shilling</option>\n";
$estimated .= "<option value=150>Ukrainian Hryvnia</option>\n";
$estimated .= "<option value=151>Uruguayan Peso</option>\n";
$estimated .= "<option value=152>Utd. Arab Emir. Dirham</option>\n";
$estimated .= "<option value=153>Uzbekistan Sum</option>\n";
$estimated .= "<option value=154>Vanuatu Vatu</option>\n";
$estimated .= "<option value=155>Venezuelan Bolivar</option>\n";
$estimated .= "<option value=156>Vietnamese Dong</option>\n";
$estimated .= "<option value=157>Yemen Rial</option>\n";
$estimated .= "<option value=158>Zambia Kwacha</option>\n";
$estimated .= "<option value=159>Zimbabwe Dollar</option>\n";
$estimated .= "</select>\n";
$estimated .= "<input name=\"outV\" value=\"0.00\" style=\"text-align: right; width: 80px;\" disabled=\"disabled\">\n";

echo row2(tra("Amount you would like to donate"), $amount);
echo row2(tra("Estimated value in"), $estimated);
if ($user_id) {
    $tmp_user_name = $user_id." (".$logged_in_user->name.")";
    echo row2(tra("Anonymous donation")."<br><small>".tra("Select this if you dont want your name and account number displayed in
donator lists.<br>If not checked, you will be recorded as user ID %1", $tmp_user_name)."</small>",
        "<input type=\"checkbox\" name=\"anonymous\" id=\"anonymous\" value=\"1\"><label for=\"anonymous\">".tra("Yes")."</label>");
} else {
    echo row2(tra("Anonymous donation")."<br><small>".tra("To assign the donation with your user ID, please log in.")."</small>",
        "<input type=\"checkbox\" name=\"anonymous\" id=\"anonymous\" value=\"1\" checked=\"checked\" disabled=\"disabled\">
        <label for=\"anonymous\">".tra("Yes")."</label>");
}
echo row2("", "<input class=\"btn btn-primary\" type=\"submit\" value=\"".tra("Proceed")."\">");
echo row1(tra("Donations are accepted through")."<br><img src=\"img/paypal_logo.png\" alt=\"PayPal\">");
end_table();
echo "</form>\n";

page_tail();

?>
