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

require_once('../inc/util.inc');
require_once('../inc/time.inc');
require_once('../inc/forum.inc');
require_once('../inc/user.inc');

check_get_args(array("userid", "offset"));

$userid = get_int("userid");
$offset = get_int("offset", true);
if (!$offset) $offset=0;
$items_per_page = 20;

$user = lookup_user_id($userid);
$logged_in_user = get_logged_in_user(false);

// Policy for what to show:
// Team message board posts:
//    if requesting user is a member of team
//        if post is hidden
//           show only if requesting user is team admin
//    else don't show
// Other posts
//    if post is hidden
//       show only if requesting user is project admin
//

$show_all = false;
$show_hidden = false;
$teamid = 0;
$show_team = false;
$show_team_hidden = false;

if ($logged_in_user) {
    if ($user->id == $logged_in_user->id) {
        $show_all = true;
    } else {
        BoincForumPrefs::lookup($logged_in_user);
        if ($logged_in_user->prefs->privilege(0)) {
            $show_hidden = true;
        }
        $teamid = $logged_in_user->teamid;
        if ($teamid) {
            $team = BoincTeam::lookup_id($teamid);
            if ($team) {
                $show_team = true;
                if (is_team_admin($logged_in_user, $team)) {
                    $show_team_hidden = true;
                }
            } else {
                $teamid = 0;
            }
        }
    }
}
page_head(tra("Posts by %1", $user->name));

$posts = BoincPost::enum("user=$userid order by id desc limit 10000");
$n = 0;
start_table();
$options = get_output_options($logged_in_user);

$show_next = false;
foreach ($posts as $post) {
    $thread = BoincThread::lookup_id($post->thread);
    if (!$thread) continue;
    $forum = BoincForum::lookup_id($thread->forum);
    if (!$forum) continue;
    if (!$show_all) {
        if ($forum->parent_type == 1) {
            // post to team msg board
            if ($forum->category == $teamid) {
				if ($thread->hidden && !$show_team_hidden) {
					continue;
				}
                if ($post->hidden && !$show_team_hidden) {
                    continue;
                }
            } else {
                continue;
            }
        } else {
			if ($thread->hidden && !$show_hidden) {
				continue;
			}
            if ($post->hidden && !$show_hidden) {
                continue;
            }
        }
    }
	if ($n == $offset + $items_per_page) {
		$show_next = true;
		break;
	}
    if ($n >= $offset) {
        show_post_and_context($post, $thread, $forum, $options, $n+1);
    }
    $n++;
}
echo "</table><br><br>\n";

if ($offset) {
	$x = $offset - $items_per_page;
    echo "<a href=forum_user_posts.php?userid=$userid&offset=$x>
		<b>".tra("Previous %1", $items_per_page)."</b>
		</a>
    ";
	if ($show_next) echo " | ";
}

if ($show_next) {
    $offset += $items_per_page;
    echo "<a href=forum_user_posts.php?userid=$userid&offset=$offset>
		<b>".tra("Next %1", $items_per_page)."</b>
		</a>
    ";
}

page_tail();
?>
