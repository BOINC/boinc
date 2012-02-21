#!/usr/bin/env php
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
require_once("../inc/util_ops.inc");
require_once("../inc/forum_db.inc");

define('MAX_REWARD', 4096);
define('SCALAR', 0.9);
set_time_limit(0);

echo date(DATE_RFC822), ": Starting\n";

$now = time();
$threads = BoincThread::enum();
foreach ($threads as $thread) {
    $forum = BoincForum::lookup_id($thread->forum);
    $category = BoincCategory::lookup_id($forum->category);
    if ($category->is_helpdesk) {
        $diff = ($now - $thread->create_time)/86400;
        $activity = ($thread->sufferers+1)/$diff;
        echo "help $diff $activity\n";
    } else {
        $posts = BoincPost::enum("thread=$thread->id");
        $activity = 0;
    
        foreach ($posts as $post) {
            $diff = $now - $post->timestamp;
            $diff /= 7*86400;
            $activity += pow(2, -$diff);
        }
        echo "forum $activity\n";
    }
    $thread->update("activity=$activity");
    
}

echo date(DATE_RFC822), ": Finished\n";

?>
