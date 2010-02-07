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

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");

// We have indices on id, name, total credit, and RAC.
// So we provide the following searches:
// - new users in last N days (1, 7)
// - users with names starting with X
// - users ordered by decreasing total credit
// - users ordered by decreasing RAC
//
// In addition you can filter by:
// - a given country
// - with/without profile
// - with/without team
//
// We don't do the filtering in the SQL, because that would
// lead to table scans in the worst case.
// Instead we use the following policies:
// - a given search can scan no more than MAX_ROWS rows
// - a given search can return no more than MAX_RESULTS results
//
// here's how we do this:
// result_list = empty
// while n<MAX_ROWS
//   enumerate rows w/o filtering, limit n, 1000
//      (read only the fields we need for display)
//   scan through batch, doing filtering, appending to result_list


function filter_user($user, $filter) {
    if ($filter->do_country && $user->country!=$filter->country) return false;
    if ($filter->do_profile) {
        if ($user->has_profile and !$filter->has_profile) return false;
        if (!$user->has_profile and $filter->has_profile) return false;
    }
    if ($filter->do_team) {
        if ($filter->team xor $user->teamid!=0) return false;
    }
    return true;
}

function show_user($user) {
    echo "
        <tr class=row1>
        <td>", $user->id, user_links($user), "</td>
    ";
    if ($user->teamid) {
        $team = BoincTeam::lookup_id($user->teamid);
        echo "
            <td> <a href=team_display.php?teamid=$team->id>$team->name</a> </td>
        ";
    } else {
        echo "<td><br></td>";
    }
    echo "
        <td align=right>", format_credit($user->expavg_credit), "</td>
        <td align=right>", format_credit_large($user->total_credit), "</td>
        <td>", $user->country, "</td>
        <td>", time_str($user->create_time),"</td>
        </tr>
    ";
}

function do_search($order, $filter) {
    $filtered_list = array();
    $nrows_scanned = 0;
    $fields = "id, create_time, name, country, total_credit, expavg_credit, teamid, url, has_profile, donated";
    while (1) {
        if (count($filtered_list) > 500) break;
        $limit_clause = " limit $nrows_scanned, 1000";
        $users = BoincUser::enum_fields($fields, null, $order.$limit_clause);
        $n = count($users);
        $nrows_scanned += $n;
        if ($nrows_scanned > 10000) break;
        if ($n==0) break;
        foreach($users as $user) {
            if (filter_user($user, $filter)) {
                $filtered_list[] = $user;
            }
        }
    }
    start_table();
    table_header(tra("Name"), tra("Team"), tra("Average credit"), tra("Total credit"), tra("Country"), tra("Joined"));
    foreach ($filtered_list as $user) {
        show_user($user);
    }
    end_table();
}

function search_form() {
    page_head("User search");
    echo "<form name=f method=get action=user_search.php>";
    start_table();
    row1(tra("Search type"), 2, "heading");
    row2(tra("User name starts with")." <input type=text name=search_string>", "<input type=radio name=search_type value=\"name_prefix\" checked >");
    row2(tra("Decreasing sign-up time"), "<input type=radio name=search_type value=\"date\">");
    row2(tra("Decreasing average credit"), "<input type=radio name=search_type value=\"rac\">");
    row2(tra("Decreasing total credit"), "<input type=radio name=search_type value=\"total\">");
    row1(tra("Filters"), 2, "heading");
    row2_init(tra("Country"), "<select name=country><option value=\"any\" selected>".tra("Any")."</option>");
    print_country_select("asdf");
    echo "</select></td></tr>";
    row2(tra("With profile?"),
        "<input type=radio name=profile value=either checked=1> ".tra("Either")."
        <input type=radio name=profile value=no> ".tra("No")."
        <input type=radio name=profile value=yes> ".tra("Yes")."
    ");
    row2(tra("On a team?"),
        "<input type=radio name=team value=either checked=1> ".tra("Either")."
        <input type=radio name=team value=no> ".tra("No")."
        <input type=radio name=team value=yes> ".tra("Yes")."
    ");
    row2("", "<input type=submit name=action value=".tra("Search").">");
    end_table();
    echo "
        <script>document.f.search_string.focus()</script>
    ";
            
    page_tail();
}

function name_search($filter) {
    $count = 100;
    $search_string = get_str('search_string');

    if (strlen($search_string)<3) {
        error_page(tra("search string must be at least 3 characters"));
    }
    $s = boinc_real_escape_string($search_string);
    $s = escape_pattern($s);
    $fields = "id, create_time, name, country, total_credit, expavg_credit, teamid, url, has_profile, donated";
    $users = BoincUser::enum_fields($fields, "name like '$s%'", "limit $count");
    $n=0;
    foreach ($users as $user) {
        if (!filter_user($user, $filter)) continue;
        if ($n==0) {
            echo "<h3>".tra("User names starting with")." '".htmlspecialchars($search_string)."'</h3>\n";
            start_table();
            table_header(tra("Name"), tra("Team"), tra("Average credit"), tra("Total credit"), tra("Country"), tra("Joined"));

        }
        show_user($user);
        $n++;
    }
    end_table();
    if (!$n) {
        echo tra("No users match your search criteria.");
    }
}

function main() {
    $search_type = get_str('search_type', true);
    if ($search_type) {
        switch ($search_type) {
        case 'date':
            $order = 'order by id desc';
            break;
        case 'rac':
            $order = 'order by expavg_credit desc';
            break;
        case 'total':
            $order = 'order by total_credit desc';
            break;
        case 'name_prefix':
            break;
        default:
            error_page(tra("missing search type"));
        }

        $filter = null;
        $filter->do_country = false;
        $filter->do_profile = false;
        $filter->do_team = false;
        $country = get_str('country');
        if ($country != 'any') {
            $filter->do_country = true;
            $filter->country = $country;
        }
        switch (get_str('profile')) {
        case 'yes':
            $filter->do_profile = true;
            $filter->has_profile = true;
            break;
        case 'no':
            $filter->do_profile = true;
            $filter->has_profile = false;
            break;
        case 'either':
            $filter->do_profile = false;
            break;
        }
        switch (get_str('team')) {
        case 'yes':
            $filter->do_team = true;
            $filter->team = true;
            break;
        case 'no':
            $filter->do_team = true;
            $filter->team = false;
            break;
        case 'either':
            $filter->do_team = false;
            break;
        }
        page_head(tra("User search results"));
        if ($search_type == 'name_prefix') {
            name_search($filter);
        } else {
            do_search($order, $filter);
        }
        page_tail();
    } else {
        search_form();
    }
}

main();

$cvs_version_tracker[]="\$Id: user_search.php 13586 2007-09-13 09:46:36Z Rytis $";  //Generated automatically - do not edit
?>
