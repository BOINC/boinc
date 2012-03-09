<?php

// This is a sample project file.
// Modify it to suit your project.
// Put your version in html/project/project.inc
// add tra() around visible strings to make them translatable
// see inc/translation.inc and http://boinc.berkeley.edu/trac/wiki/TranslateProject for Details

require_once("../inc/util.inc");

$master_url = parse_config(get_config(), "<master_url>");
define("PROJECT", "REPLACE WITH PROJECT NAME");
define("URL_BASE", $master_url);
define("IMAGE_PATH", "../user_profile/images/");
define("IMAGE_URL", "user_profile/images/");
define("PROFILE_PATH", "../user_profile/");
define("PROFILE_URL", "user_profile/");
define("LANGUAGE_FILE", "languages.txt");
define("STYLESHEET", "white.css");
//define("STYLESHEET2", "xxx.css");
define("FORUM_QA_MERGED_MODE", false); // Set to true to merge Message boards and Q&A section
define("COPYRIGHT_HOLDER", "REPLACE WITH COPYRIGHT HOLDER");
define("SYS_ADMIN_EMAIL", "admin@$master_url");
define("UOTD_ADMIN_EMAIL", "admin@$master_url");
    // who gets user of the day pool running low e-mails?
//define("MEMCACHE_SERVERS", "127.0.0.1:11211");

// Email addresses separated by pipe ( | ) that will receive user reported 
// offensive forum posts.
define("POST_REPORT_EMAILS", "moderator1@$master_url|moderator2@$master_url");

// set the following if SVN requires specific configuration (e.g. proxy)
// Used on ops/index.php.
// define("SVN_CONFIG_DIRECTORY", "/home/boincadm/.subversion/");

function project_banner($title, $prefix) {
    // Put your project title and logo here
    // If you include any links, prepend URL with $prefix

    echo "<div class=\"page_title\">$title</div>";
}

function project_footer($show_return, $show_date, $prefix) {
    // If you include any links, prepend URL with $prefix
    echo "<br><hr noshade size=1><center>";
    if ($show_return) {
        echo "<a href=".$prefix."index.php>".tra("Main page")."</a> &middot; <a href=".$prefix."home.php>".tra("Your account")."</a> &middot; <a href=".$prefix."forum_index.php>".tra("Message boards")."</a><br>\n";
    }
    echo "<br><br>".tra("Copyright")." &copy; ".date("Y ").COPYRIGHT_HOLDER."</center>\n";
    if ($show_date) {
        $t = time_str(time());
        echo "<span class=\"note\">".tra("Generated")." $t</span>\n";
    }
}

// Authorize access to administrative pages.
// You can check for login, IP address, or whatever you want.
//
function auth_ops_example() {
    // if running from cmdline, skip checks
    //
    if (!$_SERVER['REMOTE_ADDR']) {
        return;
    }

    // example: require login as a particular user (id 1 in this case)
    //
    if (0) {
        auth_ops_userid(array(1));
        return;
    }

    // example: require that logged-in user have ADMIN or DEV flags
    // set in their forum_prefs.privilege
    //
    if (0) {
        auth_ops_privilege();
        return;
    }
}

function show_profile_heading1() {
    return tra("Your personal background.");
}

function show_profile_question1() {
    return tra("Tell us about yourself. You could tell us where you're from, your age, occupation, hobbies, or anything else about yourself.");
}

function show_profile_heading2() {
    return tra("Your opinions about %1", PROJECT);
}

function show_profile_question2() {
    return tra("Tell us your thoughts about %1<ol>
    <li>Why do you run %1?
    <li>What are your views about the project?
    <li>Any suggestions?
    </ol>", PROJECT);
}

function project_workunit($wu){
    // shown in the workunit page
}

function project_user_summary($user){
    // shown in the user summary page
}

function project_user_page_private($user){
    // shown in the private account page
}

function project_user_credit($user){
    // shown in the the private account page, credit area
}

function project_forum_post_rules() {
    // additional forum post rules
    return "";
}

?>
