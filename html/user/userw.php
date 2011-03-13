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
    
    $cache = unserialize(get_cached_data(USER_PAGE_TTL, "userid=".$userid));
    if (!$cache) {
        $cache = new stdClass;
        $cache->user = BoincUser::lookup_id($userid);
        if ($cache->user->teamid) {
            $cache->team = BoincTeam::lookup_id($cache->user->teamid);
        }
        $cache->wap_timestamp = wap_timestamp();
        set_cached_data(USER_PAGE_TTL, serialize($cache), "userid=".$userid);
    }
    
    if (!$cache->user) {
        echo "<br/>".tra("User not found!")."<br/>";
        wap_end();
        return;
    }
    
    // keep a 'running tab' in wapstr in case exceeds 1K WAP limit
    $wapstr = PROJECT."<br/>".tra("Account Data<br/>for %1<br/>Time:", $cache->user->name)." ".$cache->wap_timestamp;
    $wapstr .= show_credit_wap($cache->user);
    if ($cache->user->teamid) {
        $wapstr .= "<br/>".tra("Team:")." ".$cache->team->name."<br/>";
        $wapstr .= tra("Team TotCred:")." " . format_credit($cache->team->total_credit) . "<br/>";
        $wapstr .= tra("Team AvgCred:")." " . format_credit($cache->team->expavg_credit) . "<br/>";
    } else {
        $wapstr .= "<br/>".tra("Team: None")."<br/>";
    }
    
    // don't want to send more than 1KB probably?
    if (strlen($wapstr) > 1024) {
        $wapstr = substr($wapstr, 0, 1024);
    }
    
    echo $wapstr;
    wap_end();
}

show_user_wap(get_int('id'));

?>
