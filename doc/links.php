<?php
require_once("docutil.php");

function show_link($language, $name, $url) {
    list_item($language, "<a href=$url>$name</a>");
}

function language($lang, $sites) {
    echo "<tr><td bgcolor=eeeeee valign=top>$lang</td><td>\n";
    foreach ($sites as $s) {
        echo "$s<br>\n";
    }
    echo "</td></tr>\n";
}

function site($url, $name) {
    return "<a href=$url>$name</a>";
}

page_head("Web sites for BOINC participants");

echo "
<h2>Project status</h2>
<a href=http://boincprojectstatus.ath.cx/boinc/>BOINC project status</a>:
shows whether the servers of various projects are up or down.
<h2>Statistics</h2>
";
stats_sites();
echo "
<h2>Informational sites</h2>
";
list_start();
echo "
<tr><th>Language</th><th>Site</th></tr>
";

language("Belgium (Dutch & French)", array(
    site("http://www.boinc.be", "www.boinc.be")
));
language("Chinese", array(
    site("http://boinc.equn.com/", "boinc.equn.com")
));
language("Czech", array(
    site("http://www.boincteamcz.net/", "BOINC Team CZ"),
    site("http://www.boinc.cz/", "www.boinc.cz")
));
language("Danish", array(
    site("http://setiboinc.dk", "setiboinc.dk"),
    site("http://www.boinc.dk", "www.boinc.dk"),
    site("http://www.setihome.dk", "www.setihome.dk")
));
language("Dutch", array(
    site("http://www.seti.nl/content.php?c=boincmain",
        "SETI@Netherlands"
    )
));
language("English", array(
    site("http://www.boinc-doc.net", "boinc-doc.net"),
    site("http://www.boincsynergy.com/", "BOINC Synergy"),
    site("http://www.esea.dk/esea/boinc.asp", "Earth Space Exploration Agency"),
    site("http://boinc.mundayweb.com", "boinc.mundayweb.com - stats counters and more")
    //site("http://www.geocities.com/boinc_volunteers/", "BOINC Volunteers")
));
language("Estonian", array(
    site("http://boinc.tmac.pri.ee", "boinc.tmac.pri.ee")
));
language("Finnish", array(
    site( "http://www.universe-examiners.org/boinc_faq.php",
        "Universe Examiners"),
    site(
    "http://news.universe-examiners.org/asennus/boinc.html",
        "BOINC instructions in Finnish")
));
language("French", array(
    site("http://boincfrance.org", "BOINCFRANCE.ORG"),
    site("http://www.boinc-fr.net", "www.boinc-fr.net"),
    site("http://boinc-quebec.org", "boinc-quebec.org")
));
language("German", array(
    site("http://www.crunching-family.wins.info/", "Crunching Family"),
    site("http://www.boinc.at/", "www.boinc.at"),
    site("http://www.boinc-halle-saale.de", "BOINC@Halle/Saale"),
    site("http://www.boincfun.tk/", "BOINCfun"),
    site("http://www.setigermany.de", "SETI.Germany"),
    site("http://www.sar-hessen.de", "Team Science and Research Hessen"),
    site("http://www.boinc.de/", "www.boinc.de"),
    site( "http://www.boinc-lubeca.de/", "BOINC - LUBECA (L&uuml;beck, Germany)"),
    site( "http://www.boinc-forum.de/", "www.boinc-forum.de"),
    site( "http://www.emuleatboinc.de/board", "Official eMule @ BOINC Team Page")
));
language("Hungarian", array(
    site("http://seti.hwsw.hu/", "HWSW SETI@home Team")
));
language("Italian", array(
    site("http://it.groups.yahoo.com/group/BOINC-ITALIA/", "BOINC-ITALIA")
));
language("Japanese", array(
    site(
    "http://boinc.oocp.org/",
        "translation by Komori Hitoshi")
));
language("Polish", array(
    site( "http://www.boincatpoland.org", "BOINC@Poland"),
    site( "http://www.boinc.pl", "www.boinc.pl")
));
language("Portuguese", array(
    site( "http://portugalathome.pt.vu/", "Portugal@home")
));
language("Russian", array(
    site("http://www.boinc.narod.ru", "www.boinc.narod.ru")
));
//show_link(
//    "Serbian",
//    "BOINC@Serbia",
//    "http://www.boincatserbia.co.sr/"
//);
language("Slovak", array(
    site("http://www.boinc.sk/", "www.boinc.sk")
));
language("Spanish", array(
    site("http://www.boinc-ecuador.com/", "BOINC - Ecuador"),
    site("http://www.hispaseti.org/", "HispaSeti"),
    site("http://www.seti-argentina.com.ar", "BOINC Argentina"),
    site("http://boinc.blogspot.com", "Boinc y Astronomia")
    //site("http://boincspain.shyper.com/", "BOINC España</a>")
));
language("Turkish", array(
    site("http://www.turksetiteam.org/boinc/index.html",
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
