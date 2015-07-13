<?php

$host = $_SERVER["SERVER_NAME"];
if ($host == "bossa.berkeley.edu") {
    Header("Location: http://boinc.berkeley.edu/trac/wiki/BossaIntro");
    exit();

}
if ($host == "bolt.berkeley.edu") {
    Header("Location: http://boinc.berkeley.edu/trac/wiki/BoltIntro");
    exit();
}

require_once("docutil.php");
require_once("../html/inc/translation.inc");
require_once("../html/inc/language_names.inc");

function show_participant() {
    $i = rand(0, 99);
    $j = $i+1;
    echo "<table cellpadding=8 cellspacing=0 width=100%>
        <tr><td class=heading_right>
        <center>
        <span class=section_title>".tra("Computing power")."</span>
        <br>
        <a class=heading href=chart_list.php><b>".tra("Top 100 volunteers")."</b></a>
        &middot; <a class=heading href=\"links.php#stats\"><b>".tra("Statistics")."</b></a>
        </center>
        </td></tr>
        <tr><td>
    ";
    show_totals();
    include("piecharts/$i.html");
    echo "</td></tr></table>";
}

function show_totals() {
    $fn = "boinc_state.xml";
    if (!file_exists($fn) || filemtime($fn) < time()-86400) {
        $uid = time();
        $x = file_get_contents("http://boincstats.com/en/xml/boincState?uid=$uid");
        if ($x) {
            $f = fopen($fn, "w");
            fwrite($f, $x);
        } else return;
    }
    $x = file_get_contents($fn);
    $users = parse_element($x, "<participants_active>");
    $hosts = parse_element($x, "<hosts_active>");
    $credit_day = parse_element($x, "<credit_day>");
    $users = number_format($users);
    $hosts = number_format($hosts);

    $petaflops = number_format($credit_day/200000000, 3);
    echo tra("Active:")." $users ".tra("volunteers,")." $hosts ".tra("computers.
")."        <br>".tra("24-hour average:")." $petaflops ".tra("PetaFLOPS.")."
        <hr size=1 width=\"80%\">
    ";
}

function show_news_items() {
    require_once("../html/inc/news.inc");
    require_once("../html/inc/forum.inc");
    echo "
        <table border=0 cellpadding=8>
        <tr><td class=heading_right>
        <center>
        <span class=section_title>".tra("News")."</span>
        </center>
    ";
    if (!file_exists("stop_web")) {
        show_news(0, 5);
    } else {
        echo "<p>Database not available; please try again later.";
    }
    echo "
        </td></tr></table>
    ";
}

function show_participate() {
    echo "
        <tr><td class=heading_left>
    <a href=http://www.facebook.com/pages/BOINC/32672338584><img width=36 src=images/facebook_logo.jpg align=left title='BOINC on Facebook'></a>
    <a href=https://plus.google.com/117150698502685192946/posts><img width=36 src=images/google_plus_logo.jpg align=left title='BOINC on Google+'></a>
        <center>
        <span class=section_title>"
        // "Volunteer" is used as a verb
        .tra("Volunteer")
        ."</span>
        <br>
        <a class=heading href=download.php><b>".tra("Download")."</b></a>
        &middot; <a class=heading href=\"/wiki/BOINC_Help\"><b>".tra("Help")."</b></a>
        &middot; <a class=heading href=\"wiki/User_manual\"><b><span class=nobr>".tra("Documentation")."</span></b></a> 
        &middot; <a class=heading href=addons.php><b><span class=nobr>".tra("Add-ons")."</span></b></a> 
        &middot; <a class=heading href=links.php><b><span class=nobr>".tra("Links")."</span></b></a> 
        </center>
        </td></tr>
        <tr><td>
        <p>
        ".tra("Use the idle time on your computer (Windows, Mac, Linux, or Android) to cure diseases, study global warming, discover pulsars, and do many other types of scientific research.  It's safe, secure, and easy:")."
        <ol>
        <li> <a href=projects.php>".tra("Choose projects")."</a>
        <li> <a href=download.php>".tra("Download BOINC software")."</a>
        <li> ".tra("Enter an email address and password.")."
        </ol>
        <p>
        ".sprintf(
            tra("Or, if you run several projects, try an %saccount manager%s such as %sGridRepublic%s or %sBAM!%s. "),
            "<a href=\"wiki/Account_managers\">",
            "</a>",
            "<a href=\"http://www.gridrepublic.org\">",
            "</a>",
            "<a href=\"http://bam.boincstats.com/\">",
            "</a>"
        )."
        <p>
        For Android devices, download the BOINC
"
//        or <a href=http://www.htc.com/www/go/power-to-give-faqs/>HTC Power To Give</a>
."
        app from the Google Play Store or (for Kindle) the Amazon App Store.
"
//        <a href=http://www.htc.com/www/go/power-to-give-faqs/>
//        <img align=right valign=top height=50 src=images/htc-power-to-give.jpg>
//        </a>
."
        </td></tr>
    ";
}

function show_create() {
    echo "
        <tr><td class=heading_left>
        <center>
        <span class=section_title>".tra("Compute with BOINC")."</span>
        <br>
        <b><a class=heading href=\"trac/wiki/ProjectMain\">".tra("Documentation")."</a></b>
        &middot; <b><a class=heading href=\"trac/wiki/ServerUpdates\">".tra("Software updates")."</a></b>
        </center>
        </td></tr>
        <tr><td>
        <ul>
        <li>
    ",
    tra("%1Scientists%2: use BOINC to create a %3volunteer computing project%4, giving you the power of thousands of CPUs and GPUs.",
        "<b>", "</b>", "<a href=volunteer.php>", "</a>"
    ),
    "<li>",
    tra("%1Universities%2: use BOINC to create a %3Virtual Campus Supercomputing Center%4.",
        "<b>", "</b>",
        "<a href=\"trac/wiki/VirtualCampusSupercomputerCenter\">", "</a>"
    ),
    "<li>",
    tra("%1Companies%2: use BOINC for %3desktop Grid computing%4.",
        "<b>", "</b>", "<a href=dg.php>", "</a>"
    ),
    " </ul>
        </td></tr>
    ";
}

function show_other() {
    echo "
        <tr><td class=heading_left>
        <center>
        <span class=section_title>".tra("About BOINC")."</span>
        </center>
        </td></tr>
        <tr><td>
            BOINC is a
            <a href=\"trac/wiki/ProjectGovernance\">community-based project</a>.
            Anyone can
            <a href=trac/wiki/ContributePage>contribute</a>,
            by programming, testing,
            documenting, translating, or answering questions.
            Those who consistently make positive contributions
            can become part of the project's decision-making process.
            <p>
            <ul>
            <li> <a href=\"dev/\">".tra("Message boards")."</a>
            <li> <a href=\"trac/wiki/EmailLists\">".tra("Email lists")."</a>
            <li> <a href=\"trac/wiki/BoincEvents\">".tra("Events")."</a>
            <li> <a href=trac/wiki/SourceCodeGit>".tra("Source code")."</a>
            <li> <a href=https://github.com/BOINC/boinc/issues>Issue tracker on Github</a>
            <li> <a href=\"trac/wiki/SoftwareAddon\">".tra("APIs for add-on software")."</a>
            </ul>
            <br>
        </td></tr>
    ";
}

function show_nsf() {
    echo "
        <tr><td>
        <img align=left hspace=8 src=nsf.gif alt=\"NSF logo\">
        BOINC is supported by the
        <a href=\"http://nsf.gov\">National Science Foundation</a>
        through awards SCI-0221529, SCI-0438443, SCI-0506411,
                PHY/0555655, and OCI-0721124.
        <span class=note>
        Any opinions, findings, and conclusions or recommendations expressed in
        this material are those of the author(s)
        and do not necessarily reflect the views of the National Science Foundation.
        </span>
        </td></tr>
    ";
}

header("Content-type: text/html; charset=utf-8");

html_tag();
$rh_col_width = 390;

echo "
    <head>
    <link rel=\"shortcut icon\" href=\"logo/favicon.gif\">
    <link rel=\"stylesheet\" type=\"text/css\" href=\"white.css\">
    <link href=\"https://plus.google.com/117150698502685192946\" rel=\"publisher\" />
    <title>BOINC</title>
    <meta name=description content=\"BOINC is an open-source software platform for computing using volunteered resources\">
    <meta name=keywords content=\"distributed scientific computing supercomputing grid SETI@home public computing volunteer computing \">
    </head>
    <body>
    <table width=\"100%\" border=0><tr>
    <td valign=top>
    <img hspace=20 vspace=6 align=left src=\"logo/www_logo.gif\" alt=\"BOINC logo\">
    </td>
    <td align=center>
    <span class=\"title\">
    ".sprintf(tra("Open-source software for volunteer computing"))."
    </span><br><br>
    </td>
    <td width=$rh_col_width align=right>
";
search_form();
language_form();
echo "
    </td></tr>
    </table>

    <table width=\"100%\" border=0 cellspacing=0 cellpadding=4>
    <tr>
    <td valign=top>
    <table width=\"100%\" border=0 cellspacing=0 cellpadding=8>
";
show_participate();
show_create();
show_other();
//show_nsf();
echo "
    </table>
    </td>
";
echo " <td valign=top width=390>
";

show_participant();
show_news_items();

echo "
    </td></tr>
    </table>
";

page_tail(true, true);
?>
