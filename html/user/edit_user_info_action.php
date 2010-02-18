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
require_once("../inc/countries.inc");

$user = get_logged_in_user();
check_tokens($user->authenticator);

$name = post_str("user_name");
if ($name != strip_tags($name)) {
    error_page(tra("HTML tags are not allowed in your name."));
}
if (strlen($name) == 0) {
    error_page(tra("You must supply a name for your account."));
}
$url = post_str("url", true);
$url = strip_tags($url);
$country = post_str("country");
if ($country == "") {
    $country = "International";
}
if (!is_valid_country($country)) {
    error_page(tra("bad country"));
}
$country = BoincDb::escape_string($country);
$postal_code = post_str("postal_code", true);
$postal_code = strip_tags($postal_code);

$name = BoincDb::escape_string($name);
$url = BoincDb::escape_string($url);
$postal_code = BoincDb::escape_string($postal_code);

$result = $user->update(
    "name='$name', url='$url', country='$country', postal_code='$postal_code'"
);
if ($result) {
    Header("Location: home.php");
} else {
    error_page(tra("Couldn't update user info."));
}

?>
