#!/usr/bin/env php
<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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

// usage: delete_user.php ID
// effectively delete the user with given ID
// USE THIS WITH EXTREME CAUTION.  CAN'T UNDO.

require_once("../inc/delete_account.inc");
require_once("../inc/boinc_db.inc");

die("Delete this line first\n");
$config = get_config();
if (!parse_bool($config, "enable_delete_account")) {
    die(tra("This feature is disabled.  Please enable it in the config.xml."));
}

$id = (int) $argv[1];

$user = BoincUser::lookup_id($id);
if (!$user) die("no such user\n");

$retval = delete_account($user);
if ($retval) {
    echo tra("User deleted");
} else {
    echo tra("Failed to delete user");
}

?>
