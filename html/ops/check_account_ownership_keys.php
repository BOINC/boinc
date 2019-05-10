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
require_once("../inc/account_ownership.inc");

if (!file_exists($account_ownership_private_key_file_path)) {
  echo "<p>The '$account_ownership_private_key_file_name' key <b>doesn't</b> exist. Please run the 'generate_account_ownership_keys.php' script from the BOINC web server command line.";
} else {
  echo "<p>The '$account_ownership_private_key_file_name' key exists.";
}

if (!file_exists($account_ownership_public_key_file_path)) {
  echo "<p>The '$account_ownership_public_key_file_name' key <b>doesn't</b> exist. Please run the 'generate_account_ownership_keys.php' script from the BOINC web server command line.";
} else {
  echo "<p>The '$account_ownership_public_key_file_name' key exists.";
}

echo "<p>For more info see the related wiki page: <a href=\"https://boinc.berkeley.edu/trac/wiki/ProofOfOwnership\">ProofOfOwnership</a></p>"

?>
