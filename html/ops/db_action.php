<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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
require_once("../inc/db_ops.inc");

db_init();

$detail = null;
$show_aggregate = false;

$nresults = get_int("nresults", true);
$entries_to_show = get_int("entries_to_show", true);
$last_pos = get_int("last_pos", true);
$table = get_str("table", true);
$detail = get_str("detail", true);
$clauses = get_str("clauses", true);
if ($clauses && strstr($clauses, ";")) admin_error_page("bad clause");

$q = new SqlQueryString();
$q->process_form_items();

if (isset($nresults)) {
    $entries_to_show = $nresults;
} else {
    $entries_to_show = 20;
}
$page_entries_to_show = $entries_to_show;

if (isset($last_pos)) {
    $start_at = $last_pos;
} else {
    $start_at = 0;
}

$title = table_title($table);
admin_page_head($title);

$count = $q->count();

if ($count < $start_at + $entries_to_show) {
    $entries_to_show = $count - $start_at;
}

$last = $start_at + $entries_to_show;

$main_query = $q->get_select_query($entries_to_show, $start_at);

// For display, convert query string characters < and > into 'html form' so
// that they will be displayed.
//
$html_text=str_replace('<', '&lt;', str_replace('>', '&gt;', $main_query));

echo "<p>Query: <b>$html_text</b><p>\n";

$start_1_offset = $start_at + 1;
echo "
    <p>$count records match the query.
    Displaying $start_1_offset to $last.<p>
";

$url = $q->get_url("db_action.php");
if ($detail) {
    $url .= "&detail=$detail";
}

//echo "<hr>$url<hr><br>\n";
if ($start_at || $last < $count) {
    if ($start_at) {
        $prev_pos = $start_at - $page_entries_to_show;
        if ($prev_pos < 0) {
            $prev_pos = 0;
        }
        echo "
            <a href=\"$url&last_pos=$prev_pos&nresults=$page_entries_to_show\">Previous $page_entries_to_show</a>
        ";
    } else {
        echo "---";
    }
    if ($last < $count) {
        echo "
            | <a href=\"$url&last_pos=$last&nresults=$page_entries_to_show\">Next $page_entries_to_show</a>
            <p>
        ";
    }
}

if ($table == "result") {
    $url = $q->get_url("result_summary.php");
    echo "<a href=\"$url\">Summary</a> |";
}
if ($detail == "high") {
    $url = $q->get_url("db_action.php")."&detail=low";
    echo "
        <a href=\"$url\">Less detail</a>
    ";
}
if ($detail == "low") {
    $url = $q->get_url("db_action.php")."&detail=high";
    echo "
        <a href=\"$url\">More detail</a>
    ";
}

echo " | <a href=\"index.php\">Return to main admin page</a>\n";
echo "<p>\n";
if ($table == "host") {
    if ($show_aggregate) {
        $query = "select sum(d_total) as tot_sum, sum(d_free) as free_sum, sum(m_nbytes) as tot_mem from host";
        if ($clauses) {
            $query .= " WHERE $clauses";
        }
        $result = _mysql_query($query);
        $disk_info = _mysql_fetch_object($result);
        $dt = $disk_info->tot_sum/(1024*1024*1024);
        $df = $disk_info->free_sum/(1024*1024*1024);
        $mt = $disk_info->tot_mem/(1024*1024);
        $dt = round($dt, 2);
        $df = round($df, 2);
        $mt = round($mt, 2);
        echo "<p>\n
            <table border=0>
            <tr><td>
            Sum of total disk space on these hosts:
            </td><td align=right>
            $dt GB
            </td></tr>
            <tr><td>
            Sum of available disk space on these hosts:
            </td><td align=right>
            $df GB
            </td></tr>
            <tr><td>
            Sum of memory on these hosts:
            </td><td align=right>
            $mt MB
            </td></tr>
            </table><p>
        ";
    }
}

$result = _mysql_query($main_query);
if ($result) {
    if ($detail == "low") {
        start_table('table-striped');
        switch($table) {
        case "result":
            result_short_header();
            break;
        case "host":
            host_short_header();
            break;
        case "app_version":
            app_version_short_header();
            break;
        case "workunit":
            workunit_short_header();
            break;
        }
    }
    while ($res = _mysql_fetch_object($result)) {
        if ($detail == "low") {
            switch ($table) {
            case "result":
                admin_show_result_short($res);
                break;
            case "host":
                admin_show_host_short($res);
                break;
            case "app_version":
                admin_show_app_version_short($res);
                break;
            case "workunit":
                admin_show_workunit_short($res);
                break;
            }
        } else {
            switch ($table) {
            case "platform":
                admin_show_platform($res);
                break;
            case "app":
                admin_show_app($res);
                break;
            case "app_version":
                admin_show_app_version($res);
                break;
            case "host":
                admin_show_host($res);
                break;
            case "workunit":
                admin_show_workunit($res);
                break;
            case "result":
                admin_show_result($res);
                break;
            case "team":
                admin_show_team($res);
                break;
            case "user":
                admin_show_user($res);
                break;
            }
        }
    }
    if ($detail == "low" || $table == "profile") {
         end_table();
    }
    _mysql_free_result($result);
} else {
    echo "<h2>No results found</h2>";
}

admin_page_tail();
?>
