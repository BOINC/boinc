<?php

require_once("docutil.php");

include("../html/inc/stats_sites.inc");

function language($lang, $sites) {
    echo "<tr><td bgcolor=eeeeee valign=top>$lang</td><td>\n";
    shuffle($sites);
    foreach ($sites as $s) {
        echo "$s<br>\n";
    }
    echo "</td></tr>\n";
}

function site($url, $name) {
    return "<a href=$url>$name</a>";
}

$wiki_sites = array(
    array(
        "http://www.kazlev.karoo.net/noob_help.htm",
        "BOINC mini-FAQ"
    ),
    array(
        "http://boincfaq.mundayweb.com/",
        "The BOINC FAQ Service"
    ),
    array(
        "http://boinc-wiki.ath.cx/",
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
        "http://www.boincfrance.org/wakka.php?wiki=BienVenue",
        "BOINCFrance.org",
        "(in French)",
    ),
    array(
        "http://www.crunching-family.at/wiki/",
        "Crunching Family Wiki",
        "(In German)",
    ),
);

page_head("Web resources for BOINC participants");

echo "
<h3>Contents</h3>
<ul>
<li> <a href=#info>Help and Information</a>
<li> <a href=#stats>Credit statistics</a>
<li> <a href=#sigs>Credit-based signatures</a>
<li> <a href=#team_stats>Team statistics</a>
<li> <a href=#status>Project status</a>
<li> <a href=#misc>Miscellaneous</a>
<li> <a href=#skins>Skins for the BOINC Manager</a>
<li> <a href=#sites>Other BOINC-related sites</a>
(Information, message boards, and teams)
</ul>
<a name=wiki></a>
<h3>Help and Information</h3>
Sites with information and documentation about BOINC.
";
shuffle($wiki_sites);
site_list($wiki_sites);
echo "
<a name=stats></a>
<h3>Credit statistics</h3>
<p>
The following web sites show statistics for one or more BOINC projects.
These sites use XML-format data exported by BOINC projects.
The format is described
<a href=http://boinc.berkeley.edu/stats.php>here</a>.
If you're interested in running your own site or
participating in the development efforts,
please contact the people listed below.
";
shuffle($stats_sites);
site_list($stats_sites);
echo "
<a name=sigs></a>
<h3>Statistics signature images</h3>
<p>
The following sites offer dynamically-generated
images showing your statistics in BOINC projects.
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
echo "

<a name=status></a>
<h3>Project status sites</h3>
Show if the servers of various projects are up or down.
<ul>
<li> <a href=http://www.esea.dk/esea/bos.asp>BOS (BOINC Online Schedulers></a>
<li> <a href=http://boincprojectstatus.ath.cx/>BOINC Project Status</a>
</ul>
<a name=misc></a>
<h3>Miscellaneous</h3>

<ul>
<li> <a href=http://www.myboinc.com/>BOINC Users of the Day</a>
<li> <a href=http://groups.myspace.com/BOINConMYSPACE>BOINC on MySpace</a>
</ul>
<a name=skins></a>
<h3>Skins for the BOINC Manager</h3>
<ul>
<li> <a href=http://www.crunching-family.at/download-center/>Crunching Family Skin Download</a>
</ul>
<a name=sites></a>
<h3>Other BOINC-related web sites</h3>
";
list_start();
echo "
<tr><th>Language</th><th>Site</th></tr>
";

language("Belgium (Dutch/French/English)", array(
    site("http://www.boinc.be", "www.boinc.be")
));
language("Catalan", array(
    site("http://www.boinc.cat", "BOINC.cat"),
));
language("Chinese", array(
    site("http://boinc.equn.com/", "boinc.equn.com")
));
language("Czech", array(
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
    site("http://www.teamphoenixrising.net/", "Team Phoenix Rising"),
    site("http://www.unitedmacs.com/", "United Macs"),
    site("http://sirans-boincnews.com/", "Siran's BOINC Projects News Site"),
    site("http://www.ukboincteam.org.uk/", "UK BOINC Team"),
    site("http://symbion.madnezz.com/", "Symbion"),
    site("http://scotlandsseti.blogspot.com/", "Megacruncher's Blog"),
    site("http://www.bc-team.org/", "BOINC Confederation"),
    site("http://theclangers.net/", "The Clangers"),
    site("http://www.free-dc.org/", "Free-DC"),
    site("http://forums.anandtech.com/categories.aspx?catid=39&entercat=y", "TeAm Anandtech"),
    site("http://www.boinc-australia.net", "BOINC@Australia"),
    site("http://www.boinc-doc.net", "boinc-doc.net"),
    site("http://www.boincuk.com/", "BOINC UK and Team Lookers"),
    site("http://www.kazlev.karoo.net/", "TeamACC (Arthur C. Clarke fans)"),
    site("http://www.setiusa.net/", "SETI.USA"),
    site("http://www.boincsynergy.com/", "BOINC Synergy"),
    site("http://www.esea.dk/esea/boinc.asp", "Earth Space Exploration Agency"),
    site("http://www.tswb.org", "Team Starfire World BOINC"),
    //site("http://www.geocities.com/boinc_volunteers/", "BOINC Volunteers")
));
language("Estonian", array(
    site("http://boinc.tmac.pri.ee", "boinc.tmac.pri.ee"),
    site("http://setimehed.net/", "setimehed.net"),
));
language("Finnish", array(
    site( "http://www.universe-examiners.org/",
        "Universe Examiners"),
    site(
    "http://news.universe-examiners.org/asennus/boinc.html",
        "BOINC instructions in Finnish")
));
language("French", array(
    site("http://boincfrance.org", "BOINCFRANCE.ORG"),
    site("http://www.boinc-af.org", "L'Alliance Francophone"),
    site("http://boinc-quebec.org", "boinc-quebec.org")
));
language("German", array(
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
    site("http://www.setigermany.de", "SETI.Germany"),
    site("http://www.sar-hessen.org", "Team Science and Research Hessen"),
    site("http://www.boinc.de/", "www.boinc.de"),
    site( "http://www.boinc-lubeca.de/", "BOINC - LUBECA (L&uuml;beck, Germany)"),
    site( "http://www.boinc-forum.de/", "www.boinc-forum.de"),
    site( "http://www.emuleatboinc.de/board", "Official eMule @ BOINC Team Page")
));
language("Hungarian", array(
    site("http://seti.hwsw.hu/", "HWSW SETI@home Team")
));
language("Italian", array(
    site("http://gaming.ngi.it/forum/forumdisplay.php?f=73", "NGI forum"),
    site("http://it.groups.yahoo.com/group/BOINC-ITALIA/", "BOINC-ITALIA")
));
language("Japanese", array(
    site(
    "http://boinc.oocp.org/",
        "translation by Komori Hitoshi")
));
language("Korean", array(
    site("http://boincatkorea.xo.st/", "BOINC@KOREA"),
));

language("Polish", array(
    site("http://www.boinc.org.pl/", "Team boinc.pl"),
    site("http://www.boinc.prv.pl", "BOINC@Kolobrzeg"),
    site("http://www.boincatpoland.org", "BOINC@Poland"),
    site("http://www.boinc.pl", "www.boinc.pl")
));
language("Portuguese", array(
    site( "http://portugalathome.pt.vu/", "Portugal@home")
));
language("Romanian", array(
    site( "http://www.boinc.ro/", "SETI@home Romania")
));
language("Russian", array(
    site("http://www.boinc.ru", "BOINC.ru"),
    site("http://distributed.ru", "distributed.ru")
));
language("Slovak", array(
    site("http://www.boinc.sk/", "www.boinc.sk")
));
language("Spanish", array(
    site("http://efren-canarias.blogcindario.com/", "El Pais De La Computacion"),
    site("http://www.canalboinc.org/modules/news/", "Canal BOINC"),
    site("http://www.boinc-ecuador.com/", "BOINC - Ecuador"),
    site("http://www.hispaseti.org/", "HispaSeti"),
    site("http://www.seti-argentina.com.ar", "BOINC Argentina"),
    site("http://boinc.blogspot.com", "Boinc y Astronomia")
    //site("http://boincspain.shyper.com/", "BOINC España</a>")
));
language("Turkish", array(
    site("http://www.turksetiteam.org/",
        "www.turksetiteam.org")
));
echo "
</table>
<p>
If you'd like to add a web site to this list, please
<a href=mailto:davea@ssl.berkeley.edu>contact us</a>.
";
page_tail();
?>
