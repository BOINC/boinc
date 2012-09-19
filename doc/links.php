<?php

require_once("docutil.php");

include("../html/inc/stats_sites.inc");

function language($lang, $sites) {
    echo "<tr><td bgcolor=eeeeee valign=top width=250>$lang</td><td>\n";
    shuffle($sites);
    foreach ($sites as $s) {
        echo "$s<br>\n";
    }
    echo "</td></tr>\n";
}

function site($url, $name) {
    return "<a href=$url>$name</a>";
}

$info_sites = array(
    array(
        "http://projekty.czechnationalteam.cz/",
        "BOINC projects",
        "(in Czech)"
    ),
    array(
        "http://www.boincteams.com/",
        "BOINC Team Leaders Forum",
        "(a meeting place to chat and discuss team building on BOINC projects)"
    ),
    array(
        "http://www.hyper.net/dc-howto.html",
        "How to participate in grid computing projects that benefit humanity",
        "(survey of volunteer computing, including non-BOINC projects)"
    ),
    array(
        "http://www.kd-web.info/clanky.php",
        "Flash-based BOINC tutorials", "(in Czech, English, and Slovak)"
    ),
    //array(
    //    "http://www.kazlev.karoo.net/noob_help.htm",
    //    "BOINC mini-FAQ"
    //),
    array(
        "http://boincfaq.mundayweb.com/",
        "The BOINC FAQ Service",
        "(English, German, Dutch, Spanish and French)"
    ),
    array(
        "http://www.boinc-wiki.info/",
        "The Unofficial BOINC Wiki",
        "(in English)",
    ),
    array(
        "http://www.seti-argentina.com.ar/instrucciones-boinc-manager",
        "BOINC Argentina",
        "(in Spanish)",
    ),
    array(
        "http://faq.boinc.de/",
        "Deutsche BOINC FAQ",
        "(in German)",
    ),
    array(
        "http://www.boincfrance.org/",
        "BOINCFrance.org",
        "(in French)",
    ),
    array(
        "http://www.crunching-family.at/wiki/",
        "Crunching Family Wiki",
        "(In German)",
    ),
    array(
        "http://www.angelfire.com/jkoulouris-boinc/",
        "The Big BOINC! Projects and Chronology Page",
        "(by John Koulouris)"
    ),
);

page_head("Web resources for BOINC participants");

echo "
<h3>Contents</h3>
<ul>
<li> <a href=#info>Help and Information</a>
<li> <a href=#stats>Credit statistics</a>
<li> <a href=#sigs>Signature images</a>
<li> <a href=#team_stats>Team statistics</a>
";
//<li> <a href=#status>Project status</a>
echo "
<li> <a href=#misc>Miscellaneous</a>
<li> <a href=#skins>Skins for the BOINC Manager</a>
<li> <a href=#sites>Other BOINC-related sites</a>
(Information, message boards, and teams)
<li> <a href=#video>BOINC-related videos</a>
</ul>
<a name=info></a>
<h3>Help and Information</h3>
Sites with information and documentation about BOINC.
";
shuffle($info_sites);
site_list($info_sites);
echo "
<a name=stats></a>
<h3>Credit statistics</h3>
<p>
The following web sites show statistics for one or more BOINC projects.
These sites use XML-format data exported by BOINC projects,
as described
<a href=http://boinc.berkeley.edu/trac/wiki/CreditStats>here</a>.
If you're interested in running your own site or
participating in the development efforts,
please contact the people listed below.
";
shuffle($stats_sites);
site_list($stats_sites);
echo "
<a name=sigs></a>
<h3>Signature images</h3>
<p>
The following sites offer dynamically-generated
images showing your statistics in BOINC projects,
and/or news from projects.
Use these in your email or message-board signature.
";
shuffle($sig_sites);
site_list($sig_sites);
echo "
<a name=team_stats></a>
<h3>Team statistics</h3>
";
shuffle($team_stats_sites);
site_list($team_stats_sites);
if (0) {
    echo "
        <a name=status></a>
        <h3>Project status sites</h3>
        Show if the servers of various projects are up or down.
        <ul>
    ";
    //<li> <a href=http://www.esea.dk/esea/bos.asp>BOS (BOINC Online Schedulers></a>
    echo "
        <li> <a href=http://boincprojectstatus.ath.cx/>BOINC Project Status</a>
        </ul>
    ";
}
echo "
<a name=misc></a>
<h3>Miscellaneous</h3>
";
$misc_sites = array(
    //array("http://www.myboinc.com/", "BOINC Users of the Day"),
    //array("http://groups.myspace.com/BOINConMYSPACE", "BOINC on MySpace"),
    //array("http://www.boincuk.com/repository.php", "bunc", "(excellent newsletter produced by BOINC UK)"),
    array("http://www.linkedin.com/groups?gid=678497", "BOINC group on LinkedIn"),
    array("http://www.facebook.com/#!/pages/BOINC/109465765746025?ref=ts", "BOINC on Facebook"),
);
echo "<ul>";
foreach ($misc_sites as $m) {
    $u = $m[0];
    $t1 = $m[1];
    $t2 = $m[2];
    echo "<li> <a href=$u>$t1</a> $t2
    ";
}
echo "
</ul>
<a name=skins></a>
<h3>Skins for the BOINC Manager</h3>
<ul>
<li> <a href=http://www.crunching-family.at/download-center/>Crunching Family Skin Download</a>
<li> <a href=http://www.czechnationalteam.cz/view.php?cisloclanku=2007040003>Czech National Team skin</a> (in Czech)
<li> <a href=http://www.grid-france.fr/tutos/boinc-personnaliser-aux-couleurs-equipe >Skin for Equipe France (WCG)</a>
";
//<li> <a href=http://wcg.userfriendly.org/resources.jspx>Skin for team UserFriendly.org</a>
echo "
</ul>
<a name=sites></a>
<h3>Other BOINC-related web sites</h3>
";
list_start();
echo "
<tr><th>Language</th><th>Site</th></tr>
";

language("Belgium (Dutch/French/English)", array(
    site("http://www.boinc.be", "www.boinc.be"),
    site("http://icewolves.plid.be", "IceWolves"),
));
language("Bulgarian", array(
    site("http://www.boinc-bulgaria.net", "BOINC Bulgaria")
));
language("Catalan", array(
    site("http://www.boinc.cat", "BOINC.cat"),
));
language("Chinese", array(
    site("http://boinc.equn.com/", "boinc.equn.com")
));
language("Czech", array(
    site("http://www.czechnationalteam.cz/", "Czech National Team"),
    site("http://www.boincteamcz.net/", "BOINC Team CZ"),
    site("http://www.boinc.cz/", "www.boinc.cz")
));
language("Danish", array(
    site("http://boincdenmark.dk", "BOINC@Denmark"),
    site("http://www.boinc.dk", "www.boinc.dk"),
    site("http://www.setihome.dk", "www.setihome.dk")
));
language("Dutch", array(
    site("http://www.dutchpowercows.org/", "Dutch Power Cows
        </a>(also <a href=http://gathering.tweakers.net/forum/list_topics/5>forums</a>)"
    ),
    site("http://www.seti.nl/content.php?c=boincmain",
        "SETI@Netherlands"
    ),
    site("http://www.boinc.be", "www.boinc.be"),
));
language("English", array(
    site("http://z15.invisionfree.com/The_Boinc_Bar/index.php?act=idx", "The BOINC Bar"),
    site("http://www.s15.invisionfree.com/Crunchers_Inc/index.php?act=idx", "Crunchers Inc."),
    site("http://www.calmchaosonline.com/", "Calm Chaos"),
    site("http://www.teamphoenixrising.net/", "Team Phoenix Rising"),
    site("http://www.unitedmacs.com/", "United Macs"),
    //site("http://sirans-boincnews.com/", "Siran's BOINC Projects News Site"),
    site("http://www.ukboincteam.org.uk/", "UK BOINC Team"),
    site("http://symbion.madnezz.com/", "Symbion"),
    site("http://scotlandsseti.blogspot.com/", "Megacruncher's Blog"),
    site("http://www.bc-team.org/", "BOINC Confederation"),
    //site("http://theclangers.net/", "The Clangers"),
    site("http://www.free-dc.org/", "Free-DC"),
    site("http://forums.anandtech.com/categories.aspx?catid=39&entercat=y", "TeAm Anandtech"),
    site("http://www.boinc-australia.net", "BOINC@Australia"),
    //site("http://www.boinc-doc.net", "boinc-doc.net"),
    site("http://www.boincuk.com/", "BOINC UK and Team Lookers"),
    //site("http://www.kazlev.karoo.net/", "TeamACC (Arthur C. Clarke fans)"),
    //site("http://www.setiusa.net/", "SETI.USA"),
    site("http://www.boincsynergy.com/", "BOINC Synergy"),
    //site("http://www.esea.dk/esea/boinc.asp", "Earth Space Exploration Agency"),
    site("http://www.tswb.org", "Team Starfire World BOINC"),
    //site("http://www.geocities.com/boinc_volunteers/", "BOINC Volunteers")
));
language("Estonian", array(
    site("http://boinc.tmac.pri.ee", "boinc.tmac.pri.ee"),
    //site("http://setimehed.net/", "setimehed.net"),
));
language("Finnish", array(
    site( "http://www.universe-examiners.org/",
        "Universe Examiners"),
    //site("http://news.universe-examiners.org/asennus/boinc.html",
    //    "BOINC instructions in Finnish"
    //),
));
language("French", array(
    site("http://boinc.starwars-holonet.com/", "Star Wars [FR]"),
    //site("http://wwww.boinc-2tf.org", "2TF Asso"),
    site("http://boincfrance.org", "BOINCFRANCE.ORG"),
    site("http://www.boinc-af.org", "L'Alliance Francophone"),
));
language("German", array(
    site("http://www.crunchers-freiburg.de/", "crunchers@freiburg"),
    //site("http://www.boinc-gemeinschaft.de/", "BOINC Gemeinschaft"),
    site("http://www.gridcommunity.de/index.php", "International Grid Community"),
    site("http://www.swissteam.net/", "SwissTeam.net"),
    site("http://www.unitedmacs.com/", "United Macs"),
    site("http://www.rechenkraft.net/", "Rechenkraft"),
    site("http://www.seti-leipzig.de/", "SETI-Leipzig"),
    site("http://www.dc-gemeinschaft.com/", "DC - Gemeinschaft"),
    site("http://boinccast.podhost.de/", "BOINCcast (Podcast)"),
    site("http://www.boinc-team.de/", "BOINC@Heidelberg"),
    site("http://www.crunching-family.wins.info/", "Crunching Family"),
    site("http://www.boinc.at/", "www.boinc.at"),
    site("http://www.boinc-halle-saale.de", "BOINC@Halle/Saale"),
    site("http://www.bc-team.org/", "BOINC Confederation"),
    site("http://www.boincfun.tk/", "BOINCfun"),
    site("http://www.seti-germany.de", "SETI.Germany"),
    site("http://www.sar-hessen.org", "Team Science and Research Hessen"),
    site("http://www.boinc.de/", "www.boinc.de"),
    //site( "http://www.boinc-lubeca.de/", "BOINC - LUBECA (L&uuml;beck, Germany)"),
    site( "http://www.boinc-forum.de/", "www.boinc-forum.de"),
    //site( "http://www.emuleatboinc.de/board", "Official eMule @ BOINC Team Page")
));
language("Hungarian", array(
    site("http://seti.hwsw.hu/", "HWSW SETI@home Team")
));
language("Italian", array(
    site("http://www.calcolodistribuito.it/", "Calcolo Distribuito"),
    site("http://www.boincitaly.org/", "BOINC.Italy"),
    site("http://gaming.ngi.it/forum/forumdisplay.php?f=73", "NGI forum"),
    site("http://it.groups.yahoo.com/group/BOINC-ITALIA/", "BOINC-ITALIA")
));
language("Japanese", array(
    site(
    "http://boinc.oocp.org/",
        "translation by Komori Hitoshi")
));
language("Korean", array(
    site("http://cafe.naver.com/setikah", "SETIKAH@KOREA"),
    site("http://boincatkorea.xo.st/", "BOINC@KOREA"),
));

language("Polish", array(
    //site("http://www.boinc-polska.org/", "BOINC-Polska.org"),
    //site("http://www.boinc.org.pl/", "Team boinc.pl"),
    site("http://www.boinc.prv.pl", "BOINC@Kolobrzeg"),
    site("http://www.boincatpoland.org", "BOINC@Poland"),
    site("http://boinc.pl", "BOINC Polish National Team"),
    site("http://www.tomaszpawel.republika.pl/", "TomaszPawelTeam"),
    //site("http://www.gpuforce.oxyone.pl/", "GPU Force"),
));
language("Portuguese", array(
    site( "http://portugalathome.pt.vu/", "Portugal@home"),
    site("http://www.setibr.org/", "SETIBR"),
));
language("Romanian", array(
    site( "http://www.boinc.ro/", "SETI@home Romania")
));
language("Russian", array(
    site("http://vkontakte.ru/club11963359", "BOINC group on vkontakte.ru"),
    site("http://www.boinc.ru", "BOINC.ru"),
    site("http://distributed.ru", "distributed.ru")
));
language("Slovak", array(
    site("http://www.boinc.sk/", "www.boinc.sk")
));
language("Spanish", array(
    site("http://www.titanesdc.com/", "Foros TitanesDC"),
    site("http://www.seti.cl/", "BOINC SETI Chile"),
    site("http://www.easyboinc.org/", "Computación Distribuida"),
    site("http://foro.noticias3d.com/vbulletin/showthread.php?t=192297", "Noticias3D"),
    site("http://elmajo.blogspot.com", "Computación Distribuida"),
    site("http://efren-canarias.blogcindario.com/", "El Pais De La Computacion"),
    site("http://www.canalboinc.org/modules/news/", "Canal BOINC"),
    site("http://www.boinc-ecuador.com/", "BOINC - Ecuador"),
    site("http://www.hispaseti.org/", "HispaSeti"),
    site("http://www.seti-argentina.com.ar", "BOINC Argentina"),
    site("http://boinc.blogspot.com", "Boinc y Astronomia")
    //site("http://boincspain.shyper.com/", "BOINC España</a>")
));
language("Turkish", array(
    site("http://www.turksetiteam.org/", "www.turksetiteam.org"),
    site("http://www.boinctr.com/", "www.boinctr.com")
));
language("Ukrainian", array(
    site("http://distributed.org.ua/", "Ukraine - Distributed Computing"),
));

echo "
</table>
<p>
If you'd like to add a web site to this list, please
<a href=mailto:davea@ssl.berkeley.edu>contact us</a>.

<a name=video>
<h2>BOINC-related videos</h2>

<ul>
<li> <a href=http://www.liftconference.com/distributed-computing-distributed-thinking> Francois Grey at Lift, Feb 2008</a>
<li> <a href=http://www.youtube.com/watch?v=8iSRLIK-x6A>David Anderson talks about BOINC</a> (2006)
<li> <a href=http://video.google.com/videoplay?docid=5863868341014543476&hl=en>David Anderson talks at CPDN Open Day (2004)</a> (33 minutes).
<li> <a href=http://www.youtube.com/watch?v=GzATbET3g54>David Baker talks about Rosetta@home</a>
</ul>
";
page_tail();
?>
