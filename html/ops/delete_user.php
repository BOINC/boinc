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

if (is_numeric($argv[1])) {
    $user = BoincUser::lookup_id((int) $argv[1]);
    if (!$user) die("no such user\n");
    $retval = delete_account($user);
    if ($retval) {
        echo "Failed to delete user: $retval\n";
    } else {
        echo "User deleted\n";
    }
} else {
    $users = BoincUser::enum(sprintf("name='%s'", $argv[1]));
    foreach ($users as $user) {
        $retval = delete_account($user);
        if ($retval) {
            echo "Failed to delete user: $retval\n";
        } else {
            echo "User $user->id deleted\n";
        }
    }
}

?>
