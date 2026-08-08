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

// Change the status of a thread.
// A thread can be either unsolved or solved (used in the Q&A part)

require_once('../inc/forum.inc');

check_get_args(array("id", "action"));

$threadid = get_int('id');
$thread = BoincThread::lookup_id($threadid);
if (!$thread) error_page("no such thread");

$logged_in_user = get_logged_in_user();
BoincForumPrefs::lookup($logged_in_user);

if (DISABLE_FORUMS && !is_admin($logged_in_user)) {
    error_page("Forums are disabled");
}

$owner = BoincUser::lookup_id($thread->owner);
if ($logged_in_user->id == $owner->id){
    $action = get_str("action");
    if ($action == "set") {
        $ret = $thread->update("status=1");
    } else {
        $ret = $thread->update("status=0");
    }
    if (!$ret){
        error_page("Could not update the status of the thread: ".$thread->id);
    }
} else {
    error_page("You must be the creator of the thread to update its status.");
}

// --------------

page_head(tra("Thread status updated"));
echo "<p>".tra("The status has been updated.")."</p>";
echo "<p><a href=\"forum_thread.php?nowrap=true&id=".$thread->id."\">".tra("Return to thread")."</a></p>";
page_tail();
?>
