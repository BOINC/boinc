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
    echo "
        <table cellpadding=8>
        <tr><td class=fieldname>
        <center>
        <span class=section_title>Featured volunteer</span>
        <br>
        <a class=heading href=chart_list.php><b>Top 100</a>
        &middot; <a class=heading href=http://boinc.netsoft-online.com/e107_plugins/boinc/u_rank.php?list=tc_p1c1><b>Single-computer</a>
        &middot; <a class=heading href=http://boinc.netsoft-online.com/e107_plugins/boinc/bu_rankselect.php><b>Other lists</a>
        </center>
        </td></tr>
        <tr><td>
    ";
    include("piecharts/$i.html");
    echo "</td></tr></table>";
}

function show_news_items() {
    require_once("boinc_news.php");
    require_once("../html/inc/news.inc");
    echo "
        <table border=0 cellpadding=8>
        <tr><td class=fieldname>
        <center>
        <span class=section_title>News</span>
        </center>
        <br>
    ";
    $nnews_items = 6;
    show_news($project_news, $nnews_items);
    if (count($project_news) > $nnews_items) {
        echo "<a href=old_news.php>... more</a>\n";
    }

    echo "
        <p><font size=-2>News is available as an
        <a href=rss_main.php>RSS feed</a> <img src=xml.gif></font>
        </td></tr></table>
    ";
}

function show_participate() {
    echo "
        <tr><td class=fieldname>
        <center>
        <span class=section_title>".tr(HOME_HEADING1)."</span>
        <br>
        <a class=heading href=download.php><b>".tr(HOME_DOWNLOAD)."</b></a>
        &middot; <a class=heading href=help.php><b><nobr>".tr(HOME_HELP)."</nobr></b></a> 
        &middot; <a class=heading href=links.php><b><nobr>".tr(HOME_WEB_SITES)."</nobr></b></a>
        &middot; <a class=heading href=addons.php><b>".tr(HOME_ADD_ONS)."</b></a>
        &middot; <a class=heading href=poll.php><b><nobr>".tr(HOME_SURVEY)."</nobr></b></a>
        </center>
        </td></tr>
        <tr><td>
        <p>
        ".sprintf(tr(HOME_P1), "<ol> <li> <a href=projects.php><b>", "</b></a>", "<li> <a href=download.php><b>", "</b></a>", "<li> <b>", "</b>")."
        </ol>
        <p>
        ".sprintf(tr(HOME_P2), "<a href=trac/wiki/AccountManagers>", "</a>", "<a href=http://www.gridrepublic.org>", "</a>", "<a href=http://bam.boincstats.com/>", "</a>")."
        </td></tr>
    ";
}

function show_create() {
    echo "
        <tr><td class=fieldname>
        <center>
        <span class=section_title>Compute with BOINC</span>
        <br>
        <a class=heading href=trac/wiki/CreateProjectOutline>Documentation</a>
        &middot; <a class=heading href=trac/wiki/ServerUpdates>Updates</a>
        &middot; <a class=heading href=trac/wiki/ConferenceList>Conferences</a>
        </center>
        </td></tr>
        <tr><td>
        <b>Scientists</b>: if your group has moderate
        programming, web, sysadmin, and hardware resources,
        you can use BOINC to create a
        <a href=volunteer.php>volunteer computing project</a>.
        With a single Linux server you can get
        the computing power of thousands of CPUs.
        Organizations such as IBM World Community Grid may be able
        to host your project
        (please <a href=trac/wiki/ProjectPeople>contact us</a> for information).
        <br>
        <b>Universities</b>: use BOINC to create a
        <a href=trac/wiki/VirtualCampusSupercomputerCenter>Virtual Campus Supercomputing Center</a>.
        <br>
        <b>Companies</b>:
        use BOINC for <a href=dg.php>desktop Grid computing</a>.
        </td></tr>
    ";
}

function show_other() {
    echo "
        <tr><td class=fieldname>
        <center>
        <span class=section_title>The BOINC project</span>
        </center>
        </td></tr>
        <tr><td>
            <ul>
            <li> <a href=trac/wiki/BoincIntro/>Overview</a>
            <li> <a href=trac/wiki/SoftwareDevelopment>Software development</a>
            <li> <a href=trac/wiki/TranslateIntro>Translation</a> of web and GUI text
            <li> <a href=trac/wiki/ProjectPeople>Personnel and contributors</a>
            <li> BOINC <a href=email_lists.php>email lists</a>
            <li> BOINC <a href=dev/>message boards</a>
            <li> <a href=http://boinc.berkeley.edu/trac/wiki/BoincPapers>Papers and talks</a> on BOINC
            <li> <a href=logo.php>Logos and graphics</a>
            <li> <a href=trac/wiki/BoincEvents>Events</a>
            <li> <a href=trac/wiki/BoltIntro>Bolt</a> (software for web-based education and training)</a>
            <li> <a href=trac/wiki/BossaIntro>Bossa</a> (software for distributed thinking projects)</a>
            </ul>
            <br>
        </td></tr>
    ";
}

function show_nsf() {
    echo "
        <tr><td>
        <img align=left hspace=8 src=nsf.gif>
        BOINC is supported by the
        <a href=http://nsf.gov>National Science Foundation</a>
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

function language_form() {
    echo "
        <table><tr><td>
        <form name=language method=get action=set_language.php>
        <select class=selectbox name=lang onchange=\"javascript: submit()\">
        <option value=auto selected=\"selected\">-- language --
        <option value=auto>Browser default
        <option value=ar>العربية
        <option value=be>Беларускі
        <option value=bg>Български
        <option value=ca>Català
        <option value=de>Deutsch
        <option value=en>English
        <option value=es>Espa&ntilde;ol
        <option value=fr>Fran&ccedil;ais
        <option value=ja>日本語
        <option value=lt>Lietuvių
        <option value=nl>Nederlands
        <option value=pl>Polski
        <option value=pt_BR>Portugu&ecirc;s - Brasil
        <option value=ru>Русский
        <option value=sk>Slovenčina
        <option value=tr>Türkçe
        <option value=zh_CN>简体中文
        </select>
        </form>
        <script>
        document.language.lang.selectedIndex=0;
        </script>
            </td></tr></table>
    ";
}

html_tag();
if (defined("CHARSET")) {
    header("Content-type: text/html; charset=".tr(CHARSET));
}

echo "
<head>
<link rel=\"shortcut icon\" href=\"logo/favicon.gif\">
<link rel=\"stylesheet\" type=text/css href=white.css>
<title>BOINC</title>
<meta name=description content=\"BOINC is an open-source software platform for computing using volunteered resources\">
<meta name=keywords content=\"distributed scientific computing supercomputing grid SETI@home public computing volunteer computing \">
</head>
<body bgcolor=#ffffff>
<table width=100% border=0><tr><td valign=top>
<img hspace=20 vspace=6 align=left src=logo/www_logo.gif>
</td><td align=center>
<span class=subtitle>
".sprintf(tr(HOME_BOINC_DESC), '<a href=volunteer.php><nobr>', '</nobr></a>', '<a href=dg.php><nobr>', '</nobr></a>')."
</span>
<table><tr><td>
";
language_form();
echo "</td><td>";
search_form();
echo "
</td></tr></table>
</td></tr></table>
<table width=100% border=0 cellspacing=0 cellpadding=4>
<tr>
<td valign=top>
<table width=100% border=0 cellspacing=0 cellpadding=8>
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
