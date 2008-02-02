<?php

require_once('../inc/forum.inc');

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

if (!get_str('action')) {
    error_page("no action");
}
$thread = BoincThread::lookup_id(get_int('thread'));
$forum = BoincForum::lookup_id($thread->forum);
        
if (!is_moderator($logged_in_user, $forum)) {
    error_page("not authorized");
}

page_head('Moderate');

echo "<form action=forum_moderate_thread_action.php?thread=$thread->id method=POST>\n";
echo form_tokens($logged_in_user->authenticator);
start_table();
row1("Moderate thread");

$action = get_str('action');
switch ($action) {
case 'hide':
case 'lock':
    echo "<input type=hidden name=action value=$action>";
    row2("",
        "Select the reason category, or write a longer description of why you're hiding or locking the thread; then press OK."
    );
    row2("Category",
        "<select name=\"category\">
        <option value=\"1\">Obscene</option>
        <option value=\"2\">Flame/Hate mail</option>
        <option value=\"3\">Commercial spam</option>
        <option value=\"4\">Other</option>
        </select>"
    );
    break;
case 'move':
    if ($forum->parent_type != 0) error_page("Nope");
    echo "<input type=hidden name=action value=move>";
    $selectbox = '<select name="forumid">';  
    $categories = BoincCategory::enum();
    foreach ($categories as $category) {
        $forums = BoincForum::enum("category=$category->id");
        foreach ($forums as $forum) {
            $selectbox .= '<option value="'.$forum->id.'">'.$forum->title.'</option>';  
        }  
    }  
    $selectbox .= '</option>';  
    
    row2("Destination forum:", $selectbox);  
    break;
case 'title':
    echo "<input type=hidden name=action value=title>";
    row2("New title:",
        "<input name=\"newtitle\" value=\"".stripslashes(htmlspecialchars($thread->title))."\">"
    );
    break;
default:
    error_page("Unknown action");
}

row2("Reason<br>Mailed if nonempty",
    "<textarea rows=10 cols=80 name=\"reason\"></textarea>"
);

row2(
    "",
    "<input type=\"submit\" name=\"submit\" value=\"OK\">"
);

end_table();

echo "</form>";

page_tail();

?>
