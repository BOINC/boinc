<?php
require_once("docutil.php");

function show_link($language, $name, $url) {
    list_item($language, "<a href=$url>$name</a>");
}

page_head("Web sites related to BOINC");
echo "
Information about BOINC is available in the following languages:
";
list_start();
show_link(
    "Chinese",
    "www.equn.com/boinchina",
    "http://www.equn.com/boinchina"
);
show_link(
    "Czech",
    "www.boinc.cz",
    "http://www.boinc.cz/"
);
show_link(
    "Danish",
    "setiboinc.dk",
    "http://setiboinc.dk"
);
show_link(
    "Danish",
    "www.boinc.dk",
    "http://www.boinc.dk"
);
show_link(
    "Danish",
    "www.setihome.dk",
    "http://www.setihome.dk"
);
show_link(
    "Dutch",
    "www.seti-nl.org",
    "http://www.seti-nl.org/content.php?c=boinc_berkeley_main"
);
show_link(
    "Estonian",
    "boinc.tmac.pri.ee",
    "http://boinc.tmac.pri.ee"
);
show_link(
    "Finnish",
    "Universe Examiners",
    "http://www.universe-examiners.org/boinc_faq.php"
);
show_link(
    "French",
    "www.boinc-fr.net",
    "http://www.boinc-fr.net"
);
show_link(
    "French",
    "boinc-quebec.org",
    "http://boinc-quebec.org"
);
show_link(
    "German",
    "www.boinc.de",
    "http://www.boinc.de/"
);
show_link(
    "German",
    "www.boinc-forum.de",
    "http://www.boinc-forum.de/"
);
show_link(
    "German (message board)",
    "Official eMule @ BOINC Team Page",
    "http://www.emuleatboinc.de/Board"
);
//list_item("Italian",
//    "<a href=http://boinc.homeunix.org/>boinc.homeunix.org</a>"
//);
show_link(
    "Japanese",
    "translation by Komori Hitoshi",
    "http://boinc.oocp.org/"
);
show_link(
    "Polish",
    "www.boinc.pl",
    "http://www.boinc.pl"
);
show_link(
    "Russian",
    "www.boinc.narod.ru",
    "http://www.boinc.narod.ru"
);
//list_item("Serbian",
//    "<a href=http://www.boincatserbia.co.sr/>BOINC@Serbia</a>"
//);
show_link(
    "Spanish",
    "BOINC España</a>",
    "http://members.lycos.co.uk/boincspain/"
);
show_link(
    "Turkish",
    "www.turksetiteam.org",
    "http://www.turksetiteam.org/boinc/index.html"
);
list_end();
echo "
If you'd like to add a web site to this list, please
<a href=mailto:davea@ssl.berkeley.edu>contact us</a>.
";
page_tail();
?>
