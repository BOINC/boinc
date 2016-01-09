#!/usr/bin/env php

<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2015 University of California
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
require_once("../inc/uotd.inc");

$force_new = false;
if ($argc > 1) {
    if ($argv[1] == "-f" || $argv[1] == "--force") {
        $force_new = true;
    } else {
        echo "Usage: ".$argv[0]." [-f|--force]\n";
        echo "     -f | --force  Will select a new User of the day regardless if there already is one for the current day\n";
        exit(1);
    }
}

select_uotd($force_new);
?>
