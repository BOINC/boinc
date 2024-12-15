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


// Allows users to search for a post or thread.
// Sends to forum_search_action.php for action and display.

require_once('../inc/forum.inc');

if (DISABLE_FORUMS) error_page("Forums are disabled");

check_get_args(array("forumid"));

page_head(tra("Forum search"));

start_table();
echo "<form action=\"forum_search_action.php\" method=\"post\">";
row1(tra("Search query"));
row2(tra("Search for keywords:")."<br />
    <small>".tra("Posts that contain all the specified words will be displayed")."</small>",
    "<input type=\"text\" style=\"width: 290px\" name=\"search_keywords\" size=\"30\" /><br />
    <small>".tra("For example: \"screensaver freeze\"")."</small>");
row2(tra("Search for author ID:")."<br />
    <small>".tra("Only posts by this author will be displayed")."</small>",
    "<input type=\"text\" style=\"width: 150px\" name=\"search_author\" size=\"10\" /><br />
    <small>".tra("For example: \"43214\"")."</small>");

row1(tra("Search options"));
row2(tra("Search limits")."<br />
    <small>".tra("Search at most this many days back in time")."</small>",
    "<select class=\"form-control\" name=\"search_max_time\">
    <option value=\"1\">".tra("1 day")."</option>
    <option value=\"3\">".tra("%1 days", "3")."</option>
    <option value=\"7\">".tra("%1 days", "7")."</option>
    <option value=\"15\">".tra("%1 days", "15")."</option>
    <option value=\"30\" selected>".tra("%1 days", "30")."</option>
    <option value=\"90\">".tra("%1 months", "3")."</option>
    <option value=\"180\">".tra("%1 months", "6")."</option>
    <option value=\"365\">".tra("1 year")."</option>
    <option value=\"0\">".tra("no limit")."</option>
    </select>");

$forumid = null;
if (get_str("forumid",true)){
    $forumid = get_str("forumid");
}
$forumlist="<option value=\"-1\">".tra("All")."</option>";
$categories = BoincCategory::enum();
foreach ($categories as $category) {
    $forums = BoincForum::enum("parent_type=0 and category=$category->id");
    foreach ($forums as $forum) {
        if ($forum->id==$forumid){
            $forumlist.="<option selected value=\"".$forum->id."\">".$forum->title."</option>";
        } else {
            $forumlist.="<option value=\"".$forum->id."\">".$forum->title."</option>";
        }
    }
}
row2(tra("Forum")."<br />
    <small>".tra("Only display posts from this forum")."</small>",
    '<select class="form-control" name="search_forum">'.$forumlist.'</select');

$sortlist = null;
foreach ($thread_sort_styles as $id => $style){
    if ($id == CREATE_TIME_NEW){
        $sortlist.="<option selected value=\"".$id."\">".$style."</option>";
    } else {
        $sortlist.="<option value=\"".$id."\">".$style."</option>";
    }
}
row2(tra("Sort by"),
    '<select class="form-control" name="search_sort">'.$sortlist.'</select');

row1("&nbsp;");
row2("",
    sprintf('<input class="btn btn-success" %s type="submit" value="%s"',
        button_style(),
        tra("Start the search")
    )
);
echo "</form>";
end_table();

page_tail();

?>
