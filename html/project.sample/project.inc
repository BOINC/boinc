<?php
//z

// This is a sample project file.
// Modify it to suit your project.
// Put your version in html_user/project_specific/project.inc

require_once("../inc/util.inc");

define("PROJECT", "Test Project");
define("MASTER_URL", "");
define("URL_BASE", "");
define("IMAGE_PATH", "../user_profile/images/");
define("IMAGE_URL", "user_profile/images/");
define("PROFILE_PATH", "../user_profile/");
define("PROFILE_URL", "user_profile/");
define("LANGUAGE_FILE", "languages.txt");
define("STYLESHEET", "white.css");
define("COPYRIGHT_HOLDER", "Test Group");
define("SYS_ADMIN_EMAIL", "");


function project_intro() {
    echo"
        This is a test project of
        the Berkeley Open Infrastructure for Network Computing (BOINC)
    ";
}

function project_banner($user, $fd) {
    // Put your project title and logo here

    write_fd($fd, "<table width=100%><tr><td><h2>".PROJECT."</h2></td><td align=right>\n");
    if ($user) {
        write_fd($fd, "Logged in as $user->name");
    } else {
        write_fd($fd, "<br>");
    }
    write_fd($fd, "</td></tr></table><hr>\n");
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
        HTML formatting tags are allowed (other tags will be removed),
        and responses are limited to 2000 characters.
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

function project_news() {
    echo "No news is good news\n";
}

?>
