<?php
 
require_once("project.inc");
require_once("util.inc");
require_once("db.inc");

// TODO: Fix javascript popup tables- they are currently wrapping past
// the edge of the screen.

db_init();

$userid = $_GET['userid'];

if (!$userid) {
  profile_error_page("No user ID was specified.<p>");
  exit();
}

$user = get_user_from_id($userid);

if (!$user) {
  profile_error_page("No user exists for that ID, or there was a database error.<p>");
  exit();
}

// Check for recommendation or rejection votes.
if ($_POST['recommend']) {
  process_results("recommend");
  exit();
} else if ($_POST['reject']) {
  process_results("reject");
  exit();
}

$result = mysql_query("SELECT * FROM profile WHERE userid = $user->id");
if ($result) {
  $profile_info = mysql_fetch_array($result, MYSQL_ASSOC);
}
else {
  profile_error_page("No profile exists for that user, or there was a database error.<p>");
  exit();
}

$logged_in_user = get_logged_in_user(false);  // (false) since anyone can look at profiles.
$can_edit = $logged_in_user && $user->id == $logged_in_user->id;


page_head("User Profile: ".$user->name);

if ($can_edit) {
  echo "<a href=create_profile.php>[Edit Your Profile]</a>";
}

start_table_noborder();
echo "<tr><td>";
show_profile_summary();
echo "</tr></td>";
show_profile_heading1();
echo "<tr><td>", $profile_info['response1'], "<br><br></td></tr>";
show_profile_heading2();
echo "<tr><td>", $profile_info['response2'], "</td></tr>";
end_table();
page_tail();


function show_profile_summary() {
  global $user;
  global $profile_info;
  global $can_edit;

  echo "
<table border=0 cellpadding = 1 width=100%>\n
<tr><td><h1>$user->name</h1></td><td align=\"center\">";

  if (!$can_edit) {
    show_buttons();
  }

echo "</td></tr>\n<tr><td colspan=\"2\">\n";

  // Only display an image if the user has uploaded one.
  if (file_exists(IMAGE_PATH . $user->id . '_sm.jpg')) {
    echo "
<a href=\"" , IMAGE_PATH , $user->id , '.jpg' . "\"><img align=left vspace=6 hspace=9 src=\"" , IMAGE_PATH , $user->id , '_sm.jpg' . "\"></a>\n";
  }
  
  echo "
<font size=\"-1\">
<b>Country:</b> ", $user->country, "&nbsp&nbsp<b>Language:</b> ", $profile_info['language'], "<br>
<b>Email:</b> <a href=\"mailto:", $user->email_addr, "\">", $user->email_addr, "</a><br>
<b>Total Credit:</b> ", $user->total_credit, "<br>";

  if ($user->teamid) {
    $result = mysql_query("select * from team where id = $user->teamid");
    $team = mysql_fetch_object($result);
    echo "<b>Team:</b> <a href=team_display.php?teamid=$team->id>$team->name</a><br>";
  }
  echo "
<b>Date Registered:</b> ", date_str($user->create_time), "
</font>
</td></tr>
</table>
<br>\n";
}

function show_buttons() {
  global $userid;

  echo "
<form action=", $_SERVER['PHP_SELF'], "?userid=$userid method=\"POST\">
<input type=\"submit\" name=\"recommend\" value=\"RECOMMEND\">
<font size=-1><a href=\"javascript:;\" onClick=\"window.open ('explanation.php?val=recommend','_blank','width=350,height=200,left=50,top=150,menubar=0,directories=0,scrollbars=0,resizable=0,status=0')\">what is recommend?</a></font>
<br>
<input type=\"submit\" name=\"reject\" value=\"VOTE TO REJECT\">
<font size=-1><a href=\"javascript:;\" onClick=\"window.open ('explanation.php?val=reject','_blank','width=350,height=200,left=50,top=150,menubar=0,directories=0,scrollbars=0,resizable=0,status=0')\">what is vote to reject?</a></font>
</form>
";
}

function process_results($vote) {
  global $userid;

  if ($vote != "recommend" && $vote != "reject") {
    echo "Invalid vote type.<br>";
    exit();
  }
  
  $result = mysql_query("SELECT * FROM profile WHERE userid = $userid");
  $profile = mysql_fetch_array($result);
  
  $newValue = $profile[$vote] + 1;
  $newresult = mysql_query("UPDATE profile SET $vote = $newValue WHERE userid = $userid");
  
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
  
}
?>