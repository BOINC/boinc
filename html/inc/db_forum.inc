<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit


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

function getThreads($forumID, $min=-1, $nRec=-1, $sort_style='modified-new', $show_hidden = 0, $sticky = 1) {
	/* Calling function:  Set $show_hidden to 1 if it is a moderator reading */
    $sql = 'SELECT * FROM thread WHERE forum = ' . $forumID ;
    if ($sticky){
	$stickysql = "sticky DESC, ";
    }
    if (!$show_hidden) {
    	$sql .= ' AND hidden = 0';
    }
    switch($sort_style) {
    case 'modified-new':
        $sql .= ' ORDER BY '.$stickysql.'timestamp DESC';
        break;
    case 'modified-old':
        $sql .= ' ORDER BY '.$stickysql.'timestamp ASC';
        break;
    case 'views-most':
        $sql .= ' ORDER BY '.$stickysql.'views DESC';
        break;
    case 'replies-most':
        $sql .= ' ORDER BY '.$stickysql.'replies DESC';
        break;
    case 'create_time':
        $sql .= ' ORDER by '.$stickysql.'create_time desc';
        break;
    case 'timestamp':
        $sql .= ' ORDER by '.$stickysql.'timestamp desc';
        break;
    case 'sufferers':
        $sql .= ' ORDER by '.$stickysql.'sufferers desc';
        break;
    case 'activity':
        $sql .= ' ORDER by '.$stickysql.'activity desc';
        break;
    case 'score':
        $sql .= ' ORDER by '.$stickysql.'score desc';
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

    $data = mysql_query($sql);
    echo mysql_error();
    return $data;
}

function getPosts($threadID, $min = -1, $nRec = -1, $sort_style="timestamp", $show_hidden = false) {
	/* Calling function: Set $show_hidden = true when it is a moderator reading */ 
    $sql = 'SELECT * FROM post WHERE thread = '. $threadID;
    if (!$show_hidden) {
    	$sql .= ' AND hidden = 0';
    }
    switch($sort_style) {
    case 'timestamp':
        $sql .= ' ORDER BY timestamp desc';
        break;
    case 'timestamp_asc':
        $sql .= ' ORDER BY timestamp asc';
        break;
    case 'score':
        $sql .= ' ORDER BY score DESC';
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
        $user->jump_to_unread=$prefs->jump_to_unread;
        $user->hide_signatures=$prefs->hide_signatures;
        $user->rated_posts=$prefs->rated_posts;
        $user->low_rating_threshold=$prefs->low_rating_threshold;
        $user->high_rating_threshold=$prefs->high_rating_threshold;
        $user->ignorelist=$prefs->ignorelist;
        $user->last_post=$prefs->last_post;
        $user->ignore_sticky_posts=$prefs->ignore_sticky_posts;
        $user->forum_preferences=1;

        //Set defaults in certain cases:
        if ($user->low_rating_threshold==0 and $user->high_rating_threshold==0){
            $user->low_rating_threshold=DEFAULT_LOW_RATING_THRESHOLD;
            $user->high_rating_threshold=DEFAULT_HIGH_RATING_THRESHOLD;
        }
    } else {
        mysql_query("insert into forum_preferences set userid='".$user->id."'");
        $user->forum_preferences=0;
    }
    return $user;
}

function setHasRated($user, $postid){
    mysql_query("UPDATE forum_preferences SET rated_posts = concat('|$postid',rated_posts) WHERE userid = '".$user->id."'");
    return mysql_error();
}

function setSortStyle($user,$place,$new_style){
    if ($user->id!=""){
        list($forum,$thread,$faq,$answer)=explode("|",$user->sorting);
        $$place=$new_style;
        $user->sorting=implode("|",array($forum,$thread,$faq,$answer));
        $sql = "UPDATE forum_preferences SET sorting = '".$user->sorting."' where userid = '".$user->id."'";
        mysql_query($sql);
    } else {
        list($forum,$thread,$faq,$answer)=explode("|",$_COOKIE['sorting']);
        $$place=$new_style;
        setcookie('sorting', implode("|",array($forum,$thread,$faq,$answer)), time()+3600*24*365);
    }
}

function getThreadLastVisited($user, $thread){
    if ($user->id==""){                                        //Disable read/unread stuff for users that are not logged in
        $user->thread_last_visited=time();                //Always display as visited
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

// Forum modifying functions.

function createThread($forumID, $ownerID, $title, $content, $add_signature=false) {

    $content = addslashes(sanitize_html(stripslashes($content)));

    $title = strip_tags(trim($title));
    if (strlen($title) == 0) {
        echo "empty title\n";
        return 0;
    }

    $sql = "insert into thread (forum, owner, title, create_time, timestamp) VALUES (" . $forumID . ", " . $ownerID . ", '" . $title . "', UNIX_TIMESTAMP(), UNIX_TIMESTAMP())";
    $result = mysql_query($sql);
    if (!$result) return false;
    $threadID = mysql_insert_id();

    addPost($threadID, $ownerID, NULL, $content, $add_signature);

    $sql = "UPDATE forum_preferences SET posts = posts + 1, last_post = ".time()." WHERE userid = " . $ownerID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE forum SET threads = threads + 1, posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $forumID . " LIMIT 1";
    mysql_query($sql);

    return $threadID;
}

function replyToThread($threadID, $userID, $content, $parent_post=NULL, $add_signature=false) {
    $thread = getThread($threadID);

    $content = addslashes(sanitize_html(stripslashes($content)));

    addPost($threadID, $userID, $parent_post, $content, $add_signature);

    $sql = "UPDATE forum_preferences SET posts = posts + 1, last_post = ".time()." WHERE userid = " . $userID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE thread SET replies = replies + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $threadID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE forum SET posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $thread->forum . " LIMIT 1";
    mysql_query($sql);
}

function addPost($threadID, $userID, $parentID, $content, $add_signature=false) {
    if ($add_signature){$sig=1;} else {$sig=0;};
    if ($parentID) {
        $sql = "INSERT INTO post (thread, user, timestamp, content, parent_post, signature) VALUES (" . $threadID . ", " . $userID . ", UNIX_TIMESTAMP(), '" . $content . "', " . $parentID . ", ".$sig.")";
    } else {
        $sql = "INSERT INTO post (thread, user, timestamp, content, signature) VALUES (" . $threadID . ", " . $userID . ", UNIX_TIMESTAMP(), '" . $content . "', ".$sig.")";
    }
    $result = mysql_query($sql);
    if (!$result) return false;
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

?>