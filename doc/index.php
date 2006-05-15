<?php
require_once("docutil.php");
require_once("boinc_news.inc");
require_once("../html/inc/news.inc");

$projects = array(
    array("Climateprediction.net",
        "http://climateprediction.net",
        "Oxford University",
        "Earth sciences",
        "To investigate the approximations that have to be made in state-of-the-art climate models. By running the model thousands of times we hope to find out how the model responds to slight tweaks to these approximations - slight enough to not make the approximations any less realistic. This will allow us to improve our understanding of how sensitive our models are to small changes and also to things like changes in carbon dioxide and the sulphur cycle. This will allow us to explore how climate may change in the next century under a wide range of different scenarios.",
        "cpn_logo_world_1.jpg"
    ),
    array(
        "BBC Climate Change Experiment",
        "http://bbc.cpdn.org/",
        "Oxford University",
        "Earth sciences",
        "The experiment adds the processing power of your home or office computer to thousands of others to predict climate change. The same model that the Met Office uses to make daily weather forecasts has been adapted by climateprediction.net to run on home PCs.<p> The model incorporates many variable parameters, allowing thousands of sets of conditions. Your computer will run one individual set of conditions . in effect your individual version of how the world\'s climate works . and then report back to the climateprediction.net team what it calculates.",

        "cpn_logo_world_1.jpg"
    ),
    array(
        "Seasonal Attribution Project",
        "http://attribution.cpdn.org/",
        "Oxford University",
        "Earth sciences",
        "To determine the extent to which extreme weather events like the United Kingdom floods of Autumn 2000 are attributable to human-induced climate change. We invite you to download and run high-resolution model simulations of the world\'s climate on your own computer. By comparing the results of these simulations, half of which will include the effects of human-induced climate change, and half of which will not, we will investigate the possible impact of human activity on extreme weather risk. This project has fairly high computing requirements, including 1GB RAM.",
        "cpn_logo_world_1.jpg"
    ),
    array(
        "Einstein@home",
        "http://einstein.phys.uwm.edu/",
        "Univ. of Wisconsin - Milwaukee, Albert Einstein Institute",
        "Astrophysics",
        "Search for spinning neutron stars (also called pulsars) using data from the LIGO and GEO gravitational wave detectors. Einstein@Home is a World Year of Physics 2005 project supported by the American Physical Society (APS) and by a number of international organizations.",
        "einstein.jpg"
    ),
    array(
        "LHC@home",
        "http://lhcathome.cern.ch/",
        "CERN (European Organization for Nuclear Research)",
        "Physics",
        "The Large Hadron Collider (LHC) is a particle accelerator which is being built at CERN, the European Organization for Nuclear Research, the world\'s largest particle physics laboratory. When it switches on in 2007, it will be the most powerful instrument ever built to investigate on particles proprieties. LHC@home simulates particles traveling around the LHC to study the stability of their orbits.",
        "lhc.jpg"
    ),
    array(
        "Predictor@home",
        "http://predictor.scripps.edu",
        "Scripps Research Institute",
        "Biology",
        "Protein structure prediction starts from a sequence of amino acids and attempts to predict the folded, functioning, form of the protein.  Predicting the structure of an unknown protein is a critical problem in enabling structure-based drug design to treat new and existing diseases.",
        "predictor.jpg"
    ),
    array(
        "Rosetta@home",
        "http://boinc.bakerlab.org/rosetta/",
        "University of Washington",
        "Biology",
        "Determine the 3-dimensional shapes of proteins in research that may ultimately lead to finding cures for some major human diseases. By running Rosetta@home you will help us speed up and extend our research in ways we couldn\'t possibly attempt without your help. You will also be helping our efforts at designing new proteins to fight diseases such as HIV, Malaria, Cancer, and Alzheimer\'s",
        "rosetta_at_home_logo.jpg"
    ),
    array(
        "SETI@home",
        "http://setiathome.berkeley.edu/",
        "U.C. Berkeley Space Sciences Laboratory",
        "Astrophysics, astrobiology",
        "SETI (Search for Extraterrestrial Intelligence) is a scientific area whose goal is to detect intelligent life outside Earth. One approach, known as radio SETI, uses radio telescopes to listen for narrow-bandwidth radio signals from space. Such signals are not known to occur naturally, so a detection would provide evidence of extraterrestrial technology.",
        "seti_logo.png"
    ),
    array(
        "SIMAP",
        "http://boinc.bio.wzw.tum.de/boincsimap/",
        "Technical University of Munich",
        "Biology",
        "Calculate similarities between proteins. SIMAP provides a public database of the resulting data, which plays a key role in many bioinformatics research projects.",
        "simaplogo.jpg"
    ),
    array(
        "SZTAKI Desktop Grid",
        "http://szdg.lpds.sztaki.hu/szdg/",
        "MTA-SZTAKI Laboratory of Parallel and Distributed Systems (Budapest)",
        "Mathematics",
        "Find all the generalized binary number systems (in which bases are matrices and digits are vectors) up to dimension 11.",
        "szdg1_small.jpg"
    ),
    array(
        "World Community Grid",
        "http://www.worldcommunitygrid.org/",
        "IBM",
        "Biomedicine",
        "Advance our knowledge of human disease.",
        "wcg.jpg",
    ),
    array(
        "Quantum Monte Carlo at Home",
        "http://qah.uni-muenster.de/",
        "University of Muenster",
        "Chemistry",
        "Study the structure and reactivity of molecules using Quantum Chemistry.",
        "logo_oben.jpg"
    ),
);

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
<br>
";
search_form();
echo "
</center>
</div>
<br clear=all>

<table width=100% border=0 cellspacing=0 cellpadding=4>
<tr>
<td valign=top>
<table width=100% border=0 cellspacing=0 cellpadding=8>
  <tr><td bgcolor=$light_blue>
    <h2>&nbsp;Participate</h2>
  </td></tr>
  <tr><td>
    <p>
    BOINC lets you donate computing power
    to scientific research projects (mouse over for details):
    <ul>
";
shuffle($projects);
foreach ($projects as $p) {
    echo "<li> <a href=$p[1] onmouseover=\"return escape('<img align=right vspace=4 hspace=4 src=images/$p[5]><b>Home:</b> $p[2]<br><b>Area:</b> $p[3]<br><b>Goal:</b> $p[4]')\">$p[0]</a>
    ";
}
echo "
    </ul>
    <p>
    BOINC is an open-source software platform for volunteer computing.
    You can participate in several projects, ensuring that
    your computer will be kept busy even when one project has no work.
    <p>
    To participate:
    <ol>
    <li> <a href=projects.php>Select projects</a>
    <li> <a href=download.php>Download</a> and run BOINC software
    <li> Enter the projects' URLs (visit each project's web site,
    and copy the URL from your browser's address field).
    </ol>
    <center>
    <table cellpadding=1 width=100%><tr>
    <td bgcolor=$med_blue align=middle>
        <a href=participate.php><b><nobr>More info</nobr></b></a> 
    </td>
    <td bgcolor=$med_blue align=middle>
        <a href=download.php><b>Download</b></a>
    </td>
    <td bgcolor=$med_blue align=middle>
        <a href=links.php><b><nobr>Web sites</nobr></b></a>
    </td>
    <td bgcolor=$med_blue align=middle>
        <a href=download_network.php><b>Add-ons</b></a>
    </td>
    <td bgcolor=$med_blue align=middle>
        <a href=dev/><b><nobr>Message boards</nobr></b></a>
    </td>
    </tr></table>
    </center>

  </td></tr>
  <tr><td bgcolor=$light_blue><h2>Create a volunteer computing project</h2></td></tr>
  <tr><td>
    If you are a scientist with a computationally-intensive task,
    you may be able to use BOINC.
    A BOINC project with a single Linux server
    can provide computing power equivalent
    to a cluster with tens of thousands of CPUs.
    <p>
    If you lack the resources (manpower, server capacity,
    or network bandwidth) to operate a BOINC project directly,
    organizations such as World Community Grid may be able
    to assist you.
    Please <a href=contact.php>contact us</a> for information.
    <center>
    <table cellpadding=1><tr>
    <td bgcolor=$med_blue align=middle>
        <a href=create_project.php><b>Documentation</b></a>
    </td>
    </center>
    </tr></table>
  </td></tr>
  <tr><td bgcolor=$light_blue><h2>Software</h2></td></tr>
  <tr><td>
      <ul>
      <li> <a href=compile.php>Getting and building BOINC software</a>
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
    <li> <a href=intro.php>Overview</a>
    <li> <a href=contact.php>Personnel and contributors</a>
    <li> BOINC <a href=email_lists.php>email lists</a>
    <li> BOINC <a href=dev/>message boards</a>
    <li> <a href=papers.php>Papers related to BOINC</a>
    <li> <a href=logo.php>Logos and graphics</a>
    </ul>
    <br>
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
One of over 600,000 people worldwide participating in BOINC:
<p>
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
echo "
    <script language=\"JavaScript\" type=\"text/javascript\" src=\"wz_tooltip.js\"></script>
    </body>
    </html>
";
?>
