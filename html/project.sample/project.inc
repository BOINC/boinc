<?php

// This is a sample project file.
// Modify it to suit your project.
// Put your version in html/project/project.inc

require_once("../inc/util.inc");

$master_url = parse_config(get_config(), "<master_url>");
define("PROJECT", "Test Project");
define("URL_BASE", $master_url);
define("IMAGE_PATH", "../user_profile/images/");
define("IMAGE_URL", "user_profile/images/");
define("PROFILE_PATH", "../user_profile/");
define("PROFILE_URL", "user_profile/");
define("LANGUAGE_FILE", "languages.txt");
define("STYLESHEET", "white.css");
define("COPYRIGHT_HOLDER", "Test Group");
define("SYS_ADMIN_EMAIL", "admin@$master_url");

// Email addresses separated by pipe ( | ) that will receive user reported 
// offensive forum posts.
define("POST_REPORT_EMAILS", "moderator1@$master_url|moderator2@$master_url");

function project_banner($title) {
    // Put your project title and logo here

    echo "<link rel=stylesheet type=text/css href=white.css>
        <table width=100%>
        <tr>
        <td><h2>$title</h2></td>
        <td align=right>
       </td></tr></table>
    ";
}

function project_footer($show_return, $show_date) {
    echo "<br><hr noshade size=1><center>";
    if ($show_return) {
        echo "<a href=".URL_BASE.">Return to ".PROJECT." main page</a><br>\n";
    }
    echo "<br><br>Copyright &copy; ".date("Y ").COPYRIGHT_HOLDER."</center>\n";
    if ($show_date) {
        echo "<font size=-2 color=bbbbbb>Generated ",time_str(time()),"</font>\n";
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
    //echo "This is displayed on the workunit page";
}

function project_user_summary($user){
    //echo "Text here will go to the user summary page";
}

function project_user_page_private($user){
    //echo "Whatever is here is added to the private account page";
}

function project_user_credit($user){
    //echo "Whatever is here is added to the private account page";
}

?>
