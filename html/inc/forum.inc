<?php

require_once('../inc/db.inc');
require_once('../inc/sanitize_html.inc');
require_once('../inc/time.inc');

define('AVATAR_WIDTH', 100);
define('AVATAR_HEIGHT',100);
define('ST_ADMIN', 'Project administrator');
define('ST_MODERATOR', 'Forum moderator');
define('ST_DEV', 'Developer');
define('ST_VOLDEV', 'Volunteer developer');
define('ST_SCIENT', 'Project scientist');
define('ST_NEW_TIME', 1209600); //3600*24*14 - 14 days
define('ST_NEW', 'New member');


define('FORUM_OPEN_LINK_IN_NEW_WINDOW',1);
define('MAX_FORUM_LOGGING_TIME', 604800); //3600*24*7 - 7 days
define('NO_CONTROLS', 0);
define('FORUM_CONTROLS', 1);
define('HELPDESK_CONTROLS', 2);
define("EXCERPT_LENGTH", "120");


define ('SOLUTION', 'This answered my question');
define ('SUFFERER', 'I also have this question');
define ('OFF_TOPIC', 'Off-topic');

$forum_sort_styles['modified-new'] = "Most recent post first";
$forum_sort_styles['modified-old'] = "Least recent post first";
//$forum_sort_styles['activity-most'] = "Most recent activity first";
$forum_sort_styles['views-most'] = "Most views first";
$forum_sort_styles['replies-most'] = "Most posts first";

$thread_sort_styles['timestamp'] = "Newest first";
$thread_sort_styles['timestamp_asc'] = "Oldest first";
$thread_sort_styles['score'] = "Highest rated first";

$faq_sort_styles['create_time'] = "Most recent question first";
$faq_sort_styles['timestamp'] = "Most recent answer first";
$faq_sort_styles['activity'] = "Most frequently asked first";

$answer_sort_styles['score'] = "Highest score first";
$answer_sort_styles['timestamp'] = "Most recent first";
$answer_sort_styles['timestamp_asc'] = "Oldest first";

$thread_filter_styles['2'] = "\"Very helpful\"";
$thread_filter_styles['1'] = "At least \"helpful\"";
$thread_filter_styles['0'] = "At least \"neutral\"";
$thread_filter_styles['-1'] = "At least \"unhelpful\"";
$thread_filter_styles['-2'] = "All posts";

$post_ratings['2'] = "Very helpful (+2)";
$post_ratings['1'] = "Helpful (+1)";
$post_ratings['0'] = "Neutral";
$post_ratings['-1'] = "Unhelpful (-1)";
$post_ratings['-2'] = "Off topic (-2)";

db_init();

function getCategories() {
    $langID = (!empty($_SESSION['lang']['id']))?$_SESSION['lang']['id']:1;
    $sql = "SELECT * FROM category WHERE lang = ".$langID." AND is_helpdesk = 0 ORDER BY orderID ASC";
    return mysql_query($sql);
}

function getHelpDeskCategories() {
    $sql = "SELECT * FROM category WHERE is_helpdesk = 1 ORDER BY orderID ASC";
    return mysql_query($sql);
}

function getForums($categoryID) {
    $sql = 'SELECT * FROM forum WHERE category = ' . $categoryID . ' ORDER BY orderID ASC';
  return mysql_query($sql);
}

function getThreads($forumID, $min=-1, $nRec=-1, $sort_style='modified-new') {
    $sql = 'SELECT * FROM thread WHERE forum = ' . $forumID;
    switch($sort_style) {
        case 'modified-new':
            $sql .= ' ORDER BY timestamp DESC';
            break;
        case 'modified-old':
            $sql .= ' ORDER BY timestamp ASC';
            break;
        case 'views-most':
            $sql .= ' ORDER BY views DESC';
            break;
        case 'replies-most':
            $sql .= ' ORDER BY replies DESC';
            break;
        case 'create_time':
            $sql .= ' ORDER by create_time desc';
            break;
        case 'timestamp':
            $sql .= ' ORDER by timestamp desc';
            break;
        case 'sufferers':
            $sql .= ' ORDER by sufferers desc';
            break;
        case 'activity':
            $sql .= ' ORDER by activity desc';
            break;
        case 'score':
            $sql .= ' ORDER by score desc';
            break;
    }
    if ($min > -1) {
        $sql .= ' LIMIT '.$min;
        if ($nRec > -1) {
            $sql .= ', '.$nRec;
        }
    } else if ($nRec > -1) {
        $sql .= ' LIMIT '.$nRec;
    }

    return mysql_query($sql);
}

function getPosts($threadID, $min = -1, $nRec = -1, $sort_style="timestamp") {
    $sql = 'SELECT * FROM post WHERE thread = '. $threadID;
    switch($sort_style) {
    case 'timestamp':
        $sql = $sql . ' ORDER BY timestamp desc';
        break;
    case 'timestamp_asc':
        $sql = $sql . ' ORDER BY timestamp asc';
        break;
    case 'score':
        $sql = $sql . ' ORDER BY score DESC';
        break;
    }

    if ($min > -1) {
        $sql .= ' LIMIT '.$min;
        if ($nRec > -1) {
            $sql .= ', '.$nRec;
        }
    } elseif ($nRec > -1) {
        $sql .= ' LIMIT '.$nRec;
    }
    return mysql_query($sql);
}

/* specific database functions */

function getCategory($categoryID) {
    $sql = "SELECT * FROM category WHERE id = ".$categoryID;
    $result = mysql_query($sql);
    if ($result) {
      return mysql_fetch_object($result);
    } else {
      return NULL;
    }
}

function getForum($forumID) {
    $sql = "SELECT * FROM forum WHERE id = " . $forumID;
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

function getThread($threadID) {
    $sql = "SELECT * FROM thread WHERE id = ".$threadID;
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

function getPost($postID) {
    $sql = "SELECT * FROM post WHERE id = ".$postID;
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

// Returns the post that started the thread with id = $threadId
function getFirstPost($threadID) {
    $sql = "SELECT * FROM post WHERE thread = " . $threadID ." ORDER BY id ASC limit 1";
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

function getForumPreferences($user){
    $sql = "SELECT * FROM forum_preferences WHERE userid = '".$user->id."'";
    $result = mysql_query($sql);
    if (mysql_num_rows($result)>0) {
        $prefs=mysql_fetch_object($result);

	//Todo - find out how to simply merge two objects instead of specifying all the fields manually here
	$user->avatar=$prefs->avatar;
	$user->hide_avatars=$prefs->hide_avatars;
	$user->sorting=$prefs->sorting;
	$user->images_as_links=$prefs->images_as_links;
	$user->signature=$prefs->signature;
	$user->posts=$prefs->posts;
	$user->avatar_type=$prefs->avatar_type;
	$user->no_signature_by_default=$prefs->no_signature_by_default;
	$user->link_popup=$prefs->link_popup;
	$user->mark_as_read_timestamp=$prefs->mark_as_read_timestamp;
	$user->special_user=$prefs->special_user;
	$user->forum_preferences=1;
    } else {
        mysql_query("insert into forum_preferences set userid='".$user->id."'");
	$user->forum_preferences=0;
    }
    return $user;
}

function getSortStyle($user,$place){
    if ($user->id!=""){
	list($forum,$thread,$faq,$answer)=explode("|",$user->sorting);
    } else {
        list($forum,$thread,$faq,$answer)=explode("|",$_COOKIE['sorting']);
    }
    return $$place;
}

function setSortStyle($user,$place,$new_style){
    if ($user->id!=""){
	list($forum,$thread,$faq,$answer)=explode("|",$user->forum_sorting);
	$$place=$new_style;
	$user->forum_sorting=implode("|",array($forum,$thread,$faq,$answer));
	$sql = "UPDATE forum_preferences SET sorting = '".$user->forum_sorting."' where userid = '".$user->id."'";
	mysql_query($sql);
    } else {
        list($forum,$thread,$faq,$answer)=explode("|",$_COOKIE['sorting']);
	$$place=$new_style;
        setcookie('sorting', implode("|",array($forum,$thread,$faq,$answer)), time()+3600*24*365);
    }	
}

function getThreadLastVisited($user, $thread){
    if ($user->id==""){					//Disable read/unread stuff for users that are not logged in
	$user->thread_last_visited=time();		//Always display as visited
	return $user;
    }
    $sql = "SELECT timestamp from forum_logging where userid='".$user->id."' and threadid='".$thread->id."'";
    $result = mysql_query($sql);
    if ($result) {
	$data=mysql_fetch_object($result);
	$user->thread_last_visited=$data->timestamp;
    } else {
    }
    $user->thread_last_visited= max(time()-MAX_FORUM_LOGGING_TIME,$user->thread_last_visited,$user->mark_as_read_timestamp);
    //echo $user->thread_last_visited." - ".time();
    return $user;
}

function setThreadLastVisited($user, $thread, $timestamp=""){
    if ($timestamp==""){$timestamp=time();};
    $sql = "REPLACE DELAYED into forum_logging set userid='".$user->id."', threadid='".$thread->id."',  timestamp='$timestamp'";
    mysql_query($sql);
}


function incThreadViews($threadID) {
    $sql = "UPDATE thread SET views = views + 1 WHERE id = " . $threadID . " LIMIT 1";
    mysql_query($sql);
}

function cleanup_forum_log(){
    $sql = "SELECT timestamp FROM forum_logging where userid=0 and threadid=0";
    $result=mysql_query($sql);
    if (mysql_num_rows($result)>0) {
	$data=mysql_fetch_object($result);
	if ($data->timestamp<time()-MAX_FORUM_LOGGING_TIME){
	    $sql = "DELETE FROM forum_logging where timestamp<'".(time()-MAX_FORUM_LOGGING_TIME)."' and userid != 0";
	    mysql_query($sql);
	    echo mysql_error();
	    $sql = "REPLACE INTO forum_logging set userid=0, threadid=0, timestamp='".time()."'";
	    mysql_query($sql);
	}
    } else {
	//No cleanup timestamp found, make one:
	$sql = "INSERT INTO forum_logging set userid=0, threadid=0, timestamp=0";
	mysql_query($sql);
	echo mysql_error();
    }
}

/* Forum modifying functions. */

function createThread($forumID, $ownerID, $title, $content) {

    $title = addslashes(sanitize_html($title));
    $content = addslashes(sanitize_html($content));

    $title = trim($title);
    if (strlen($title) == 0) {
        return 0;
    }

    $sql = "insert into thread (forum, owner, title, create_time, timestamp) VALUES (" . $forumID . ", " . $ownerID . ", '" . $title . "', UNIX_TIMESTAMP(), UNIX_TIMESTAMP())";
    $result = mysql_query($sql);
    if (!$result) return false;
    $threadID = mysql_insert_id();

    $postID = addPost($threadID, $ownerID, NULL, $content);

    $sql = "UPDATE forum_preferences SET posts = posts + 1 WHERE userid = " . $ownerID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE forum SET threads = threads + 1, posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $forumID . " LIMIT 1";
    mysql_query($sql);

    return $threadID;
}

function replyToThread($threadID, $userID, $content, $parent_post=NULL) {
    $thread = getThread($threadID);

    $content = addslashes(sanitize_html(stripslashes($content)));

    addPost($threadID, $userID, $parent_post, $content);

    $sql = "UPDATE forum_preferences SET posts = posts + 1 WHERE userid = " . $userID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE thread SET replies = replies + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $threadID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE forum SET posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $thread->forum . " LIMIT 1";
    mysql_query($sql);
}

function addPost($threadID, $userID, $parentID, $content) {
    if ($parentID) {
        $sql = "INSERT INTO post (thread, user, timestamp, content, parent_post) VALUES (" . $threadID . ", " . $userID . ", UNIX_TIMESTAMP(), '" . $content . "', " . $parentID . ")";
    } else {
        $sql = "INSERT INTO post (thread, user, timestamp, content) VALUES (" . $threadID . ", " . $userID . ", UNIX_TIMESTAMP(), '" . $content . "')";
    }
    $result = mysql_query($sql);
    if (!$result) return false;
    //return ($post->id = mysql_insert_id());
    mysql_insert_id();
    return true;
}

function updatePost($postID, $content) {
    $x = addslashes(sanitize_html(stripslashes($content)));
    $sql = "UPDATE post SET content = \"$x\", modified = UNIX_TIMESTAMP() WHERE id = " . $postID;
    $result = mysql_query($sql);
    if (!$result) return false;
    return true;
}

function updateThread($threadID, $title) {
   $title = addslashes(sanitize_html(stripslashes($title)));
    $title = trim($title);
    if (strlen($title) == 0) {
        return false;
    }
   $sql = "UPDATE thread SET title = \"$title\" WHERE id = " . $threadID;
   $result = mysql_query($sql);
   if (!$result) return false;
   return true;
} 

/* display functions */

function show_posts($thread, $sort_style, $filter, $show_controls=true, $do_coloring=true, $is_helpdesk=false) {
    global $logged_in_user;
    $n = 1;

    if ($show_controls && !$is_helpdesk) {
        $controls = FORUM_CONTROLS;
    } else if ($show_controls && $is_helpdesk) {
        $controls = HELPDESK_CONTROLS;
    } else {
        $controls = NO_CONTROLS;
    }

    $posts = getPosts($thread->id, -1, -1, $sort_style);
    $logged_in_user = getThreadLastVisited($logged_in_user,$thread);
    setThreadLastVisited($logged_in_user,$thread);
    
    
    $firstPost = getFirstPost($thread->id);

    if ($is_helpdesk) {
        if ($firstPost) {
            show_post($firstPost, $thread, $logged_in_user, $n, $controls, true);
        }
    }

    while ($post = mysql_fetch_object($posts)) {
        if ($post->score >= $filter) {
            if (!$is_helpdesk || ($is_helpdesk && $post->id != $firstPost->id)) {
                show_post($post, $thread, $logged_in_user, $n, $controls, false);
                if ($do_coloring) $n = ($n+1)%2;
            }
        }
    }
}

function show_post($post, $thread, $logged_in_user, $n, $controls=FORUM_CONTROLS, $separate=false) {
    global $post_ratings;

    $user = lookup_user_id($post->user);
    $user = getForumPreferences($user);
    $data = mysql_query("SELECT userid FROM profile WHERE userid = " . $user->id);		//Lookup existance of profile for user
    $user->has_profile = (mysql_numrows($data) > 0);						//and store this info in the user object
    $user->has_avatar = ($user->avatar != "");							//for later access
	
    $can_edit = $logged_in_user && $user->id == $logged_in_user->id;

    echo "
        <tr class=\"row$n\" valign=\"top\">
        <td>
        <a name=\"$post->id\"></a>
    ";

    echo user_links($user, URL_BASE);
    
    if ($user->special_user) {									//If this user is somehow special
	if ($user->special_user==1) $fstatus=ST_ADMIN;						//this is displayed in the forums
	if ($user->special_user==2) $fstatus=ST_MODERATOR;					//so that people know who they are
	if ($user->special_user==3) $fstatus=ST_DEV;						//talking to.
	if ($user->special_user==4) $fstatus=ST_VOLDEV;
	if ($user->special_user==5) $fstatus=ST_SCIENT;
	/*...*/
    } else {
	if ($user->create_time>time()-ST_NEW_TIME) $fstatus=ST_NEW;
	/*...*/
    }
    if ($fstatus) echo "<br><font size=\"-2\">$fstatus</font>";
    
    echo "
        <p style=\"font-size:8pt\">";

    if ($user->has_avatar and $logged_in_user->hide_avatars!=1) echo "<img width=\"".AVATAR_WIDTH."\" height=\"".AVATAR_HEIGHT."\" src=\"".$user->avatar."\" alt=\"Avatar\"><br>";

    echo "
        Joined: ", gmdate('M j, Y', $user->create_time), "<br>Posts: ", $user->posts, "
        </p>
        </td>
        <td>
    ";

    if ($controls == FORUM_CONTROLS || $controls == HELPDESK_CONTROLS) {
        echo "<form action=\"forum_rate.php?post=", $post->id, "\" method=\"post\">";
    }
    
    echo "
        <table width=100% cellpadding=0 cellspacing=0 border=0 cellborder=0>
        <tr valign=top>
        <td align=left style=border:0px><font size=-2>";

    if ($post->timestamp>$logged_in_user->thread_last_visited){
	echo "<a name=\"unread\"><img src=\"".NEW_IMAGE."\" alt=\"Unread post\" height=\"".NEW_IMAGE_HEIGHT."\"></a>";
    }
	
    echo "	
        Posted: ", pretty_time_str($post->timestamp);
    ;

    if ($post->parent_post) echo "<br>in response to <a href=#$post->parent_post>Message ID $post->parent_post</a>.";
    if ($can_edit && $controls != NO_CONTROLS) echo "&nbsp;<a href=forum_edit.php?id=$post->id>[Edit this post]</a>";
    if ($post->modified) echo "<br>Last modified: ", pretty_time_Str($post->modified);

    echo "</p>\n</td>\n";

    if ($controls == FORUM_CONTROLS) {
        //echo "<td align=\"right\" style=\"border:0px\">Rate this post:";
        //show_select_from_array("rating", $post_ratings, "0");
        //echo "<input type=\"submit\" value=\"Rate\"></td>";
    } else if ($controls == HELPDESK_CONTROLS && $separate) {
        echo "
            <td align=\"right\" style=\"border:0px\">
            <input type=submit name=submit value=\"", SUFFERER, "\">
            </td>
        ";
    } else if ($controls == HELPDESK_CONTROLS && !$separate) {
        echo "
            <td align=\"right\" style=\"border:0px\">
            <input type=\"submit\" name=\"submit\" value=\"", SOLUTION, "\">
            <input type=\"submit\" name=\"submit\" value=\"", OFF_TOPIC, "\">
            </td>
        ";
    }

    echo "</tr>\n</table>\n";

    if ($controls == FORUM_CONTROLS || $controls == HELPDESK_CONTROLS) {
        echo "</form>";
    }

    $posttext=nl2br(stripslashes($post->content));
    //echo $logged_in_user->images_as_links;
    
    if ($logged_in_user->images_as_links==1){
	$posttext=image_as_link($posttext);
    }
    //echo $logged_in_user->link_popup;
    if ($logged_in_user->link_popup==1){
	$posttext=externalize_links($posttext);
    }
    
    echo "<p>", $posttext, "</p>";



    echo "<table width=100% cellspacing=0 cellpadding=0>
        <tr valign=\"bottom\">
        <td align=\"left\" style=\"border:0px; font-size:7pt\"><i>ID: ", $post->id;
    if ($controls == HELPDESK_CONTROLS && $separate) {
        echo "</i></td>";
    } else if ($controls == HELPDESK_CONTROLS && !$separate) {
        echo " / Score: ", ($post->score * $post->votes), "</i></td>";
    } else {
        echo " / Rating: ", $post->score, "</i></td>";
    }

    if ($controls == FORUM_CONTROLS) {
        echo "<td align=\"right\" style=\"border:0px\">[<a href=\"forum_reply.php?thread=" . $thread->id . "&post=" . $post->id . "#input\">Reply to this post</a>]</td>";
    } else if ($controls == HELPDESK_CONTROLS && !$separate) {
        echo "<td align=\"right\" style=\"border:0px\">[<a href=\"forum_reply.php?thread=" . $thread->id . "&post=" . $post->id . "&helpdesk=1#input\">Reply to this answer</a>]</td>";
    }

    echo "</tr></table></td></tr>";

    if ($separate) {
        echo "
            </table>
            <br><br>
            <table border=0 cellpadding=5 cellspacing=0 width=100%>
            <tr>
            <th>Author</th>
            <th>Answers</th>
        ";
    }
}

/* utility functions */

function externalize_links($text){
    $i=0;$linkpos=true;
    while (true){				//Find a link
        $linkpos=strpos($text,"<a ",$i);
	if ($linkpos===false) break;
	$out.= substr($text,$i,$linkpos-$i)."<a target=\"_new\"";	//Replace with target='_new'
	$i=$linkpos+3;
    }
    $out.=substr($text,$i);
    return $out;
}

function image_as_link($text){
/* This function depends on sanitized HTML - always use KSES or equivalent before using this */
    $i=0;
    while (true){							//Find an image
	$imgpos=strpos($text,"<img ",$i);
	if ($imgpos===false) break;
	$out.= substr($text,$i,$imgpos-$i)."<a href=";			//Replace with link start
	$temp1=strpos($text,"src=",$imgpos)+5;				//Find the image source
	$temp2=strpos($text,">",$imgpos)+1;				//Or the end of the tag
	if ($temp1<$temp2){						//If source was found within tag
	    $temp3=strpos($text,"\"",$temp1);				//Find the end of source
	    $out.=substr($text,$temp1,$temp3-$temp1);			//output the source
	}
	$out.=">[Image link]</a>";
	$i=$temp2;							//Now move to end of tag to continue
    }
    $out.=substr($text,$i);						//Output the rest
    return $out;
}
    

function start_forum_table($headings, $span=NULL) {
    echo "
        <p style=\"text-align:center\">
        <table order=0 cellpadding=5 cellspacing=0 width=100%>
        <tr>
    ";

    for ($i = 0; $i < count($headings); $i++) {
        $cell = "<th";
        if ($span) {
            $cell = $cell . " colspan=$span";
        }
        $cell = $cell . ">";
        echo $cell, $headings[$i], "</th>\n";
    }
    echo "</tr>\n";
}

function end_forum_table() {
    echo "</table></p>\n";
}

// generate a "select" element from an array of values
//
function select_from_array($name, $array, $selection) {
    $out= "<select name=\"$name\">";

    foreach ($array as $key => $value) {
        $out.= "<option ";
        if ($key == $selection) {
            $out.= "selected ";
        }
        $out.= "value=\"". $key. "\">". $value. "</option>";
    }
    $out.= "</select>";
    return $out;
}

function show_select_from_array($name, $array, $selection) {
    echo select_from_array($name,$array,$selection);
}

function show_forum_title($forum=NULL, $thread=NULL, $helpdesk=false) {
    echo "<p>\n";
    if (!$forum && !$thread) {
        echo "<p class=\"title\">";
        if ($helpdesk) {
            echo " Questions and problems</p>";
        } else {
            echo " Message boards</p>";
        }
    } else if ($forum && !$thread) {
        echo "<span class=title>";
        if ($helpdesk) {
            echo "<a href=forum_help_desk.php>", " Questions and problems</a> : ";
        } else {
            echo "<a href=forum_index.php>", " Message boards</a> : ";
        }
        echo $forum->title;
        echo "</span><br>";
    } else if ($forum && $thread) {
        echo "<span class=title>";
        if ($helpdesk) {
            echo "<a href=forum_help_desk.php>", " Questions and problems</a> : ";
        } else {
            echo "<a href=forum_index.php>", " Message boards</a> : ";
        }
        echo "<a href=forum_forum.php?id=$forum->id>", $forum->title, "</a> : ";
        echo strip_tags(stripslashes($thread->title));
        echo "</span><br>";
    } else {
        echo "Invalid input to show_forum_title<br>";
    }
    echo "</p>\n";
}

// show a thread with its context (e.g. for search results)
//
function show_thread($thread, $n) {
    $forum = getForum($thread->forum);
    $category = getCategory($forum->category);
    $first_post = getFirstPost($thread->id);
    $title = stripslashes($thread->title);
    $where = $category->is_helpdesk?"Questions and answers":"Message boards";
    $top_url = $category->is_helpdesk?"forum_help_desk.php":"forum_index.php";
    $excerpt = sub_sentence(stripslashes($first_post->content), ' ', EXCERPT_LENGTH, true);
    $posted = time_diff_str($thread->create_time, time());
    $last = time_diff_str($thread->timestamp, time());
    $m = $n%2;
    echo "
        <tr class=row$m>
        <td><font size=-2>
            $n) Posted $posted
            <br>
            Last response $last
        </td>
        <td valign=top>
            <a href=$top_url>$where</a> : $category->name :
            <a href=forum_forum.php?id=$forum->id>$forum->title</a> :
            <a href=forum_thread.php?id=$thread->id>$title</a>
            <br>
            <font size=-2>$excerpt</font>
        </td>
        </tr>
    ";
}

// show a post with its context (e.g. for search results)
//
function show_post2($post, $n) {
    $thread = getThread($post->thread);
    $forum = getForum($thread->forum);
    $category = getCategory($forum->category);
    $where = $category->is_helpdesk?"Questions and answers":"Message boards";
    $top_url = $category->is_helpdesk?"forum_help_desk.php":"forum_index.php";
    $content = nl2br(stripslashes($post->content));
    $when = time_diff_str($post->timestamp, time());
    $user = lookup_user_id($post->user);
    $title = stripslashes($thread->title);
    $m = $n%2;
    echo "
        <tr class=row$m>
        <td>
            $n) <a href=$top_url>$where</a> : $category->name :
            <a href=forum_forum.php?id=$forum->id>$forum->title</a> :
            <a href=forum_thread.php?id=$thread->id>$title</a>
            <br>
            Posted $when by $user->name
            <hr>
            $content
        </td>
        </tr>
    ";
}

function show_forum_summary($forum) {
    $x = time_diff_str($forum->timestamp, time());
    echo "
        <tr class=row1 style=\"font-size:8pt; text-align:right\">
        <td class=indent style=\"text-align:left\">
            <span style=\"font-size:10pt; font-weight:bold\">
            <a href=forum_forum.php?id=$forum->id>", $forum->title,
            "</a></span>
            <br>", $forum->description, "
        </td>
        <td>", $forum->threads, "</td>
        <td>", $forum->posts, "</td>
        <td>", $x, "</td>
    </tr>
    ";
}

?>
