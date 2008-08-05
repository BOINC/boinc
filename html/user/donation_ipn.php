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
db_init();

// read the post from PayPal system and add 'cmd'
$req = 'cmd=_notify-validate';

foreach ($_POST as $key => $value) {
    $value = urlencode(undo_magic_quotes($value));
    $req .= "&$key=$value";
}

// post back to PayPal system to validate
$header .= "POST /cgi-bin/webscr HTTP/1.0\r\n";
$header .= "Content-Type: application/x-www-form-urlencoded\r\n";
$header .= "Content-Length: " . strlen($req) . "\r\n\r\n";
$fp = fsockopen ('www.paypal.com', 80, $errno, $errstr, 30);

// assign posted variables to local variables
$item_name = $_POST['item_name'];
$item_number = $_POST['item_number'];
$payment_status = $_POST['payment_status'];
$payment_amount = $_POST['mc_gross'];
$payment_fee = $_POST['mc_fee'];
$payment_currency = $_POST['mc_currency'];
$txn_id = $_POST['txn_id'];
$receiver_email = $_POST['receiver_email'];
$payer_email = $_POST['payer_email'];
$payer_name = $_POST['first_name']." ".$_POST['last_name'];
$ip = $_SERVER['REMOTE_ADDR'];
$agent = strtolower($_SERVER[HTTP_USER_AGENT]);

if (!$fp) {
    // HTTP ERROR, might want to do additional handling here
} else {
    fputs ($fp, $header . $req);
    while (!feof($fp)) {
        $res = fgets ($fp, 1024);
        if (strcmp ($res, "VERIFIED") == 0) {
            $item_array = explode("_",$item_number);
            $payment_id = abs($item_array[0]);
            $order_time = abs($item_array[1]);
            $result = mysql_query("SELECT * FROM donation_paypal WHERE order_time = '$order_time' AND id = '$payment_id' AND processed = '0'");
            $num_rows = mysql_num_rows($result);
            if ($num_rows == 1) {
                $row = mysql_fetch_object($result);
                $userid = $row->userid;
                mysql_query("UPDATE donation_paypal SET processed = '1', payment_time = '".time()."', item_name = '$item_name', payment_status = '$payment_status', payment_amount = '$payment_amount', payment_fee = '$payment_fee', payment_currency = '$payment_currency', txn_id = '$txn_id', receiver_email = '$receiver_email', payer_email = '$payer_email', payer_name = '$payer_name' WHERE id = '$payment_id'");
                if ($userid > 0) {
                    mysql_query("UPDATE user SET donated = '1' WHERE id = '$userid'");
                }
            }
        }
    }
    fclose ($fp);
}

?>
