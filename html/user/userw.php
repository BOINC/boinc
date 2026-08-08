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

require_once("../inc/util.inc");
require_once("../inc/wap.inc");

check_get_args(array("id"));

function show_credit_wap($user) {
    $retstr = "<br/>User TotCred: " . format_credit($user->total_credit) . "<br/>";
    $retstr .= "User AvgCred: " . format_credit($user->expavg_credit) . "<br/>";
    return $retstr;
}

function show_user_wap($userid) {
    wap_begin();

    $user = BoincUser::lookup_id($userid);
    if (!$user) {
        echo "<br/>".tra("User not found!")."<br/>";
        wap_end();
        return;
    }

    if ($user->teamid) {
        $team = BoincTeam::lookup_id($user->teamid);
    }

    $wapstr = PROJECT."<br/>".tra("Account Data<br/>for %1<br/>Time:", $user->name)." ".wap_timestamp();
    $wapstr .= show_credit_wap($user);
    if ($user->teamid && $team) {
        $wapstr .= "<br/>".tra("Team:")." ".$team->name."<br/>";
        $wapstr .= tra("Team TotCred:")." " . format_credit($team->total_credit) . "<br/>";
        $wapstr .= tra("Team AvgCred:")." " . format_credit($team->expavg_credit) . "<br/>";
    } else {
        $wapstr .= "<br/>".tra("Team: None")."<br/>";
    }

    // don't want to send more than 1KB (WAP limit)
    //
    if (strlen($wapstr) > 1024) {
        $wapstr = substr($wapstr, 0, 1024);
    }

    echo $wapstr;
    wap_end();
}

show_user_wap(get_int('id'));

?>
