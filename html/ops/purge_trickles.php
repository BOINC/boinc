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

require_once("../inc/boinc_db.inc");
$db = BoincDb::get();
if (!$db) die("no DB connection");
$db->do_query("delete from msg_from_host where handled <> 0");
$db->do_query("delete from msg_to_host where handled <> 0");

?>
