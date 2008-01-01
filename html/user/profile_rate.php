<?php
 
// HUH??? both types of of votes are handled the same!!

require_once("../inc/boinc_db.inc");

$userid = $_GET['userid'];
$vote = $_GET['vote'];

if ($vote != "recommend" && $vote != "reject") {
    echo "Invalid vote type.<br>";
    exit();
}

BoincProfile::update_aux("$vote=$vote+1 WHERE userid = $userid");

page_head("Vote Recorded");

start_table_noborder();

row1("Thank you");

if ($vote == "recommend") {
    rowify("Your recommendation has been recorded.");
} else {
    rowify("Your vote to reject has been recorded.");
}
end_table();
echo "<br><a href=\"view_profile.php?userid=", $userid ,"\">Return to profile.</a>";

page_tail();

?>
