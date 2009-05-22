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

function show_participant() {
    $i = rand(0, 99);
    $j = $i+1;
    echo "<table cellpadding=8 cellspacing=0>
        <tr><td class=heading_right>
        <center>
        <span class=section_title>Computing power</span>
        <br>
        <a class=heading href=chart_list.php><b>Top 100 volunteers</b></a>
        &middot; <a class=heading href=\"links.php#stats\"><b>Statistics</b></a>
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
        $x = file_get_contents("http://www.boincstats.com/xml/boinc_state.php");
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

    $teraflops = number_format($credit_day/100000, 2);
    echo "Active: $users volunteers, $hosts computers.
        <br>24-hour average: $teraflops TeraFLOPS.
        <hr size=1 width=\"80%\">
    ";
}

function show_news_items() {
    require_once("boinc_news.php");
    require_once("../html/inc/news.inc");
    echo "
        <table border=0 cellpadding=8>
        <tr><td class=heading_right>
        <center>
        <span class=section_title>News</span>
        </center>
    ";
    $nnews_items = 6;
    show_news($project_news, $nnews_items);
    if (count($project_news) > $nnews_items) {
        echo "<a href=old_news.php>... more</a>\n";
    }

    echo "
        <p><font size=-2>News is available as an
        <a href=rss_main.php>RSS feed</a> <img src=xml.gif alt=\"RSS logo\"></font>
        </td></tr></table>
    ";
}

function show_participate() {
    echo "
        <tr><td class=heading_left>
        <center>
        <span class=section_title>".tra("Volunteer")."</span>
        <br>
        <a class=heading href=download.php><b>".tra("Download")."</b></a>
        &middot; <a class=heading href=\"/wiki/BOINC_Help\"><b>".tra("Help")."</b></a>
        &middot; <a class=heading href=\"wiki/User_manual\"><b><span class=nobr>".tra("Documentation")."</span></b></a> 
        </center>
        </td></tr>
        <tr><td>
        <p>
        ".sprintf(
            tra(" Use the idle time on your computer (Windows, Mac, or Linux) to cure diseases, study global warming, discover pulsars, and do many other types of scientific research.  It's safe, secure, and easy:  %sChoose%s projects  %sDownload%s and run BOINC software  %sEnter%s an email address and password. "),
            "<ol> <li> <a href=projects.php><b>",
            "</b></a>",
            "<li> <a href=download.php><b>",
            "</b></a>",
            "<li> <b>",
            "</b>"
        )."
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
        </td></tr>
    ";
}

function show_create() {
    echo "
        <tr><td class=heading_left>
        <center>
        <span class=section_title>Compute with BOINC</span>
        <br>
        <a class=heading href=\"trac/wiki/ProjectMain\">Documentation</a>
        &middot; <a class=heading href=\"trac/wiki/ServerUpdates\">Software updates</a>
        </center>
        </td></tr>
        <tr><td>
        <ul>
        <li><b>Scientists</b>: 
        use BOINC to create a
        <a href=volunteer.php>volunteer computing project</a>,
        giving you the computing power of thousands of CPUs.
        <li><b>Universities</b>: use BOINC to create a
        <a href=\"trac/wiki/VirtualCampusSupercomputerCenter\">Virtual Campus Supercomputing Center</a>.
        <li><b>Companies</b>:
        use BOINC for <a href=dg.php>desktop Grid computing</a>.
        </ul>
        Related software:
        <ul>
            <li> <a href=\"http://bolt.berkeley.edu/\">Bolt</a>: middleware for web-based education and training
            <li> <a href=\"http://bossa.berkeley.edu/\">Bossa</a>: middleware for distributed thinking projects
        </ul>
        </td></tr>
    ";
}

function show_other() {
    echo "
        <tr><td class=heading_left>
        <center>
        <span class=section_title>The BOINC project</span>
        </center>
        </td></tr>
        <tr><td>
            <ul>
            <li> Help wanted!
            <ul>
                <li> <a href=\"trac/wiki/DevProjects\">Programming</a>
                <li> <a href=\"trac/wiki/TranslateIntro\">Translation</a>
                <li> <a href=\"trac/wiki/AlphaInstructions\">Testing</a>
                <li> <a href=\"trac/wiki/WikiMeta\">Documentation</a>
            </ul>
            <li> <a href=\"trac/wiki/SoftwareDevelopment\">Software development</a>
            <li> <a href=\"trac/wiki/SoftwareAddon\">APIs for add-on software</a>
            <li> <a href=\"trac/wiki/ProjectPeople\">Personnel and contributors</a>
            <li> <a href=\"dev/\">Message boards</a>
            <li> <a href=email_lists.php>Email lists</a>
            <li> <a href=\"trac/wiki/BoincEvents\">Events</a>
            <li> <a href=\"trac/wiki/ConferenceList\">Conferences</a>
            <li> <a href=\"trac/wiki/BoincPapers\">Papers and talks</a>
            <li> <a href=logo.php>Logos and graphics</a>
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

// how to add language names:
// paste into notepad++, select ANSI format,
// then copy/paste into here
function language_form() {
    echo "
        <table><tr><td>
        <form name=language method=get action=set_language.php>
        <select class=selectbox name=lang onchange=\"javascript: submit()\">
        <option value=auto selected=\"selected\">-- language --
        <option value=auto>Browser default"
        //."<option value=ar>العربية (Arabic)"
        //."<option value=be>Беларускі (Belarusian)"
        //."<option value=bg>Българск� (Bulgarian)"
        ."<option value=ca>Català(Catalan)"
        ."<option value=cs>Čeština (Czech)"
        ."<option value=de>Deutsch (German)"
        //."<option value=en>English"
        ."<option value=es>Espa&ntilde;ol (Spanish)"
        ."<option value=fr>Fran&ccedil;ais (French)"
        ."<option value=ja>日本語 (Japanese)"
        ."<option value=ko>한국어 (Korean)"
        //."<option value=lt>Lietuvių (Lithuanian)"
        ."<option value=nl>Nederlands (Dutch)"
        //."<option value=pl>Polski (Polish)"
        //."<option value=pt_BR>Portugu&ecirc;s - Brasil (Portuguese - Brazil)"
        ."<option value=ru>Русский (Russian)"
        //."<option value=sk>Slovenčina (Slovenian)"
        //."<option value=tr>Türkçe (Turkish)"
        ."<option value=zh_CN>简体中文 (Chinese)"
        ."</select>
        </form>
        <script type=\"text/javascript\">
        document.language.lang.selectedIndex=0;
        </script>
            </td></tr></table>
    ";
}

    header("Content-type: text/html; charset=utf-8");

html_tag();
echo "
<head>
<link rel=\"shortcut icon\" href=\"logo/favicon.gif\">
<link rel=\"stylesheet\" href=\"white.css\">
<title>BOINC</title>
<meta name=description content=\"BOINC is an open-source software platform for computing using volunteered resources\">
<meta name=keywords content=\"distributed scientific computing supercomputing grid SETI@home public computing volunteer computing \">
</head>
<table width=\"100%\" border=0><tr><td valign=top>
<img hspace=20 vspace=6 align=left src=\"logo/www_logo.gif\" alt=\"BOINC logo\">
</td><td align=center>
<span class=\"subtitle\">
".sprintf(tra("Open-source software for %svolunteer computing%s and %sgrid computing%s."), '<a href=volunteer.php><span class=nobr>', '</span></a>', '<a href=dg.php><span class=nobr>', '</span></a>')."
</span>
<table><tr><td>
";
language_form();
echo "</td><td>";
search_form();
echo "
</td></tr></table>
</td></tr></table>
<table width=\"100%\" border=0 cellspacing=0 cellpadding=4>
<tr>
<td valign=top>
<table width=\"100%\" border=0 cellspacing=0 cellpadding=8>
";
show_participate();
show_create();
show_other();
show_nsf();
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
