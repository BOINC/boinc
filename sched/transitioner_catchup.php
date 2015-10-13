#! /usr/bin/env php
<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

chdir("html/ops");
require_once("../inc/boinc_db.inc");
chdir("../..");

// script to let transitioners catch up, then run start

function catchup() {
    while (1) {
        $now = time();
        $wus = BoincWorkunit::enum("transition_time<$now limit 1");
        if (count($wus) == 0) break;
        echo "Some WUs need transition - running transitioner.\n";
        $ret = system("bin/transitioner --one_pass");
        echo "ret: $ret\n";
        sleep(1);
    }
    echo "Transitioner is caught up.\n";
}

catchup();

?>
