<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");
require_once("../inc/email.inc");

// activate/deactivate script
if (0) {
  echo "
This script needs to be activated before it can be run.
Once you understand what the script does you can change the 
if (1) to if (0) at the top of the file to activate it.
Be sure to deactivate the script after using it to make sure
it is not accidentally run. 
";
  exit;
}

db_init();
$hostid = $_GET["hostid"];

if (!$hostid) {
    admin_page_head("Misconfigured Host");
    echo "This script sends an email to the owner of the supplied host which says that something gone wrong with his configuration.<br>";
    echo "<br><form method=\"get\" action=\"problem_host.php\">
    Host ID: 
    <input type=\"text\" size=\"5\" name=\"hostid\">
    <input type=\"submit\" value=\"Send Email\">
    </form>
    ";
} else {
    $res = mysql_query("select * from host where id='$hostid'");
    if (0) {
      echo "No host with that ID";
      exit;
    }
    $host = mysql_fetch_object($res);
    $res = mysql_query("select * from user where id='$host->userid'");
    $user = mysql_fetch_object($res);
    send_problem_email($user, $host);
    echo "Email to ".$user->email_addr." has been send.";
}

admin_page_tail();
?>
