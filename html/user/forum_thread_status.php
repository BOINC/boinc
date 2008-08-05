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



// Change the status of a thread.
// A thread can be either unsolved or solved (used in the Q&A part)

require_once('../inc/forum.inc');

$threadid = get_int('id');
$thread = BoincThread::lookup_id($threadid);
$logged_in_user = get_logged_in_user();

$owner = BoincUser::lookup_id($thread->owner);
if ($logged_in_user->id == $owner->id){ 
    $ret = $thread->update("status=".THREAD_SOLVED);
    if (!$ret){
        error_page("Could not update the status of the thread: ".$thread->id);
    }
} else {
    error_page("You must be the owner of the thread to do this");
}

// --------------

page_head("Status of the thread");
echo "<p>The status has been updated. Thank you!</p>";
echo "<p><a href=\"forum_thread.php?nowrap=true&id=".$thread->id."\">Return to the thread</a></p>";
page_tail();
?>
