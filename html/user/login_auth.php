<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// form for logging in with auth.
// this is intended for project admins only,
// so that they can log in to user accounts based on auth in DB

require_once("../inc/util.inc");

function show_auth_form() {
    page_head("Login with authenticator");
    form_start("login_action.php", "post");
    form_input_text("Authenticator", "authenticator");
    form_submit("OK");
    page_tail();
}

show_auth_form();

?>
