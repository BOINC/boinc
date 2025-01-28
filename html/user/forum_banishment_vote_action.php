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
require_once("../inc/forum_db.inc");
require_once("../inc/forum_banishment_vote.inc");

if (DISABLE_FORUMS) error_page("Forums are disabled");

check_get_args(array("action", "userid", "tnow", "ttok"));

$config = get_config();

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);
check_tokens($logged_in_user->authenticator);

if (!$logged_in_user->prefs->privilege(S_MODERATOR)) {
    error_page(tra("You are not authorized to banish users."));
}

// See if "action" is provided - either through post or get
if (!post_str('action', true)) {
    if (!get_str('action', true)){
	    error_page(tra("You must specify an action..."));
    } else {
        $action = get_str('action');
    }
} else {
    $action = post_str('action');
}

$userid = post_int('userid');
$user=BoincUser::lookup_id($userid);
if (!$user) error_page('No such user');

if ($action!="start"){
    error_page("Unknown action");
}

// TODO: create a function for this in forum_banishment_vote.inc to make it more flexible
switch (post_int("category", true)) {
    case 1:
        $mod_category = tra("Obscene");
    case 2:
        $mod_category = tra("Flame/Hate mail");
    case 3:
        $mod_category = tra("User Request");
    default:
        $mod_category = tra("Other");
}

if (post_str('reason', true)){
    start_vote($config,$logged_in_user,$user, $mod_category,post_str("reason"));
} else {
    start_vote($config,$logged_in_user,$user, $mod_category,"None given");
}

?>
