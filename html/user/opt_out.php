<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/boinc_db.inc");

check_get_args(array("code", "userid"));

$code = get_str("code");
$userid = get_int('userid');
$user = BoincUser::lookup_id($userid);
if (!$user) {
    error_page("no such user");
}

if (salted_key($user->authenticator) != $code) {
    error_page("invalid code");
}

$result = $user->update("send_email=0");

if ($result) {
    page_head("Removed from mailing list");
    echo "
        No further emails will be sent to $user->email_addr.
        <p>
        To resume getting emails,
        <a href=".url_base()."prefs_edit.php?subset=project>edit your project preferences</a>.
    ";
    page_tail();
}

?>
