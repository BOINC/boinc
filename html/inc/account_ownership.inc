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

require_once("../inc/boinc_db.inc");
require_once("../inc/user.inc");
require_once("../inc/util.inc");

$temp_config = get_config();
$temp_keydir = parse_config($temp_config, "<key_dir>"); // key directory can be customized

$account_ownership_private_key_file_name = "account_ownership_private.pem"; // private key file name
$account_ownership_private_key = "$temp_keydir/$account_ownership_private_key_file_name"; // for overwriting key
$account_ownership_private_key_file_path = "file://$account_ownership_private_key"; // for checking file existence

$account_ownership_public_key_file_name = "account_ownership_public.pem"; // public key file name
$account_ownership_public_key = "$temp_keydir/$account_ownership_public_key_file_name"; // for overwriting key
$account_ownership_public_key_file_path = "file://$account_ownership_public_key"; // for checking file existence

?>
