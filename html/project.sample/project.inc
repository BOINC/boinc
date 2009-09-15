<?php

// This is a sample project file.
// Modify it to suit your project.
// Put your version in html/project/project.inc

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
define("FORUM_QA_MERGED_MODE", false); // Set to true to merge Message boards and Q&A section
define("COPYRIGHT_HOLDER", "REPLACE WITH COPYRIGHT HOLDER");
define("SYS_ADMIN_EMAIL", "admin@$master_url");
define("UOTD_ADMIN_EMAIL", "admin@$master_url");
    // who gets user of the day pool running low e-mails?

// Email addresses separated by pipe ( | ) that will receive user reported 
// offensive forum posts.
define("POST_REPORT_EMAILS", "moderator1@$master_url|moderator2@$master_url");

// set the following var if your project is behind a proxy.
// This is used on ops/index.php to get the current SVN rev from BOINC
//$project_http_proxy = "tcp://proxyname:port"

function project_banner($title, $prefix) {
    // Put your project title and logo here
    // If you include any links, prepend URL with $prefix

    echo "
        <p>
        <span class=\"page_title\">$title</span>
        <p>
    ";
}

function project_footer($show_return, $show_date, $prefix) {
    // If you include any links, prepend URL with $prefix
    echo "<br><hr noshade size=1><center>";
    if ($show_return) {
        echo "<a href=".$prefix."index.php>Home</a> | <a href=".$prefix."home.php>My Account</a> | <a href=".$prefix."forum_index.php>Message Boards</a><br>\n";
    }
    echo "<br><br>Copyright &copy; ".date("Y ").COPYRIGHT_HOLDER."</center>\n";
    if ($show_date) {
        $t = time_str(time());
        echo "<span class=\"note\">Generated $t</span>\n";
    }
}

// Authorize an access to an administrative page.
// You can check for login, IP address, or whatever you want.
//
function auth_ops() {
    if (1) {
        echo "To allow access to admin pages,
            you must define a function auth_ops() in html/project/project.inc
            that specifies your access control policy.
        ";
        exit;
    }

    // example:
    if (0) {
        $admin_user_ids = array(1);
        $user = get_logged_in_user_ops();
        if (!$user) {
            echo "You must log in to performance admin functions.<p>\n";
            print_login_form_ops();
            exit;
        } else if (!$in_array($user->id, $admin_user_ids)) {
            echo "
                You must be logged in as an administrator to perform admin functions.
                <p>
                <a href=logout.php>Log out</a>
            ";
            exit;
        }
    }
}

// Same, but for command-line scripts
//
function auth_ops_cmdline() {
    if ($_SERVER['REMOTE_ADDR']) {
        error_page("Web access to this script is not allowed.");
    }
}

function show_profile_heading1() {
    return "Your personal background.";
}

function show_profile_question1() {
    return "
        Tell us about yourself.
        You could tell us where you're from, your age, occupation, hobbies,
        or anything else about yourself.
    ";
}

function show_profile_heading2() {
    return "Your opinions about " . PROJECT;
}

function show_profile_question2() {
    return "
        Tell us your thoughts about " . PROJECT . "<ol>
        <li>Why do you run " . PROJECT . "?
        <li>What are your views about the project?
        <li>Any suggestions?
        </ol>
    ";
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
