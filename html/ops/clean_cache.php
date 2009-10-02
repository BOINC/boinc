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

$cli_only = true;
require_once("../inc/cache.inc");
require_once("../inc/util_ops.inc");

set_time_limit(0);

function cache_check_diskspace2(){
   $too_old = 86400;
   while (1) {
       $f = disk_free_space("../cache");
       $u = disk_usage("../cache");
       echo "free: $f used: $u\n";
       if ($f > MIN_FREE_SPACE && $u < MAX_CACHE_USAGE) {
           break;
       }
       clean_cache($too_old, "../cache");
       $too_old/=2;
   }
}

cache_check_diskspace2();
?>
