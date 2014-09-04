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

// DEPRECATED: this was used to clean user names from html that was allowed at this time

die("This file is DEPRECATED, see source for more information.");

$cli_only = true;
require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");

set_time_limit(0);
db_init();

function clean_user($user) {
    if ($user->name != sanitize_tags($user->name)) {
        $x = sanitize_tags($user->name);
        echo "ID: $user->id
name: $user->name
stripped name: $x
email: $user->email_addr
-----
";
        $x = boinc_real_escape_string($x);
        $x = trim($x);
        $query = "update user set name='$x' where id=$user->id";
        $retval = _mysql_query($query);
        echo $query;
    }
}

$result = _mysql_query("select id, name, email_addr from user");
while ($user = _mysql_fetch_object($result)) {
    clean_user($user);
}

?>
