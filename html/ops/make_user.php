#! /usr/bin/env php

<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2016 University of California
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


// Script to create a user

// usage: make_user.php username emailaddr

require_once("../inc/user_util.inc");

if ($argc != 3) die("usage: make_user username email\n");

$user_name = $argv[1];
$email_addr = $argv[2];

$user = BoincUser::lookup_email_addr($email_addr);
if ($user) {
    die("user already exists\n");
}
$passwd_hash = md5("foobar".$email_addr);
$user = make_user($email_addr, $user_name, $passwd_hash);
if (!$user) die("can't create user\n");
echo "created user $user->id\n";

?>
