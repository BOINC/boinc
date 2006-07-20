<?php
require_once("docutil.php");
require_once("projects.inc");

function show_participant() {
    global $light_blue;
    $i = rand(0, 99);
    $j = $i+1;
    echo "
        <tr><td align=middle>
        <hr width=70%>
    ";
    include("piecharts/$i.html");
    echo "
        <br>
        <center>
        <a href=chart_list.php><b>Top 100</a> |
        <a href=http://boinc.netsoft-online.com/rankings.php?list=r_p_1_e><b>Single-computer</a> |
        <a href=http://boinc.netsoft-online.com/rankings.php><b>Other lists</a>
        </center>
        </td></tr>
    ";
}

function show_news_items() {
    global $light_blue;
    require_once("boinc_news.inc");
    require_once("../html/inc/news.inc");
    echo "
        <table border=2 cellpadding=8><tr><td bgcolor=$light_blue>
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
        </td></tr></table>
        <p>
    ";
}

function show_participate() {
    global $light_blue;
    echo "
        <tr><td bgcolor=$light_blue>
            <font size=4>&nbsp;Volunteer</font>
        </td></tr>
        <tr><td>
        <p>
        Use the idle time on your computer
        (Windows, Mac, or Linux)
        to do all sorts of scientific research:
        <ol>
        <li> <a href=projects.php>Choose projects</a>
        <li> <a href=download.php>Download</a> and run BOINC software
        <li> Enter the projects' URLs, your email address, and password.
        </ol>
        <center>
        <a href=participate.php><b><nobr>Details</nobr></b></a> 
        | <a href=download.php><b>Download</b></a>
        | <a href=links.php><b><nobr>Web sites</nobr></b></a>
        | <a href=download_network.php><b>Add-ons</b></a>
        | <a href=poll.php><b><nobr>Survey</nobr></b></a>
        </center>
        </td></tr>
    ";
}

function show_create() {
    global $light_blue;
    echo "
        <tr><td bgcolor=$light_blue><font size=4>Create a volunteer computing project</font></td></tr>
        <tr><td>
        If you are a scientist with a computationally-intensive task,
        you may be able to use BOINC.
        A BOINC project with a single Linux server
        can provide computing power equivalent
        to a cluster with tens of thousands of CPUs.
        <p>
        Learn how to <a href=create_project.php><b>create
        and operate a BOINC project</b></a>.
        <p>
        If you lack the resources to operate a BOINC project directly,
        organizations such as World Community Grid may be able
        to assist you.
        Please <a href=contact.php>contact us</a> for information.
        </td></tr>
    ";
}

function show_other() {
    global $light_blue;
    echo "
        <tr><td bgcolor=$light_blue><font size=4>Other info</font></td></tr>
        <tr><td>
            <ul>
            <li> <a href=boinc_dev.php>Software development</a>
            <li> <a href=intro.php>Overview</a>
            <li> <a href=contact.php>Personnel and contributors</a>
            <li> BOINC <a href=email_lists.php>email lists</a>
            <li> BOINC <a href=dev/>message boards</a>
            <li> <a href=papers.php>Papers related to BOINC</a>
            <li> <a href=logo.php>Logos and graphics</a>
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

echo "
<html>
<head>
<link rel=\"shortcut icon\" href=\"iconsmall.ico\">
<link rel=\"stylesheet\" type=text/css href=white.css>
<title>BOINC</title>
<meta name=description content=\"BOINC is an open-source software platform for computing using volunteered resources\">
<meta name=keywords content=\"distributed scientific computing supercomputing grid SETI@home public computing volunteer computing \">
</head>
<body bgcolor=#ffffff>
<img hspace=30 vspace=10 align=left src=logo/logo_small.png>
<h1>
Berkeley Open Infrastructure for Network Computing
</h1>
BOINC is free, open-source software for
<a href=volunteer.php>volunteer computing</a> and desktop grid computing.
<p>
";
search_form();
echo "
<br clear=all>

<table width=100% border=0 cellspacing=0 cellpadding=4>
<tr>
<td valign=top>
<table width=100% border=0 cellspacing=0 cellpadding=8>
";
show_participate();
show_participant();
show_create();
show_other();
echo "
</table>
</td>
";
echo " <td valign=top width=390>
";

show_news_items();
echo "<table>";
show_nsf();
echo "</table>";

echo "
</td></tr>
</table>


<hr>
";
copyright();
echo "
    <script language=\"JavaScript\" type=\"text/javascript\" src=\"wz_tooltip.js\"></script>
    </body>
    </html>
";
?>
