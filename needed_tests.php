<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/phpmailer/class.phpmailer.php");
require_once("test.inc");

db_init();

function tcount($p, $v) {
    $r = mysql_query("select count(*) from test_report where version='$v' and test_group = '$p' and status<>3");
    $s = mysql_fetch_row($r);
    return $s[0];
}

$r = mysql_query("select * from test_report");
while ($tr = mysql_fetch_object($r)) {
    $tarr[$tr->test_group][$tr->version][$tr->status]++;
}

$max_versions = 1;
$message = "";
$mail = new PHPMailer();

$html = "<html><head><title>Testing status report</title></head><body>
    <p>
    This report shows the tests for which more reports are needed.
    <p>
    Please visit http://boinc.berkeley.edu/trac/wiki/TestMatrix
    to see descriptions of the various tests,
    and submit reports as soon as possible.
    <p>
    Thanks for your time and effort in making BOINC
    a great platform for volunteer computing.
    <p>
    The BOINC Development Team
    <p>
    Tests for which we need more reports:
    <table><tr><td>Test</td><td>Additional reports needed</td></tr>
";

$message = "This report shows which test areas need more test coverage.

Please visit http://boinc.berkeley.edu/trac/wiki/TestMatrix
to see descriptions of the various tests,
and submit reports as soon as possible.
Thanks for your time and effort in making BOINC
a great platform for volunteer computing.

The BOINC Development Team

Tests for which we need more reports:
";

for ($i=0; $i<count($test_groups); $i++) {
    $p = $test_groups[$i][0];
    $pl = $test_groups[$i][1];
    $tr = $test_groups[$i][2];
    for ($j=0; $j<count($versions)&&$j<$max_versions; $j++) {
        $v = $versions[$j];
        $x1 = tcount($p, $v);
        $x2 = $tr - $x1;
        if ($tr > $x1) {
          $html = $html."<tr><td>$pl</td><td>$x2</td></tr>";
          $message = $message."$pl\t$x2\n";
        }
    }
}

$html = $html."</table></body></html>";

$mail->From     = "boincadm@ssl.berkeley.edu";
$mail->FromName = "BOINC Administrator";
$mail->Host     = "mail.ssl.berkeley.edu";
$mail->Mailer   = "smtp";
$mail->Subject  = "Testing status report";
$mail->Body     = $html;
$mail->AltBody  = $message;
$mail->AddAddress("boinc_alpha@ssl.berkeley.edu", "BOINC Alpha Email List");
$mail->Send();

echo "<pre>";
echo "$message";
echo "</pre>";

?>
