<?php
require_once("docutil.php");
require_once("boinc_news.inc");
require_once("../html/inc/news.inc");

echo "
<html>
<head>
<link rel=\"shortcut icon\" href=\"iconsmall.ico\">
<link rel=\"stylesheet\" type=text/css href=white.css>
<title>Berkeley Open Infrastructure for Network Computing (BOINC)</title>
<meta name=description content=\"BOINC is a software platform for developing public-participation distributed computing projects\">
<meta name=keywords content=\"distributed scientific computing supercomputing grid SETI@home public computing volunteer computing \">
</head>
<body bgcolor=#ffffff text=#000088 link=#000088 vlink=#000088>
<img hspace=30 vspace=10 align=left src=boinc.gif>
<center>
<br>
<h1>Berkeley Open Infrastructure for Network Computing</h1>
A software platform for distributed computing using volunteered
computer resources
<br>
</center>

<br clear=all>

<table width=100% border=0 cellspacing=0 cellpadding=4>
<tr>
<td valign=top width=60%>
<table width=100% border=0 cellspacing=0 cellpadding=8>
  <tr><td bgcolor=$light_blue>
    <h2>&nbsp;Donate computing power to science</h2>
  </td></tr>
  <tr><td>
    <p>
    If you have a computer (Windows, Mac, Linux or Unix)
    you can participate in many scientific research projects:
    <ul>
    <li>
    <a href=http://climateprediction.net>Climateprediction.net</a>:
    study climate change
    <li>
    <a href=http://einstein.phys.uwm.edu/>Einstein@home</a>:
    search for gravitational signals coming from pulsars
    <li>
    <a href=http://athome.web.cern.ch/athome/>LHC@home</a>:
    improve the design of the CERN LHC particle accelerator
    <li>
    <a href=http://predictor.scripps.edu>Predictor@home</a>:
    investigate protein-related diseases
    <li><a href=http://setiweb.ssl.berkeley.edu/>SETI@home</a>:
    Look for radio evidence of extraterrestrial life
    <li><a href=http://www.cellcomputing.net/>Cell Computing</a>
        biomedical research
        (Japanese; requires nonstandard client software)
    </ul>
    To participate in a project:
    <br>
    1) Visit the project's web site and create an account.
    <br>
    2) <a href=download.php>Download</a> and run BOINC software.
    <p>
    You can participate in any or all projects,
    and you control the percentage of your computing power
    that goes to each project.
    By participating in several projects,
    you ensure that your computer will be kept busy
    even when one project has no work.
    <p>
    <a href=participate.php>... more</a>

    <br><br>
  </td></tr>
  <tr><td bgcolor=$light_blue><h2>Create a BOINC project</h2></td></tr>
  <tr><td>
    If you're a scientist with a computationally-intensive task,
    you may be able to use BOINC.
    <p>
    A BOINC project requires just a single Linux server,
    and can provide computing power equivalent
    to a cluster with tens of thousands of nodes.
    <p>
      <a href=create_project.php>
      ... more
    </a>
    <br><br>
  </td></tr>
  <tr><td bgcolor=$light_blue><h2>Other Info</h2></td></tr>
  <tr><td>
    <ul>
    <li> <a href=contact.php>Personnel and contact info</a>
    <li> BOINC <a href=email_lists.php>email lists</a>
    <li> BOINC <a href=dev/>message boards</a>
    <li> An <a href=intro.php>overview of BOINC</a>, and links to papers
    <li> Wikis (user-editable information and documentation):
        <ul>
        <li> <a href=http://boinc-doc.net/boinc-wiki/>The Unofficial BOINC Wiki</a> (in English)
        <li> <a href=http://faq.boinc.de/>Deutsche BOINC FAQ</a> (in German)
        <li> <a href=http://www.boincfrance.org/wakka.php?wiki=BienVenue>BOINCFrance.org</a> (in French)
        </ul>
    <li> <a href=links.php>Web sites</a> for BOINC participants
    <li>
        <a href=boinc_dev.php>Software development</a>
        <blockquote>
        BOINC is free software,
        distributed under the Lesser GNU Public License (LGPL).
        If you are fluent in C++, PHP, SQL, and/or Python,
        you may be able to help debug and enhance BOINC.
        </blockquote>
    <li>
      Non-English <a href=translation.php>translations</a>
      of project web sites and the BOINC manager.
    <li>
    <a href=logo.php>Logos and graphics</a>
    </ul>
    <br>
  </td></tr>
<tr><td>
<img align=left src=nsf.gif>
BOINC is supported by the
<a href=http://nsf.gov>National Science Foundation</a>
through award SCI/0221529.
<font size=-2>
Any opinions, findings, and conclusions or recommendations expressed in
this material are those of the author(s)
and do not necessarily reflect the views of the National Science Foundation.
</font>
</td></tr>
</table>

</td>
<td valign=top>
<table border=2 cellpadding=8><tr><td bgcolor=$light_blue>
<center>
    <form method=get action=http://www.google.com/search>
    <input type=hidden name=domains value=http://boinc.berkeley.edu>
    <input type=hidden name=sitesearch value=http://boinc.berkeley.edu>
    <nobr>
    <input class=small name=q size=30>
    <input type=submit value='Site search'>
    </nobr>
    </form>
<h2>News</h2>
</center>
";
$nnews_items = 8;
show_news($project_news, $nnews_items);
if (count($project_news) > $nnews_items) {
    echo "<a href=old_news.php>... more</a>\n";
}

echo "
<p><font size=-2>News is available as an
<a href=rss_main.php>RSS feed</a></font>
</td></tr></table>
</td></tr>
</table>


<hr>
";
copyright();
echo "</html>\n";
?>
