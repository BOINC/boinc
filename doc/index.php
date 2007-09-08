<?php
require_once("docutil.php");
require_once("../html/inc/translation.inc");

function show_participant() {
    $i = rand(0, 99);
    $j = $i+1;
    echo "
        <tr><td>
        <center>
        <hr width=80% size=1>
        <table border=0 cellpadding=6><tr><td>
    ";
    include("piecharts/$i.html");
    echo "
        <br>
        <center>
        <a href=chart_list.php><b>Top 100</a> |
        <a href=http://boinc.netsoft-online.com/e107_plugins/boinc/u_rank.php?list=tc_p1c1><b>Single-computer</a> |
        <a href=http://boinc.netsoft-online.com/e107_plugins/boinc/bu_rankselect.php><b>Other lists</a>
        </center>
        </td></tr></table>
        </td></tr>
    ";
}

function show_news_items() {
    require_once("boinc_news.php");
    require_once("../html/inc/news.inc");
    echo "
        <table border=0 cellpadding=8><tr><td class=fieldname>
        <font size=4>News</font>
        <br>
    ";
    $nnews_items = 8;
    show_news($project_news, $nnews_items);
    if (count($project_news) > $nnews_items) {
        echo "<a href=old_news.php>... more</a>\n";
    }

    echo "
        <p><font size=-2>News is available as an
        <a href=rss_main.php>RSS feed</a> <img src=xml.gif></font>
        <br>
        <font size=-2>You can help
        <a href=wwl.php>translate these news items to other languages</a>.
        </td></tr></table>
        <p>
    ";
}

function show_participate() {
    echo "
        <tr><td class=fieldname>
            <font size=4>&nbsp;".tr(HOME_HEADING1)."</font>
        </td></tr>
        <tr><td>
        <p>
        ".sprintf(tr(HOME_P1), "<ol> <li> <a href=projects.php><font size=+1>", "</font></a>", "<li> <a href=download.php><font size=+1>", "</font></a>", "<li> <font size=+1>", "</font>")."
        </ol>
        <p>
        ".sprintf(tr(HOME_P2), "<a href=acct_mgrs.php>", "</a>", "<a href=http://www.gridrepublic.org>", "</a>", "<a href=http://bam.boincstats.com/>", "</a>")."
        <p>
        ".sprintf(tr(HOME_P3), "<a href=help.php>", "</a>")."
        </td></tr>
    ";
}

function show_create() {
    echo "
        <tr><td class=fieldname><font size=4>Compute with BOINC</font></td></tr>
        <tr><td>
        Learn how to <a href=trac/wiki/CreateProjectOutline>create
        and operate a BOINC project</a>.
        <ul>
        <li> <b>Scientists</b>: if your group has moderate
        programming, web, sysadmin, and hardware resources,
        you can use BOINC to create a
        <a href=volunteer.php>volunteer computing project</a>.
        A BOINC project with a single Linux server
        can provide computing power equivalent
        to a cluster with tens of thousands of CPUs.
        <li>
        Organizations such as IBM World Community Grid may be able
        to host your project
        (please <a href=contact.php>contact us</a> for information).
        <li> <b>Universities</b>: use BOINC to create a
            <a href=vcsc.php>Virtual Campus Supercomputing Center</a>.
        <li> <b>Companies</b>:
            use BOINC for <a href=dg.php>desktop Grid computing</a>.
        </ul>
        </td></tr>
    ";
}

function show_other() {
    echo "
        <tr><td class=fieldname><font size=4>Other info</font></td></tr>
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
            <li> <a href=events.php>Events</a> (Note: the <a href=http://boinc.berkeley.edu/trac/wiki/WorkShop07>3rd Pan-Galactic
                BOINC Workshop</a> will be held 5-6 September 2007 in Geneva, Switzerland.</a>)
            </ul>
            <br>
        </td></tr>
    ";
}

function show_nsf() {
    echo "
        <tr><td>
        <img align=left src=nsf.gif>
        BOINC is supported by the
        <a href=http://nsf.gov>National Science Foundation</a>
        through awards SCI/0221529, SCI/0438443 and SCI/0506411.
        <font size=-2>
        Any opinions, findings, and conclusions or recommendations expressed in
        this material are those of the author(s)
        and do not necessarily reflect the views of the National Science Foundation.
        </font>
        </td></tr>
    ";
}

function language_form() {
    echo "
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
<table border=0><tr><td valign=top>
<img hspace=40 vspace=10 align=left src=logo/www_logo.gif>
</td><td>
<h1>
".tr(HOME_BOINC)."
</h1>
</td></tr>
<tr><td colspan=2>
<font size=+1> &nbsp;
".sprintf(tr(HOME_BOINC_DESC), '<a href=volunteer.php>', '</a>', '<a href=dg.php>', '</a>')."
</font>
</td></tr></table>
<br clear=all>
<table width=100% border=0 cellpadding=8 cellspacing=0><tr><td valign=center>
<a href=download.php><b>".tr(HOME_DOWNLOAD)."</b></a>
| <a href=trac/wiki/RunningBoinc><b><nobr>".tr(HOME_MORE_INFO)."</nobr></b></a> 
| <a href=links.php><b><nobr>".tr(HOME_WEB_SITES)."</nobr></b></a>
| <a href=addons.php><b>".tr(HOME_ADD_ONS)."</b></a>
| <a href=poll.php><b><nobr>".tr(HOME_SURVEY)."</nobr></b></a>
</td><td align=right>
";
language_form();
search_form();
echo "
</td></tr></table>

<table width=100% border=0 cellspacing=0 cellpadding=4>
<tr>
<td valign=top>
<table width=100% border=0 cellspacing=0 cellpadding=8>
";
show_participate();
show_participant();
show_create();
show_other();
show_nsf();
echo "
</table>
</td>
";
echo " <td valign=top width=390>
";

show_news_items();
echo "<table>";
echo "</table>";

echo "
</td></tr>
</table>
";

page_tail(true, true);
?>
