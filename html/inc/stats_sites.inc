<?php

// the following sites generate stats for a given CPID
//
$cpid_stats_sites = array(
    array("BOINCStats",
        "http://www.boincstats.com/stats/boinc_user_graph.php?pr=bo&id=%s"
    ),
    array("BOINC Statistics for the WORLD!",
        "http://www.boincsynergy.com/stats/boinc-individual.php?cpid=%s"
    ),
    array("BOINC Combined Statistics",
        "http://boinc.netsoft-online.com/get_user.php?cpid=%s&html=1"
    ),
    array("The Knights Who Say 'Ni!'",
        "http://stats.kwsn.net/user.php?proj=all&cpid=%s"
    ),
);

$stats_sites = array(
    array(
        "http://www.boincuk.com/globalprojectstats.php",
        "BOINC UK",
        ""
    ),
    array("http://www.setisynergy.com/stats/index.php",
        "BOINC Statistics for the WORLD!",
        "developed by Zain Upton (email: zain.upton at setisynergy.com)"
    ),
    array("http://boinc.netsoft-online.com/",
        "BOINC Combined Statistics",
        "developed by James Drews"
    ),
    array("http://home.btconnect.com/Gabys_Bazar/hwupgrade.html",
        "Every Earthly Hour",
        "developed by Hydnum Repandum."
    ),
    array("http://www.boincstats.com/",
        "BOINCStats",
        "by Willy de Zutter"
    ),
    array("http://www.dc-vault.com",
        "DC-Vault",
        "Include non-BOINC projects"
    ),
    array("http://www.seti.nl/boinc_team.php",
        "SETI@Netherlands stats page",
        ""
    ),
    array("http://www.boinc.dk/index.php?page=statistics",
        "BOINC.dk",
        "developed by Janus Kristensen (email: stats at boinc.dk)."
    ),
    //array("http://stats.boincbzh.net/BZHwds/index.php",
    //    "BOINC Alliance Francophone",
    //    "developed by Vincent Mary (email: stats at hoincbzh.net).
    //    Supports competition between 'mini-teams'."
    //),
    array("http://stats.kwsn.net/",
        "The Knights Who Say 'Ni' stats",
        ""
    ),
    array("http://www.teamocuk.com/",
        "Team OcUK stats",
        ""
    ),
);

$team_stats_sites = array(
    array("http://boincdenmark.dk/", "BOINC@Denmark", "(Danish)"),
    array("http://boincdenmark.dk/default_en.html", "BOINC@Denmark", "(English)"),
    array("http://www.bigbee.be/comp/boinc/index.php",
        "Boinc.be team stats",
        ""
    ),
);

$sig_sites = array(
    array(
        "http://www.boincuk.com/show3.php",
        "BOINC UK",
        "Highly configurable!"
    ),
    array("http://boinc.mundayweb.com/",
        "http://boinc.mundayweb.com",
        "user-configurable stats counters. Cool!"
    ),
    array("http://www.boinc.dk/index.php?page=signatures",
        "http://www.boinc.dk/index.php?page=signatures",
        ""
    ),
    array("http://www.bigbee.be/comp/boinc/signature.php?id=9",
        "http://www.bigbee.be/comp/boinc/signature.php",
        ""
    ),
);

function site_list($sites) {
    echo "<ul>\n";
    for ($i=0; $i<count($sites); $i++) {
        $s = $sites[$i];
        $url = $s[0];
        $name = $s[1];
        $comment = $s[2];
        echo "<li> <a href=$url>$name</a> $comment\n";
    }
    echo "</ul>\n";
}

?>
