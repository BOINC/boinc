<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');
require_once('../inc/subscribe.inc');
require_once('../inc/translation.inc');

db_init();

$logged_in_user = get_logged_in_user(true);
$logged_in_user = getForumPreferences($logged_in_user);

$forumid = get_int("id");
$forum = getForum($forumid);
if (!$forum) {
    error_page(tr(FORUM_ERR_NOT_FOUND));
}
if ($logged_in_user->total_credit<$forum->post_min_total_credit || $logged_in_user->expavg_credit<$forum->post_min_expavg_credit){
    //If user haven't got enough credit (according to forum regulations)
    //We do not tell the (ab)user how much this is - no need to make it easy for them to break the system.
    error_page(sprintf(tr(FORUM_ERR_EXPAVG),$forum->title));
}
if (time()-$logged_in_user->last_post<$forum->post_min_interval){
    //If the user is posting faster than forum regulations allow
    //Tell the user to wait a while before creating any more posts
    error_page(tr(FORUM_ERR_INTERVAL));
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
    //Tell people to first search for answers THEN ask the question...
    echo "<p>".sprintf(tr(FORUM_QA_POST_MESSAGE), "<b>".tr(LINKS_QA)."</b>");
    echo "<ul><li>".sprintf(tr(FORUM_QA_POST_MESSAGE2), "<b>".tr(FORUM_QA_GOT_PROBLEM_TOO)."</b>", "<b>".tr(FORUM_QA_QUESTION_ANSWERED)."</b>");
    echo "<li>".tr(FORUM_QA_POST_MESSAGE3);
    echo "</ul>".tr(FORUM_QA_POST_MESSAGE4);
}

echo "<form action=forum_post.php?id=$forumid method=POST>\n";

start_table();

if ($category->is_helpdesk) {
    row1(tr(FORUM_QA_SUBMIT_NEW)); //New question
    $submit_help = "<br>".tr(FORUM_QA_SUBMIT_NEW_HELP);
    $body_help ="<br>".tr(FORUM_QA_SUBMIT_NEW_BODY_HELP);
} else {
    row1(tr(FORUM_SUBMIT_NEW)); //New thread
    $submit_help = "";
    $body_help = "";
}

//Title
row2(tr(FORUM_SUBMIT_NEW_TITLE).html_info().$submit_help, "<input type=text name=title size=62>");
//Message
row2(tr(FORUM_MESSAGE).html_info().post_warning().$body_help, "<textarea name=content rows=12 cols=54></textarea>");

if ($logged_in_user->no_signature_by_default==0) {
    $enable_signature="checked=\"true\"";
} else {
    $enable_signature="";
}

row2("", "<input name=add_signature value=add_it ".$enable_signature." type=checkbox>".tr(FORUM_ADD_MY_SIG));
row2("", "<input type=submit value=\"OK\">");

end_forum_table();

echo "</form>\n";

page_tail();
?>
