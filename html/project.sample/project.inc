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
    echo "This is displayed on the workunit page";
}

function project_user_summary($user){
    echo "Text here will go to the user summary page";
}

function project_user_page_private($user){
    echo "Whatever is here is added to the private account page";
}

function project_user_credit($user){
    if ($user->seti_nresults) {
        row2(
            "SETI@home classic workunits
            <br><font size=-2>as of 14 May 2004</font>",
            number_format($user->seti_nresults)
        );
    }
    if ($user->seti_total_cpu) {
        $x = number_format($user->seti_total_cpu/3600)." hours";
        row2(
            "SETI@home classic CPU time
            <br><font size=-2>as of 14 May 2004</font>",
            $x
        );
    }
}

?>
