<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

// Show results with pending credit for a user

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");

$config = get_config();
if (!parse_bool($config, "show_results")) {
    error_page("This feature is turned off temporarily");
}

$format = get_str("format", true);

if ($format == "xml") {
    xml_header();
    
    $user = get_user_from_auth(get_str("authenticator"));
    if (!$user) {
        echo "<error>".xml_error(-136)."</error>\n";
        exit();
    }
    $sum = 0;
    echo "<pending_credit>\n";
    $results = BoincResult::enum("userid=$user->id AND (validate_state=0 OR validate_state=4) AND claimed_credit > 0");
    foreach($results as $result) {
        echo "<result>\n";
        echo "    <resultid>".$result->id."</resultid>\n";
        echo "    <workunitid>".$result->workunitid."</workunitid>\n";
        echo "    <hostid>".$result->hostid."</hostid>\n";
        echo "    <claimed_credit>".$result->claimed_credit."</claimed_credit>\n";
        echo "    <received_time>".$result->received_time."</received_time>\n";
        echo "</result>\n";
        $sum += $result->claimed_credit;
    }
    echo "<total_claimed_credit>".$sum."</total_claimed_credit>\n";
    echo "</pending_credit>\n";
} else {
    $user = get_logged_in_user();
    $sum = 0;
    
    page_head("Pending credit");
    start_table();
    echo "<tr><th>Result ID</th><th>Workunit ID</th><th>Host ID</th><th>Claimed credit</th></tr>\n";
    $results = BoincResult::enum("userid=$user->id AND (validate_state=0 OR validate_state=4) AND claimed_credit > 0");
    foreach($results as $result) {
        echo "<tr>\n";
        echo "<td><a href=\"result.php?resultid=$result->id\">$result->id</a></td>\n";
        echo "<td><a href=\"workunit.php?wuid=$result->workunitid\">$result->workunitid</a></td>\n";
        echo "<td><a href=\"show_host_detail.php?hostid=$result->hostid\">$result->hostid</a></td>\n";
        echo "<td>".format_credit($result->claimed_credit)."</td>\n";
        echo "</tr>\n";
        $sum += $result->claimed_credit;
    }
    end_table();
    
    echo "Pending credit: ".format_credit($sum);
    page_tail();
}

?>
