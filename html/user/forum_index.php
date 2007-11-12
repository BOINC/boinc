<?php

// Forum index
// shows the categories and the forums in each category

require_once('../inc/forum.inc');
require_once('../inc/pm.inc');
require_once('../inc/time.inc');

// Process request to mark all posts as read
// ???? uh, why is this here ????

$user = get_logged_in_user(false);

if ((get_int("read", true) == 1)) {
    if ($user) {
        check_tokens($user->authenticator);
        BoincForumPrefs::lookup($user);
        $now = time();
        $user->prefs->update("mark_as_read_timestamp=$now");
        echo "foo";
        exit();
        Header("Location: ".get_str("return", true));
    }
}

function forum_summary($forum) {
    echo "
        <tr class=\"row1\">
        <td>
            <em>
            <a href=\"forum_forum.php?id=$forum->id\">$forum->title
            </a></em>
            <br><span class=\"smalltext\">$forum->description</span>
        </td>
        <td>$forum->threads</td>
        <td>$forum->posts</td>
        <td>".time_diff_str($forum->timestamp, time())."</td>
    </tr>";
}

page_head(tra("%1 Message boards", PROJECT));

echo "
    <p>
    If you have a question or problem, please use the
    <a href=forum_help_desk.php>Questions & answers</a>
    area instead of the Message boards.
    </p>
";

show_forum_title($user, NULL, NULL, NULL, true);
start_forum_table(array(tra("Topic"), tra("Threads"), tra("Posts"), tra("Last post")));

$categories = BoincCategory::enum("is_helpdesk=0 order by orderID");
foreach ($categories as $category) {
    echo '
        <tr class="subtitle">
            <td class="category" colspan="4">'.$category->name.'</td>
        </tr>
    ';
    $forums = BoincForum::enum("parent_type=0 and category=$category->id order by orderID");
    foreach ($forums as $forum) {
        echo forum_summary($forum);
    }
}

end_table();
page_tail();
flush();
BoincForumLogging::cleanup();

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
