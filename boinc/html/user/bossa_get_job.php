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

require_once("../inc/util.inc");
require_once("../inc/bossa_db.inc");
require_once("../inc/bossa_impl.inc");

$user = get_logged_in_user();
BossaUser::lookup($user);

$bossa_app_id = get_int('bossa_app_id');
$app = BossaApp::lookup_id($bossa_app_id);

if (!$app) {
    error_page("no such app: $bossa_app_id");
}

{
    $trans = new BossaTransaction();
    show_next_job($app, $user);
}

?>
