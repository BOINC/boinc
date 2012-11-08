<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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


// delete results without a corresponding workunit.
// (in principle these shouldn't exist)

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

require_once("../inc/boinc_db.inc");

$ndel = 0;
while (1) {
    $rs = BoincResult::enum("true order by id limit 100");
    $found = false;
    foreach ($rs as $r) {
        $wu = BoincWorkunit::lookup_id($r->workunitid);
        if ($wu) {
            echo "$r->id has a WU\n";
            $found = true;
            break;
        } else {
            echo "$r->id has no WU - deleting\n";
            $ndel++;
            $r->delete();
        }
    }
    if ($found) break;
}
echo "Done - deleted $ndel results\n";

?>
