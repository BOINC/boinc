<?php
require_once("docutil.php");
page_head("Web sites related to BOINC");
echo "
Information about BOINC is available in the following languages:
";
list_start();
list_item("Chinese",
    "<a href=http://www.equn.com/boinchina>www.equn.com/boinchina</a>"
);
list_item("Estonian",
    "<a href=http://boinc.tmac.pri.ee>boinc.tmac.pri.ee</a>"
);
list_item("Danish",
    "<a href=http://setiboinc.dk>setiboinc.dk</a>
    <br><a href=http://www.boinc.dk>www.boinc.dk</a>
    <br><a href=http://www.setihome.dk>www.setihome.dk</a>"
);
list_item("Dutch",
    "<a href=http://www.seti-nl.org/content.php?c=boinc_berkeley_main>www.seti-nl.org</a>"
);
list_item("Finnish",
    "<a href=http://www.universe-examiners.org/boinc_faq.php>Universe Examiners</a>"
);
list_item("French",
    "<a href=http://www.boinc-fr.net>www.boinc-fr.net</a>
    <br><a href=http://boinc-quebec.org>boinc-quebec.org</a> (Canadian)"
);
list_item("German",
    "<a href=http://www.boinc.de/>www.boinc.de</a>"
);
list_item("Italian",
    "<a href=http://boinc.homeunix.org/>boinc.homeunix.org</a>"
);
list_item("Japanese",
    "<a href=http://boinc.oocp.org/>translation by Komori Hitoshi</a>"
);
list_item("Russian",
    "<a href=http://www.boinc.narod.ru>www.boinc.narod.ru</a>"
);
list_item("Serbian",
    "<a href=http://www.boincatserbia.co.sr/>BOINC@Serbia</a>"
);
list_item("Spanish",
    "<a href=http://members.lycos.co.uk/boincspain/>BOINC España</a>"
);
list_item("Turkish",
    "<a href=http://www.turksetiteam.org/boinc/index.html>www.turksetiteam.org</a>"
);
list_end();
echo "
If you'd like to add a web site to this list, please
<a href=mailto:davea@ssl.berkeley.edu>contact us</a>.
";
page_tail();
?>
