<?php

require_once("../inc/util.inc");
require_once("../inc/cert.inc");

db_init();
$user = get_logged_in_user();

$join = date('j F Y', $user->create_time);
$today = date('j F Y', time(0));

credit_to_ops($user, $ops, $unit);

$border=$_GET["border"];
if ($border=="no") {
    $border = 0;
} else {
    $border=8;
}

$credit = credit_string($user, false);

echo "
<table width=900 height=650 border=$border cellpadding=20><tr><td>
<center>
<table width=700 border=0><tr><td>
<center>
<font style=\"font-size: 52\" face=\"Old English Text MT,ZapfChancery\">Certificate of Computation


<font face=\"Lucida Bright,Times New Roman\" style=\"font-size:24\">
<br><br><br>
This certifies that
<p>
<font face=\"Lucida Bright,Times New Roman\" style=\"font-size:32\">
$user->name

<font face=\"Lucida Bright,Times New Roman\" style=\"font-size:16\">
<p>
has participated in PROJECT_NAME since $join,
and has contributed $credit
to PROJECT_NAME.

<br><br><br>
</td><tr></table>
<table width=100%><tr>
<td width=40><br></td>
<td align=left>
<font face=\"Lucida Bright,Times New Roman\" style=\"font-size:16\">
<img src=images/SIGNATURE>
<br>
PROJECT DIRECTOR NAME
<br>Director, PROJECT
<br><br>
$today
</td>
<td align=center valign=center> <img src=PROJECT_LOGO> <td>
<td align=center width=30% valign=center><img src=images/INSTITUTION_LOGO></td>

</td><tr></table>
";
?>
