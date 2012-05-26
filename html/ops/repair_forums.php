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

$cli_only = true;
require_once("../inc/forum_db.inc");
require_once("../inc/util_ops.inc");

set_time_limit(0);

// delete threads and posts for non-existent forums

function cleanup_thread($thread) {
    $posts = BoincPost::enum("thread=$thread->id");
    foreach ($posts as $post) {
        $post->delete();
    }
    $thread->delete();
}

function cleanup_orphan_threads() {
    $threads = BoincThread::enum("");
    foreach($threads as $thread) {
        $forum = BoincForum::lookup_id($thread->forum);
        if (!$forum) {
            cleanup_thread($thread);
        }
    }
}

// Old PHP code put \' and \" into text fields instead of ' and ".
// Fix this.
//
function remove_backslashes($table, $field) {
    $db = BoincDb::get();
    $query = "update ".$db->db_name.".$table set $field=replace(replace($field, '\\\\\\\"', '\\\"'), '\\\\\\'', '\'')";
    $db->do_query($query);
}

remove_backslashes("post", "content");
remove_backslashes("profile", "response1");
remove_backslashes("profile", "response2");
remove_backslashes("thread", "title");

//cleanup_orphan_threads();

?>
