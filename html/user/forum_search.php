<?php
/**
 * Allows users to search for a post or thread.  Sends to forum_search_action.php
 * for action and display.
 **/
require_once('../inc/forum.inc');
require_once('../inc/forum_std.inc');


db_init();

page_head(tr(FORUM_SEARCH));


start_table();
echo "<form action=\"forum_search_action.php\" method=\"post\">";
row1("Search query");
row2("Search for keywords:<br />
    <font size=-1>Posts that contain all the specified words will be displayed</font>",
    '<input type="text" style="width: 290px" name="search_keywords" size="30" /><br />
    <font size=-1>For example: "screensaver freeze"</font>');
row2("Search for author ID:<br />
    <font size=-1>Only posts by this author will be displayed</font>",
    '<input type="text" style="width: 150px" name="search_author" size="10" /><br />
    <font size=-1>For example: "43214"</font>');

row1("Search options");
row2("Search limits<br />
    <font size=-1>Search at most this many days back in time</font>",
    '<select name="search_max_time">
	<option value="1">1 day</option>
	<option value="3">3 days</option>
	<option value="7">7 days</option>
	<option value="15">15 days</option>
	<option value="30" selected>30 days</option>
	<option value="90">3 months</option>
	<option value="180">6 months</option>
	<option value="360">1 year</option>
    </select>');

if (get_str("forumid",true)){
    $forumid = get_str("forumid");
}
$forumlist="<option value=\"-1\">All</option>";
foreach ($mainFactory->getCategories() as $key => $category){
    foreach ($category->getForums() as $key2 => $forum){
	if ($forum->getID()==$forumid){
	    $forumlist.="<option selected value=\"".$forum->getID()."\">".$forum->getTitle()."</option>";
	} else {
	    $forumlist.="<option value=\"".$forum->getID()."\">".$forum->getTitle()."</option>";
	}
    }
}
row2("Forum<br />
    <font size=-1>Only display posts from this forum</font>",
    '<select name="search_forum">'.$forumlist.'</select');

foreach ($thread_sort_styles as $id => $style){
	if ($id == CREATE_TIME_NEW){
	    $sortlist.="<option selected value=\"".$id."\">".$style."</option>";
	} else {
	    $sortlist.="<option value=\"".$id."\">".$style."</option>";
	}
}
row2("Sort by", 
    '<select name="search_sort">'.$sortlist.'</select');

row1("&nbsp;");
row2("","<input type=\"submit\" value=\"Start the search\">");
echo "</form>";
end_table();

page_tail();
exit;

?>
