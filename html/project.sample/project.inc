<?php

// This is a sample project file.
// Modify it to suit your project.
// Put your version in html/project/project.inc

define("PROJECT", "Test Project");
define("MASTER_URL", "http://a.b.c");
define("URL_BASE", "http://a.b.c/");
define("IMAGE_PATH", "../user_profile/images/");
define("IMAGE_URL", "user_profile/images/");
define("PROFILE_PATH", "../user_profile/");
define("PROFILE_URL", "user_profile/");
define("LANGUAGE_FILE", "languages.txt");
define("STYLESHEET", "white.css");
define("COPYRIGHT_HOLDER", "Test Group");
define("SYS_ADMIN_EMAIL", "foo@a.b.c");


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

// NOTE: These functions appear here and not in profile.inc because they contain
// project-specific information that needs to be set by the project administrators.

function show_profile_heading1() {
    row1("Your personal background.");
}

function show_profile_question1() {
    rowify("
        Tell us about yourself.
        You could tell us where you're from, your age, occupation, hobbies,
        or anything else about yourself.
    ");
}

function show_profile_heading2() {
    row1("Your opinions about " . PROJECT);
}

function show_profile_question2() {
    rowify("
        Tell us your thoughts about " . PROJECT . "<ol>
        <li>Why do you run " . PROJECT . "?
        <li>What are your views about the project?
        <li>Any suggestions?
        </ol>
    ");
}

?>
