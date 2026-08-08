<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

require_once("../inc/util_ops.inc");

function getSingleQuery($query) {
    $result = _mysql_query($query);
    if (!$result) return;
    $cnt = _mysql_fetch_row($result);
    if (!$cnt) return;
    _mysql_free_result($result);
    return $cnt[0];
}

    require_once("../inc/util.inc");
    require_once("../inc/db.inc");
    //require_once("../inc/trickle.inc");
    require_once("../inc/wap.inc");

    // show the home page of app user from envvar

    $valid = $_GET['id'];
    if (!$valid || $valid!="whatever-validation-key-you-want") {
        echo "User id (t.php?id=###) missing!";
        exit(); // can't do much without a userid!
    }

    db_init();

   wap_begin();

    // keep a 'running tab' in wapstr in case exceeds 1K WAP limit

    $wapstr = PROJECT . "<br/>Status Info on<br/>" . wap_timestamp() . "<br/><br/>";

    $wapstr .= "#Users: " . getSingleQuery("select count(*) from user") . "<br/>";
    $wapstr .= "#Hosts: " . getSingleQuery("select count(*) from host") . "<br/>";
    $wapstr .= "#ModYr: " . sprintf("%ld", getSingleQuery("select sum(total_credit)/(.007*17280.0) from host")) . "<br/>";
    $wapstr .= "#Cobbl: " . sprintf("%ld", getSingleQuery("select sum(total_credit) from host")) . "<br/>";
    // I consider a host active if it's trickled in the last week
    //$wapstr .= "#Activ: " . getSingleQuery("select count(distinct hostid) from cpdnexpt.trickle "
    //   . "where trickledate>=" . sprintf("%d", mktime() - (3600*24*7))) . "<br/>";

   // finally get last 5 trickles for everyone
   //$wapstr .= show_trickles("a", 0, 5, 1);

   // limit wap output to 1KB
   if (strlen($wapstr)>1024)
       echo substr($wapstr,0,1024);
   else
       echo $wapstr;

   wap_end();

?>
