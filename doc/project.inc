<?php

// project.inc for the dev project, linked to from there.
// project_banner() is used.

define("PROJECT", "BOINC");
define("MASTER_URL", "http://boinc.berkeley.edu/dev/");
define("URL_BASE", "http://boinc.berkeley.edu/dev/");
define("SECURE_URL_BASE", "https://boinc.berkeley.edu/dev/");
define("IMAGE_PATH", "../user_profile/images/");
define("IMAGE_URL", "user_profile/images/");
define("PROFILE_PATH", "../user_profile/");
define("PROFILE_URL", "user_profile/");
define("LANGUAGE_FILE", "languages.txt");
define("STYLESHEET", "dev_custom.css");
define("COPYRIGHT_HOLDER", "UC Berkeley");
define("SYS_ADMIN_EMAIL", "davea@ssl.berkeley.edu");
define("POST_REPORT_EMAILS", "elst93@gmail.com, ksmarkspsych@gmail.com, DjangoFett@gmx.net");
define('DISABLE_TEAMS', 1);
define('COUNTRY_FLAGS', 1);
define("SHORTCUT_ICON", "img/favicon.gif");
define('RECORD_USER_IP', true);
define('NO_COMPUTING', true);

define('MODERATORS_CAN_BANISH', '1');

function navbar($prefix, $user, $fixed) {
    $brand = '<a href="http://boinc.berkeley.edu"><img height="50" src="https://boinc.berkeley.edu/logo/boinc_logo.png"></a>&nbsp;&nbsp;';
    navbar_start($brand, $fixed, false);
        // instead of PROJECT you can use an <img> or other HTML

    if ($user) {
        navbar_item(tra("Your account"), $prefix."home.php");
    }

    navbar_menu(tra("Community"), array(
        array(tra("Message boards"), $prefix."forum_index.php"),
        array(tra("User search"), $prefix."user_search.php"),
    ));
    navbar_menu(tra("Site"), array(
        array(tra("Site search"), $prefix."site_search.php"),
        array(tra("Languages"), $prefix."language_select.php")
    ));

    // add your own menu here if you want

    navbar_right($user);
    navbar_end(true);
}

function project_banner($title, $prefix, $is_main) {
    if ($is_main) {
        echo '
            <p> </p>
            <div class="container-fluid">
            <div class="row">
            <div class="col-sm-3" align=center>
            <img src="logo/www_logo.gif" alt="BOINC logo">
            </div>
            <div class="col-sm-6" align="center">
            <h1 style="padding-left:0px; padding-right:30px">
            '.sprintf(tra("Open-source software for volunteer computing")).'
            </h1>
            </div>
            <div class="col-sm-3" align=center>
        ';
        google_search_form("http://boinc.berkeley.edu");
        language_form();
        echo '
            </div>
            </div>
            </div>
        ';
        echo "<p>&nbsp;</p>";
    } else {
        navbar(secure_url_base(), get_logged_in_user(false), false);
        if ($title) {
            echo "<h2>$title</h2>\n";
        }
    }
}

//$fixed_navbar = false;
function project_banner_old($title) {
    echo "
        <a href=http://boinc.berkeley.edu><img align=right src=www_logo.gif></a>
        <center><span class=page_title>$title</span></center>
        <br clear=all>
    ";
}

function project_footer($show_return, $show_date, $prefix) {
    $y = date("Y ");
    echo "
        </div>
        <hr><p class=\"text-muted\">
        Copyright &copy; $y University of California.
        Permission is granted to copy, distribute and/or modify this document
        under the terms of the GNU Free Documentation License,
        Version 1.2 or any later version published by the Free Software Foundation.</p>
            </body></html>
    ";
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
}

function project_user_page_private($user){
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

function project_forum_post_info($forum) {
    if (!$forum || $forum->title != "Questions and problems") return "";
    return "
        <b>When reporting a problem, please include</b>:
        <ul>
        <li> The version of BOINC you're running
            (see Help->About BOINC in the BOINC Manager).
        <li> The computer's operating system type and version (e.g. Windows 8).
        <li> The project(s) involved (e.g. SETI@home, World Community Grid).
        <li> A description of the problem.
        <li> Windows: whether you installed BOINC as a Service.
        <li> <a href=https://boinc.berkeley.edu/dev/forum_thread.php?id=8600>More...</a>
        </ul>
    ";
}

function auth_ops() {
    auth_ops_userid(array(138));
}
?>
