<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// the following sites generate stats for a given CPID
//
global $cpid_stats_sites;
$cpid_stats_sites = array(
    array("Free-DC",
        "http://stats.free-dc.org/stats.php?page=userbycpid&amp;cpid=%s"
    ),
    array("BOINCstats",
        "http://boincstats.com/en/stats/-1/user/detail/%s"
    ),
    array("BOINC Combined Statistics",
        "http://boinc.netsoft-online.com/e107_plugins/boinc/get_user.php?cpid=%s&amp;html=1"
    ),
);

$stats_sites = array(
    array(
        "http://www.formula-boinc.org/",
        "Formula BOINC",
        "(Team ranking based on position within projects,
        as in Formula 1 racing, rather than total points)"
    ),
    array(
        "http://stats.free-dc.org",
        "Free-DC",
        "(Includes non-BOINC projects and completed projects.)"
    ),
/* page shows: Could not connect to the data server. Unable to proceed.
 * not mentioning statistics on the webpage, seems retired
    array(
        "http://www.boincuk.com/globalprojectstats.php",
        "BOINC UK",
        ""
    ),
*/
    array("http://boinc.netsoft-online.com/",
        "BOINC Combined Statistics",
        "developed by James Drews"
    ),
    array("http://boincstats.com/",
        "BOINCstats",
        "by Willy de Zutter"
    ),
    array("http://www.dc-vault.com",
        "DC-Vault",
        "(Include non-BOINC projects)"
    ),
    array("http://www.seti.nl/boinc_team.php",
        "SETI@Netherlands stats page",
        ""
    ),
/* page shows: This website has been permanently closed
    array("http://www.teamocuk.co.uk/",
        "Team OcUK stats",
        ""
    ),
*/
    array("http://statsbzh.boinc-af.org/listteams.php",
        "SETIBZH Stats",
        "(cross-project team stats; available in French, English, Spanish, German)"
    ),
);

$team_stats_sites = array(
    array("http://stats.czechnationalteam.cz/", "Czech National Team", "(in Czech)"),
    array("http://www.boincitaly.org/", "BOINC.Italy"),
    array("https://www.crunchersansfrontieres.org/", "CRUNCHERS SANS FRONTIERES", "(French"),
    //array("http://www.spacepage.be/component/option,com_boinc/", "Spacepage"),
    //array("http://boinc.radax.net/de_boinc.htm", "BOINC@Austria"),
    //array("http://www.myboinc.com/scores/", "L'Alliance Francophone"),
    //array("http://boincdenmark.dk/", "BOINC@Denmark", "(Danish)"),
    //array("http://boincdenmark.dk/default_en.html", "BOINC@Denmark", "(English)"),
    //array("http://www.bigbee.be/comp/boinc/index.php",
    //    "Boinc.be team stats",
    //    ""
    //),
    //array("http://www.seti-teamartbell.com/", "Team Art Bell", ""),
    array("http://www.crunchers-freiburg.de/", "crunchers@freiburg", "(German)"),
);

// The following sites generate signature images based on user CPID
//
$sig_sites = array(
    array(
        "http://www.boincuk.com/show3.php",
        "BOINC UK",
        "(Highly configurable!)"
    ),
    array("http://boinc.mundayweb.com/",
        "http://boinc.mundayweb.com",
        "(User-configurable stats counters. Cool!)"
    ),
    array("http://boincstats.com/page/faq.php#3",
        "BOINCstats",
        "by Willy de Zutter"
    ),
);

// the following sites generate cross-project team stats based on CPID
//
$team_name_sites = array(
    array("http://stats.free-dc.org/stats.php?page=teambycpid&team=",
        "Free-DC",
        ""
    ),
    array("http://boincstats.com/en/stats/-1/team/detail/",
        "BOINCstats.com",
        "hashlc"
    ),
    array("http://statsbzh.boinc-af.org/team.php?teamcpid=",
        "SETIBZH",
        "hashlc"
    ),
);

global $host_sites;
$host_sites = array(
    array(
        "http://boincstats.com/en/stats/-1/host/detail/",
        "BOINCstats.com",
        "boincstats_icon.png"
    ),
    array(
        "http://stats.free-dc.org/stats.php?page=hostbycpid&cpid=",
        "Free-DC",
        "freedc_icon.png"
    ),
);

function site_list($sites) {
    echo "<ul>\n";
    for ($i=0; $i<count($sites); $i++) {
        $s = $sites[$i];
        $url = $s[0];
        $name = $s[1];
        $comment = array_key_exists(2, $s)?$s[2]:"";
        echo "<li><a href=\"$url\">$name</a> $comment\n";
    }
    echo "</ul>\n";
}

$GLOBALS['cpid_stats_sites'] = $cpid_stats_sites;
$GLOBALS['host_sites'] = $host_sites;

?>
