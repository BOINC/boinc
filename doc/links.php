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
Information for BOINC participants is available in several languages
(also see
 <a href=guis.php>GUIs and add-on software</a>).

";

list_start();
echo "
<tr><th>Language</th><th>Site</th></tr>
";

language("Chinese", array(
    site("http://www.equn.com/boinchina", "www.equn.com/boinchina")
));
language("Czech", array(
    site("http://www.boinc.cz/", "www.boinc.cz")
));
language("Danish", array(
    site("http://setiboinc.dk", "setiboinc.dk"),
    site("http://www.boinc.dk", "www.boinc.dk"),
    site("http://www.setihome.dk", "www.setihome.dk")
));
language(
    "Dutch", array(
    site("http://www.seti-nl.org/content.php?c=boinc_berkeley_main",
        "www.seti-nl.org")
));
//language("English", array(
//    site("http://www.geocities.com/boinc_volunteers/",
//    "BOINC Volunteers")
//));
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
    site(
    "http://www.boinc-fr.net",
        "www.boinc-fr.net"),
    site(
    "http://boinc-quebec.org",
        "boinc-quebec.org")
));
language("German", array(
    site("http://www.boinc.de/", "www.boinc.de"),
    site( "http://www.boinc-lubeca.de/", "BOINC - LUBECA (Lübeck, Germany)"),
    site( "http://www.boinc-forum.de/", "www.boinc-forum.de"),
    site( "http://www.emuleatboinc.de/board", "Official eMule @ BOINC Team Page")
));
//list_item("Italian",
//    "<a href=http://boinc.homeunix.org/>boinc.homeunix.org</a>"
//);
language("Japanese", array(
    site(
    "http://boinc.oocp.org/",
        "translation by Komori Hitoshi")
));
language("Polish", array(
    site( "http://www.boincatpoland.org",
        "www.boincatpoland.org"),
    site( "http://www.boinc.pl",
        "www.boinc.pl")
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
    site("http://boincspain.shyper.com/", "BOINC España</a>")
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
