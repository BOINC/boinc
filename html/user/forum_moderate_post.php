<?php
/**
 * The form where a moderator decides what he is going to do to a post.
 * Submits informaiton to forum_moderate_post_action.php for actual action
 * to be done.
 **/

require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');

db_init();

$logged_in_user = re_get_logged_in_user();


if (!get_str('action')) {
    error_page("You must specify an action...");
}
if (!$logged_in_user->isSpecialUser(S_MODERATOR)) {
    // Can't moderate without being moderator
    error_page("You are not authorized to moderate this post.");
}    

$post = new Post(get_int('id'));
$thread = $post->getThread();



page_head('Forum');

//start form
echo "<form action=forum_moderate_post_action.php?id=".$post->getID()." method=POST>\n";
start_table();
row1("Moderate post");

if (get_str('action')=="hide") {
    //display input that selects reason
    echo "<input type=hidden name=action value=hide>";
    row2("",
    "Select the reason category, optionally write a longer describtion of why you delete the post and then press ok to hide it.");
    row2("Category",
    "<select name=\"category\">
    <option value=\"1\">Obscene</option>
    <option value=\"2\">Flame/Hate mail</option>
    <option value=\"3\">Commercial spam</option>
    <option value=\"4\">Doublepost</option>
</select>");
} elseif (get_str('action')=="move") {
    echo "<input type=hidden name=action value=move>";
    row2("Destination thread ID:", "<input name=\"threadid\">");
    //todo display where to move the post as a dropdown instead of having to get ID    
} else {
    error_page( "Unknown action");
}

row2("Reason<br>Mailed if nonempty",
    "<textarea name=\"reason\"></textarea>");

row2(
    "",
    "<input type=\"submit\" name=\"submit\" value=\"OK\">"
);

end_table();

echo "</form>";

page_tail();

?>
