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

// purge already-handled trickle messages from the DB
//
// no args: delete both up and down messages with handled != 0
// msg_from_host N
//    delete trickle ups with handled==N
// msg_to_host N
//    same, trickle down

require_once("../inc/boinc_db.inc");
$db = BoincDb::get();
if (!$db) die("no DB connection");

if ($argc == 1) {
    $db->do_query("delete from msg_from_host where handled <> 0");
    $db->do_query("delete from msg_to_host where handled <> 0");
} else if ($argv[1] == "msg_from_host") {
    $n = (int)$argv[2];
    $db->do_query("delete from msg_from_host where handled = $n");
} else if ($argv[1] == "msg_to_host") {
    $n = (int)$argv[2];
    $db->do_query("delete from msg_to_host where handled = $n");
} else {
    echo "usage: purge_trickles.php [msg_from_host | msg_to_host]\n";
}

?>
