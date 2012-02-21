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

// Redirect user to PayPal system

require_once("../inc/util.inc");

db_init();

$logged_in_user = get_logged_in_user(false);

$amount = post_str("inV");
$item_id = post_int("item_id", true);
if ($item_id == null) { $item_id = 1; }
$currency = post_str("currency");
if ((post_int("anonymous", true) == 1) || ($logged_in_user == null)) {
    $userid = 0;
} else {
    $userid = $logged_in_user->id;
}

$order_time = time();

// Write user id to paypal table, so the return script knows it's expecting this payment
mysql_query("INSERT INTO donation_paypal SET order_time = '".$order_time."', userid = '$userid', item_number=".$item_id.", order_amount = '".boinc_real_escape_string($amount)."'");

$payment_id = mysql_insert_id();

$URL = "www.paypal.com/cgi-bin/webscr";

$fields = ("cmd=_xclick&lc=US&business=".PAYPAL_ADDRESS."&quantity=1&item_name=Donation&item_number=".$payment_id."_".$order_time."&amount=".$amount."&no_shipping=1&return=".URL_BASE."donated.php?st=Completed&rm=2&cancel_return=".URL_BASE."/donated.php&no_note=1&currency_code=".$currency."&bn=PP-BuyNowBF");

header("Location: https://$URL?$fields");

exit;

?>
