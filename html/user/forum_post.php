<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/subscribe.inc');

db_init();

$logged_in_user = get_logged_in_user(true);
$logged_in_user = getForumPreferences($logged_in_user);

$forumid = get_int("id");
$forum = getForum($forumid);
if (!$forum) {
    error_page("no such forum");
}

$title = post_str("title", true);
$content = post_str("content", true);
if ($title && $content) {
    if ($_POST['add_signature']=="add_it") {
        $add_signature=true;
    } else {
        $add_signature=false;
    }
    $threadID = createThread(
        $forumid, $logged_in_user->id, $title, $content, $add_signature
    );
    if (!$threadID) {
        error_page("Can't create thread (title may be missing)");
    }

    $thread->id=$threadID;
    setThreadLastVisited($logged_in_user,$thread);
    header('Location: forum_thread.php?id=' . $threadID);
}

$category = getCategory($forum->category);

if ($category->is_helpdesk) {
    page_head('Help Desk');
} else {
    page_head('Forum');
}

show_forum_title($forum, NULL, $category->is_helpdesk);

if ($category->is_helpdesk) {
    echo "<p>The <b>Questions and problems</b> area let you
        get help from other users.
        If you have a question or problem:
        <ul>
        <li>
        Read the existing list of questions.
        If your question is already there,
        click on the <b>I also have this question or problem</b> button.
        If answers to the question have been submitted, read them.
        If one of them answers your question, click the
        <b>This answered my question</b> button.
        <li>
        If your question has not already been asked,
        fill out and submit this form.
        </ul>
        This will prevent questions from being asked repeatedly.
    ";
}

echo "<form action=forum_post.php?id=$forumid method=POST>\n";

start_table();
if ($category->is_helpdesk) {
    row1("Submit a new question/problem");
} else {
    row1("Create a new thread");
}

$x = "Title".html_info();

if ($category->is_helpdesk) {
    $x .="<br>
        Describe your question in a few words.
        A brief, clear summary will help others with the same
        question (or an answer) find it.
    ";
}

$y = "<input type=text name=title size=62>";
row2($x, $y);
$x = "Message".html_info().post_warning();

if ($category->is_helpdesk) {
    $x .= " If you are having software problems,
        mention the version number of the software,
        your computer type and operating system.
    ";
}
    

$y = "<textarea name=content rows=12 cols=54></textarea>";
if ($logged_in_user->no_signature_by_default==0) {
    $enable_signature="checked=\"true\"";
} else {
    $enable_signature="";
}
row2($x, $y);
row2("", "<input name=add_signature value=add_it ".$enable_signature." type=checkbox>Add my signature to this post");
row2("", "<input type=submit value=\"OK\">");

end_forum_table();

echo "</form>\n";

page_tail();
?>
