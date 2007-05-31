<?php

require_once("../inc/util.inc");

$st = get_str("st", true);

page_head("PayPal - Transaction Completed");

if ($st == "Completed") {
    echo "<div>Thank you for donating!<br>\n";
    echo "Your donation for has been completed.<br>\n";
    echo "Your donation will be added to the progress bar after confirmation by PayPal.</div>";
} else {
    echo "<strong>You have canceled your donation</strong>";
}

page_tail();

?>
