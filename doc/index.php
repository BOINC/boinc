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
<img hspace=30 vspace=10 align=left src=logo/logo_small.png>
<center>
<br>
<h1>Berkeley Open Infrastructure for Network Computing</h1>
An open-source software platform for computing using volunteered
resources.
    <a href=intro.php>... more</a>
<br>
</center>

<br clear=all>

<table width=100% border=0 cellspacing=0 cellpadding=4>
<tr>
<td valign=top>
<table width=100% border=0 cellspacing=0 cellpadding=8>
  <tr><td bgcolor=$light_blue>
    <h2>&nbsp;Donate computing power</h2>
  </td></tr>
  <tr><td>
    <p>
    BOINC lets you donate computing power
    to scientific research projects such as:
    <ul>
    <li> <a href=http://climateprediction.net>Climateprediction.net</a>
        and <a href=http://bbc.cpdn.org>BBC Climate Change Experiment</a>:
        study climate change.
    <li> <a href=http://einstein.phys.uwm.edu/>Einstein@home</a>:
        search for gravitational signals emitted by pulsars.
    <li> <a href=http://lhcathome.cern.ch/>LHC@home</a>:
        improve the design of the CERN LHC particle accelerator
    <li> <a href=http://predictor.scripps.edu>Predictor@home</a>:
        investigate protein-related diseases.
    <li> <a href=http://boinc.bakerlab.org/rosetta/>Rosetta@home<a>:
        help researchers develop cures for human diseases.
    <li> <a href=http://setiathome.berkeley.edu/>SETI@home</a>:
        Look for radio evidence of extraterrestrial life.
    <li> <a href=http://boinc.bio.wzw.tum.de/boincsimap/>SIMAP</a>:
        calculate protein similarity data for use by many biological
        research projects.
    <li> <a href=http://szdg.lpds.sztaki.hu/szdg/>SZTAKI Desktop Grid</a>:
        search for generalized binary number systems..
    <li> <a href=http://www.worldcommunitygrid.org/>World Community Grid</a>:
        advance our knowledge of human disease (requires 5.2.1 or greater).
    <li> <a href=http://www.cellcomputing.net/>Cell Computing</a>
        biomedical research
        (Japanese; requires nonstandard client software).
    </ul>
    <p>
    You can participate in more than one project,
    and you control the fraction of your computing power
    that goes to each project.
    If you participate in several projects,
    your computer will be kept busy even when one project has no work.
    <p>
    To participate:
    <ul>
    <li> <a href=projects.php>Select projects</a>
    <li> <a href=download.php>Download</a> and run BOINC software
    <li> Enter the project's URL (visit the project's web site,
    and copy the URL from your browser's address field).
    </ul>
    <a href=participate.php>More info</a> 
    | <a href=links.php>Web sites </a>
    | <a href=download_network.php>Add-ons</a>
    | <a href=dev/>Message boards</a>

  </td></tr>
  <tr><td bgcolor=$light_blue><h2>Create a volunteer computing project</h2></td></tr>
  <tr><td>
    Scientists with computationally-intensive tasks
    may be able to use BOINC.
    A BOINC project with a single Linux server
    can provide computing power equivalent
    to a cluster with tens of thousands of CPUs.
    <p><a href=create_project.php>More info</a> 
  </td></tr>
  <tr><td bgcolor=$light_blue><h2>Software</h2></td></tr>
  <tr><td>
      <ul>
      <li> <a href=compile.php>Getting and building BOINC software</>
      <li> <a href=boinc_dev.php>Software development process</a>
      <li> <a href=translation.php>Translation</a> of web and GUI text
      <li> Write <a href=download_network.php>'add-on' software</a>:
        <ul>
          <li> <a href=gui_rpc.php>Client GUIs</a>
          <li> <a href=stats.php>Credit statistics web sites</a>
          <li> <a href=acct_mgt.php>Account managers</a>
          <li> <a href=server_status.php>Server status web sites</a>
          <li> <a href=web_rpc.php>Web RPCs</a> for info about users and hosts
          <li> <a href=prefs_override.php>Local editing of preferences</a>
        </ul>
      </ul>
  </td></tr>
  <tr><td bgcolor=$light_blue><h2>Other info</h2></td></tr>
  <tr><td>
    <ul>
    <li> <a href=contact.php>Personnel and contributors</a>
    <li> BOINC <a href=email_lists.php>email lists</a>
    <li> BOINC <a href=dev/>message boards</a>
    <li> <a href=papers.php>Papers related to BOINC</a>
    <li> <a href=logo.php>Logos and graphics</a>
    </ul>
    <br>
    <form method=get action=http://google.com/search>
    <input type=hidden name=domains value=http://boinc.berkeley.edu>
    <input type=hidden name=sitesearch value=http://boinc.berkeley.edu>
    <nobr>
    <input class=small name=q size=30>
    <input type=submit value='Site search'>
    </nobr>
    </form>
  </td></tr>
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
</table>

</td>
";
$i = rand(0, 99);
$j = $i+1;
echo " <td valign=top width=390><table border=0 cellpadding=8 cellspacing=0>
<tr><td bgcolor=$light_blue>
<h2>Featured participant</h2>
</td></tr>
<tr><td>
<br>
One of over 500,000 people around the world participating in
BOINC-based projects.
";
include("piecharts/$i.html");
echo "
<br>
<a href=chart_list.php><b>Top 100</a> |
<a href=http://boinc.netsoft-online.com/rankings.php?list=r_p_1_e><b>Single-computer</a> |
<a href=http://boinc.netsoft-online.com/rankings.php><b>Other lists</a>
</td></tr></table>
<table border=2 cellpadding=8><tr><td bgcolor=$light_blue>
<h2>News</h2>
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
</td></tr>
</table>


<hr>
";
copyright();
echo "</html>\n";
?>
