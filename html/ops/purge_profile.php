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

// Script to help you purge spam profiles.
//
// To use: do the following query from mysql:
//
// select name, id  from user, profile where user.id=profile.userid and match(response1, response2) against ('Viagra');
// (replace "Viagra" with other keywords)
//
// Then copy the ids into the array below and run this script

require_once("../inc/db.inc");
db_init();

$ids = array(
        9031517,
        9031518,
);

foreach ($ids as $id) {
    mysql_query("delete from user where id=$id");
    mysql_query("delete from profile where userid=$id");
}

?>
