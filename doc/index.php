<?php
require_once("boinc_news.inc");
require_once("../html/inc/news.inc");
echo "
<head>
<link rel=\"shortcut icon\" href=\"iconsmall.ico\">
<link rel=\"stylesheet\" type=text/css href=white.css>
<title>Berkeley Open Infrastructure for Network Computing (BOINC)</title>
<meta name=description content=\"BOINC is a software platform for developing public-participation distributed computing projects\">
<meta name=keywords content=\"distributed scientific computing supercomputing grid SETI@home public computing volunteer computing \">
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
<table width=100% border=0 cellspacing=0 cellpadding=10>
<tr>
<td valign=top width=40%>
";

function section($title, $url, $subtitle=null) {
    echo "<a href=$url><font size=+1 >$title</font></a>\n";
    if ($subtitle) {
        echo "<br> &nbsp;&nbsp;&nbsp;$subtitle\n";
    }
    echo "<br><br>\n";
}

section("
    Overview", "intro.php"
);
section(
    "Participate", "participate.php",
    "Donate computing power to BOINC-based projects"
);

section(
    "Create a BOINC project",
    "create_project.php",
    "Use BOINC to develop distributed applications"
);
section(
    "Software development",
    "boinc_dev.php",
    "Help debug and enhance BOINC software"
);
section(
    "Community and resources",
    "community.php"
);
section(
    "Personnel",
    "contact.php"
);
echo "
<hr noshade size=0>
<img align=left src=nsf.gif>
BOINC is supported by the
<a href=http://nsf.gov>National Science Foundation</a>
through award SCI/0221529.
<font size=-2>
Any opinions, findings, and conclusions or recommendations expressed in
this material are those of the author(s)
and do not necessarily reflect the views of the National Science Foundation.
</font>
</td>
<td valign=top bgcolor=c8c8ff>
<center>
<h2>Projects</h2>
</center>
<p>
BOINC-based distributed computing projects include:
<ul>
<li>
<a href=http://climateprediction.net>Climateprediction.net</a>:
Improve the accuracy of long-term climate prediction.
<li>
<a href=http://einstein.phys.uwm.edu/>Einstein@home</a>:
search data from the Laser Interferometer Gravitational wave Observatory (LIGO)
in the US and from the GEO 600 gravitational wave observatory in Germany for signals coming from rapidly rotating neutron stars, known as pulsars.
<li>
<a href=http://athome.web.cern.ch/athome/>LHC@home</a>:
improve the design of the CERN LHC particle accelerator.
<li>
<a href=http://predictor.scripps.edu>Predictor@home</a>:
Solve biomedical questions and investigate protein-related diseases.
<li><a href=http://setiweb.ssl.berkeley.edu/>SETI@home</a>:
Analyze radio-telescope data, looking for evidence of extraterrestrial life.
</ul>
We encourage you to participate in multiple projects,
so that your computer will be kept busy even
while projects are down or out of work.


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
