<?
require_once("boinc_news.inc");
require_once("../html/inc/news.inc");
echo "
<head>
<link rel=\"shortcut icon\" href=\"iconsmall.ico\">
<link rel=\"stylesheet\" type=text/css href=white.css>
<title>Berkeley Open Infrastructure for Network Computing (BOINC)</title>
<meta name=description content=\"BOINC is a software platform for developing public-participation distributed computing projects\">
<meta name=keywords content=\"distributed scientific computing supercomputing grid SETI@home public\">
</head>
<body bgcolor=ffffff text=000088 link=000088 vlink=000088>
<img hspace=30 vspace=10 align=left src=boinc.gif>
<center>
<br>
<h1>Berkeley Open Infrastructure for Network Computing</h1>
A software platform for distributed computing using volunteered
computer resources
<br>
</center>

<br clear=all>
<table width=100% border=0 cellspacing=0 cellpadding=20>
<tr>
<td valign=top width=40%>
<a href=intro.php>Overview of BOINC</a>

<br><br>
<a href=participate.php>Participating</a>
<br>
&nbsp;&nbsp;&nbsp;
<font size=-2>
Donate computing power to BOINC-based projects.</font>

<br><br>
<a href=create_project.php>Creating BOINC projects</a>
<br>
&nbsp;&nbsp;&nbsp;
<font size=-2>
Use BOINC to develop distributed applications
</font>

<br><br>
<a href=boinc_dev.php>Developing BOINC</a>
<br>
&nbsp;&nbsp;&nbsp;
<font size=-2>
Help debug and enhance BOINC software.
</font>

<br><br>
<a href=community.php>Community and resources</a>

<br><br>
<a href=contact.php>Acknowledgements</a>

<br><br>
<hr noshade size=0>
<img align=left src=nsf.gif>
BOINC is supported by the
<a href=http://nsf.gov>National Science Foundation</a>
</td>
<td valign=top bgcolor=c8c8ff>
<center>
<h2>Status</h2>
</center>
<p>
BOINC is under development.
We are conducting a
<a href=http://maggie.ssl.berkeley.edu/ap/>beta test of BOINC</a>
using the <a href=http://setiathome.berkeley.edu>SETI@home</a> and
Astropulse applications.
The public release will be announced on the SETI@home web site.
Several other distributed computing projects are evaluating BOINC.

<center>
<h2>News</h2>
</center>
";
show_news($project_news, 6);
if (count($project_news) > 6) {
    echo "<a href=old_news.php>... more</a>\n";
}

echo "
<p><font size=-2>News is available as an
<a href=rss_main.php>RSS feed</a></font>
</table>
";
?>
