<?php

require_once("../inc/util.inc");
require_once("../inc/cert.inc");

$user = get_logged_in_user();

$join = date('j F Y', $user->create_time);
$today = date('j F Y', time(0));

$border=$_GET["border"];
if ($border=="no") {
    $border = 0;
} else {
    $border=8;
}

$credit = credit_string($user->total_credit, false);

$title_font = "\"Optima,ZapfChancery\"";
$font = "\"Optima,Lucida Bright,Times New Roman\"";

echo "
    <table width=900 height=650 border=$border cellpadding=20><tr><td>
    <center>
    <table width=700 border=0><tr><td>
    <center>
    <font style=\"font-size: 52\" face=$title_font>Certificate of Computation


    <font face=$font style=\"font-size:28\">
    <br><br><br>
    This certifies that
    <p>
    <font face=$font style=\"font-size:32\">
    $user->name

    <font face=$font style=\"font-size:18\">
    <p>
    has participated in ".PROJECT." since $join,
    and has contributed $credit
    to ".PROJECT.".

    <br><br><br>
    </td><tr></table>
    <table width=100%><tr>
    <td width=40><br></td>
    <td align=left>
    <font face=$font style=\"font-size:16\">
";
if (defined("CERT_SIGNATURE")) {
    echo "
        <img src=".CERT_SIGNATURE.">
        <br>
    ";
}
if (defined("CERT_DIRECTOR_NAME")) {
    echo CERT_DIRECTOR_NAME." <br>Director, ".PROJECT."
        <br>
    ";
}
echo "
    <br>
    $today
    </td>
";
if (defined("CERT_PROJECT_LOGO")) {
    echo "
        <td align=center valign=center> <img src=".CERT_PROJECT_LOGO."> </td>
    ";
}
if (defined("CERT_INSTITUTION_LOGO")) {
    echo "
        <td align=center width=30% valign=center><img src=".CERT_INSTITUTION_LOGO."></td>
    ";
}
echo "
</td><tr></table>
";
?>
