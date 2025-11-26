#! /usr/bin/env php
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

// delete files from the web cache until free-space criteria are met

$cli_only = true;
require_once("../inc/cache.inc");
require_once("../inc/util_ops.inc");

set_time_limit(0);

function main(){
    echo "------- Starting at ".time_str(time())."-------\n";

    echo sprintf("max cache usage: %s\n", size_string(MAX_CACHE_USAGE));
    $too_old = 86400*7;
    while (1) {
        $u = disk_usage("../cache");
        echo sprintf("cache usage: %s\n", size_string($u));
        if ($u < MAX_CACHE_USAGE) {
            echo "criteria met; quitting\n";
            break;
        }
        echo sprintf("deleting files older than %s\n", time_diff($too_old));
        clean_cache($too_old, "../cache");
        $too_old /= 2;
        if ($too_old < 60) {
            break;
        }
    }
    echo "------- Finished at ".time_str(time())."-------\n";
}

main();
?>
