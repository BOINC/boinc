<?php
/**
 * The form where a moderator decides what he is going to do to a post.
 * Submits informaiton to forum_moderate_post_action.php for actual action
 * to be done.
 **/

require_once('../inc/forum_std.inc');
require_once('../inc/forum_user.inc');
require_once('../inc/forum_banishment_vote.inc');

$config = get_config();

db_init();

$logged_in_user = re_get_logged_in_user();


if (!get_str('action')) {
    error_page("You must specify an action...");
}
if (!$logged_in_user->isSpecialUser(S_MODERATOR)) {
    // Can't moderate without being moderator
    error_page("You are not authorized to banish users.");
}    

$userid = get_int('userid');
$user=get_user_from_id($userid);

page_head('Banishment Vote');

//start form
echo "<form action=\"forum_banishment_vote_action.php?userid=".$userid."\" method=\"POST\">\n";
echo form_tokens($logged_in_user->getAuthenticator());
start_table();
row1("Banishment Vote");



if (get_str('action')=="start") {
    if (!$user) {
        error_page("no user");
    }
    $x = $user->banished_until;
    if ($x>time()) {
        error_page("User is already banished");
    }
    //display input that selects reason
    echo "<input type=hidden name=action value=start>";
    echo "<input type=\"hidden\" name=\"userid\" value=\"".$userid."\">\n";
    row1("Are you sure you want to banish ".$user->name."?
        This will prevent ".$user->name." from posting for chosen time period.<br />
        It should be done only if ".$user->name."
        has consistently exhibited trollish behavior.");
    row2("",
    "Select the reason category, optionally write a longer description of why the user should be banished.");
    row2("Category",
    "<select name=\"category\">
    <option value=\"1\">Obscene</option>
    <option value=\"2\">Flame/Hate mail</option>
    <option value=\"3\">User Request</option>
    <option value=\"4\">Other</option>
</select>");
row2("Reason<br>Mailed if nonempty",
    "<textarea name=\"reason\" rows=\"10\" cols=\"80\"></textarea>");

row2(
    "",
    "<input type=\"submit\" name=\"submit\" value=\"Proceed with vote\">"
);
} elseif (get_str('action')=="yes") {
    vote_yes($config,$logged_in_user,$user);
} elseif (get_str('action')=="no") {
    vote_no($config,$logged_in_user,$user);
} else {
    error_page( "Unknown action");
}


end_table();

echo "</form>";

page_tail();

?>
