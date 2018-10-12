<?php

require_once("../inc/util.inc");
require_once("../inc/cert.inc");
require_once("../inc/cert_functions.inc");

db_init();
$user = get_logged_in_user();

$join = gmdate('j F Y', $user->create_time);
$today = gmdate('j F Y', time(0));

$border = get_str('border', true);
if ($border=="no") {
    $border = 0;
} else {
    $border=8;
}

$credit = sah_credit_string($user, false);

$title_font = "\"Optima,ZapfChancery\"";
$font = "\"Optima,Lucida Bright,Times New Roman\"";

echo "
<table width=900 height=650 border=$border cellpadding=20><tr><td>
<center>
<table width=700 border=0><tr><td>
<center>
<font style=\"font-size: 52\" face=$title_font>Certificate of Computation


<font face=$font style=\"font-size:24\">
<br><br><br>
This certifies that
<p>
<font face=$font style=\"font-size:32\">
$user->name

<font face=$font style=\"font-size:16\">
<p>
has participated in the SETI@home project since $join,
and has contributed $credit
to SETI@home's search for extraterrestrial life.

<br><br><br>
</td><tr></table>
<table width=100%><tr>
<td width=40><br></td>
<td align=left>
<font face=$font style=\"font-size:16\">
<img src=images/grey_sig_220.png>
<br>Dr. Eric J. Korpela
<br>Director, SETI@home
<br><br>
$today
</td>
<td align=center valign=center> <img src=logo7.gif> <td>
<td align=center width=30% valign=center><img src=images/uc_logo_150.png></td>

</td><tr></table>
";
?>
