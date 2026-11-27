<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

// show a breakdown of hosts by CPU type and OS
//
// This counts hosts w/ recent credit, grouped in SQL by CPU vendor
// and OS fields, so memory use depends on the number of distinct
// combinations rather than the number of hosts.
// The result is cached for a day.

require_once('../inc/boinc_db.inc');
require_once('../inc/util.inc');
require_once('../inc/cache.inc');

//  proc_type
//      os_name
//          os_version
//              os_detail
//  Intel
//      Windows
//          10
//              Core x64 Edition
//              Enterprise x64 Edition
//          11
//      Linux
//          Ubuntu
//              20.04.6 LTS
//          Debian
//  ARM
//      Linux
//      Darwin
//          23.4.0

function make_entry() {
    $x = new StdClass;
    $x->count = 0;
    $x->credit = 0;
    $x->children = [];
    return $x;
}

// $host represents a group of hosts with the same fields
//
function inc_entry($host, $entry) {
    $entry->count += $host->count;
    $entry->credit += $host->credit;
}

function add_host(&$hosts, $host) {
    inc_entry($host, $hosts);
    if (!array_key_exists($host->proc_type, $hosts->children)) {
        $hosts->children[$host->proc_type] = make_entry();
    }
    $x = &$hosts->children[$host->proc_type];
    inc_entry($host, $x);
    if (!array_key_exists($host->os_name, $x->children)) {
        $x->children[$host->os_name] = make_entry();
    }
    $y = &$x->children[$host->os_name];
    inc_entry($host, $y);
    if (!array_key_exists($host->os_version, $y->children)) {
        $y->children[$host->os_version] = make_entry();
    }
    $z = &$y->children[$host->os_version];
    inc_entry($host, $z);
    if (!array_key_exists($host->os_detail, $z->children)) {
        $z->children[$host->os_detail] = make_entry();
    }
    $a = &$z->children[$host->os_detail];
    inc_entry($host, $a);
}

function compare($x, $y) {
    if ($x->count < $y->count) return 1;
    if ($x->count > $y->count) return -1;
    return 0;
}

function sort_entry(&$entry) {
    uasort($entry->children, 'compare');
}

function sort_all(&$hosts) {
    sort_entry($hosts);
    foreach ($hosts->children as $x) {
        sort_entry($x);
        foreach ($x->children as $y) {
            sort_entry($y);
            foreach ($y->children as $z) {
                sort_entry($z);
            }
        }
    }
}

function credit_str($c) {
    return [number_format($c, 2), "align=right"];
}

function show($hosts) {
    page_head("Host stats", null, false, '', data_tables_head_extra());
    start_table('table-striped', 'id="results-table"');
    echo "<thead>";
    table_header('CPU type', 'OS', 'OS version', 'OS release', 'Count', 'Recent credit');
    echo "</thead><tbody>";
    table_row(
        'Total', '', '', '', $hosts->count, credit_str($hosts->credit)
    );
    // names come from client-reported host info; escape them
    //
    foreach ($hosts->children as $proc_type=>$entry) {
        $proc_type = htmlspecialchars($proc_type);
        table_row(
            $proc_type, 'all', '', '', $entry->count,
            credit_str($entry->credit)
        );
        foreach ($entry->children as $os_name=>$entry2) {
            $os_name = htmlspecialchars($os_name);
            table_row(
                $proc_type, "$os_name", 'all', '',
                $entry2->count,
                credit_str($entry2->credit)
            );
            foreach ($entry2->children as $os_version=>$entry3) {
                $os_version = htmlspecialchars($os_version);
                table_row(
                    $proc_type, $os_name, "$os_version", 'all',
                    $entry3->count,
                    credit_str($entry3->credit)
                );
                foreach ($entry3->children as $os_detail=>$entry4) {
                    if (!$os_detail) continue;
                    $os_detail = htmlspecialchars($os_detail);
                    table_row(
                        $proc_type, $os_name, $os_version, $os_detail,
                        $entry4->count,
                        credit_str($entry4->credit)
                    );
                }
            }
        }
    }
    echo "</tbody>";
    end_table();
    echo data_tables_script('results-table');
    page_tail();
}

function parse_host($p_vendor, $os_name, $os_version) {
    $x = new StdClass;
    if (strstr($p_vendor, 'Intel')
        || strstr($p_vendor, 'AMD')
    ) {
        $x->proc_type = 'Intel';
    } else if (strstr($p_vendor, 'ARM')
        || strstr($p_vendor, 'Apple')
    ) {
        $x->proc_type = 'ARM';
    } else {
        $x->proc_type = "Other CPU type ($p_vendor)";
    }

    if (strstr($os_name, 'Windows')) {
        $x->os_name = 'Windows';
        $n = strrpos($os_name, ' ');
        if ($n === false) {
            $x->os_version = 'Unknown version';
        } else {
            $x->os_version = substr($os_name, $n+1);
        }
        $n = strpos($os_version, ',');
        if ($n === false) {
            $x->os_detail = $os_version;
        } else {
            $x->os_detail = substr($os_version, 0, $n);
        }
    } else if (strstr($os_name, 'Linux')) {
        $x->os_name = 'Linux';
        $n = strrpos($os_name, ' ');
        if ($n === false) {
            $x->os_version = 'Unknown distro';
        } else {
            $x->os_version = substr($os_name, $n+1);
        }
        if (strstr($os_version, 'Fedora')) {
            $x->os_version = 'Fedora';
        }
        if (strstr($os_version, 'Arch')) {
            $x->os_version = 'Arch';
        }
        if (strstr($os_version, 'Debian')) {
            $x->os_version = 'Debian';
        }
        $n = strpos($os_version, '[');
        if ($n === false) {
            $x->os_detail = $os_version;
        } else {
            $x->os_detail = substr($os_version, 0, $n-1);
        }
    } else if (strstr($os_name, 'Darwin')) {
        $x->os_name = 'Darwin';
        $x->os_version = $os_version;
        $x->os_detail = '';
    } else if (strstr($os_name, 'Android')) {
        $x->os_name = 'Android';
        $n = strpos($os_version, ' ');
        if ($n === false) {
            $n = strpos($os_version, '_');
        }
        if ($n === false) {
            $n = strpos($os_version, '-');
        }
        if ($n === false) {
            $x->os_version = 'Unknown version';
        } else {
            $x->os_version = substr($os_version, 0, $n);
        }
        $x->os_detail = '';
    } else {
        $x->os_name = "Other OS ($os_name)";
        $x->os_version = $os_version;
        $x->os_detail = '';
    }
    return $x;
}

function get_host_entries() {
    $host_entries = make_entry();
    $db = BoincDb::get();
    $result = $db->do_query(
        "select p_vendor, os_name, os_version, count(*) as nhosts, sum(expavg_credit) as credit from DBNAME.host where expavg_credit > 0 group by p_vendor, os_name, os_version"
    );
    if (!$result) return $host_entries;
    while ($h = $result->fetch_object()) {
        $x = parse_host($h->p_vendor, $h->os_name, $h->os_version);
        $x->count = (int)$h->nhosts;
        $x->credit = (double)$h->credit;
        add_host($host_entries, $x);
    }
    $result->free();
    sort_all($host_entries);
    return $host_entries;
}

function main() {
    $d = get_cached_data(86400);
    if ($d) {
        $host_entries = unserialize($d);
    } else {
        $host_entries = get_host_entries();
        set_cached_data(86400, serialize($host_entries));
    }
    show($host_entries);
}

main();

?>
