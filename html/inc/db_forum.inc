<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

function quickDBClean($unclean) {
    /* A quick function to save me a few keystrokes */
    return mysql_real_escape_string(stripslashes($unclean));
}

function getCategories() {
	/* What does $langID look like? I would like to add additional checks. */
    $langID = (!empty($_SESSION['lang']['id']))?$_SESSION['lang']['id']:1;
    $langID = quickDBClean($langID);
    $sql = "SELECT * FROM category WHERE lang = ".$langID." AND is_helpdesk = 0 ORDER BY orderID ASC";
    return mysql_query($sql);
}

function getHelpDeskCategories() {
    $sql = "SELECT * FROM category WHERE is_helpdesk = 1 ORDER BY orderID ASC";
    return mysql_query($sql);
}

function getForums($categoryID) {
	/* $categoryID - int */
	if (!is_numeric($categoryID)) {
        /* If the category ID isn't numeric, something funky is going on here
         * and we don't like it. 
         * Calling function should call an error page if this returns NULL.
         */
        return NULL;
	}
    $categoryID = quickDBClean($categoryID);
    $sql = 'SELECT * FROM forum WHERE category = ' . $categoryID . ' ORDER BY orderID ASC';
    return mysql_query($sql);
}

function getThreads($forumID, $min=-1, $nRec=-1, $sort_style='modified-new', $show_hidden = 0, $sticky = 1) {
	/* Calling function:  Set $show_hidden to 1 if it is a moderator reading
	 * Error page if this function returns NULL.
	 * $forumID - int
	 * $min - int
	 * $nRec - int
	 * $sort_style - string (checked by switch statement)
	 * $show_hidden - bool (not directly passed to SQL)
	 * $sticky - bool (not directly passed to SQL)
	 */
	if (! (is_numeric($forumID) && is_numeric($min) && is_numeric($nRec))) {
		return NULL;  // Something is wrong here.
	}
		
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
	/* Calling function: Set $show_hidden = true when it is a moderator reading
	 * error_page if this function returns NULL.
	 * $theradID - int
	 * $min - int
	 * $nRec - int
	 * $sort_style - string (checked by switch statement)
	 * $show_hidden - bool (not directly passed to SQL)
	 */ 
	if (! (is_numeric($threadID) && is_numeric($min) && is_numeric($nRec))) {
		return NULL;  // Something is wrong here.
	} 
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
	/* $categoryID - int */
	if (! is_numeric($categoryID)) {
		return NULL; // Something's rotten in Denmark
	}
    $sql = "SELECT * FROM category WHERE id = ".$categoryID ;
    $result = mysql_query($sql);
    if ($result) {
      return mysql_fetch_object($result);
    } else {
      return NULL;
    }
}

function getForum($forumID) {
	/* $forumID - int */
	if (! is_numeric($forumID)) {
		return NULL; // bad user!
	}
    $sql = "SELECT * FROM forum WHERE id = " . $forumID;
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

function getThread($threadID) {
	/* $threadID - int */
	if (! is_numeric($threadID)) {
		return NULL; // Running out of comments... 
	}
    $sql = "SELECT * FROM thread WHERE id = ".$threadID;
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

function getPost($postID) {
	/* $postID - int */
	if (! is_numeric($postID)) {
		return NULL; // /me smacks the calling function around with a trout
	}
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
	/* $threadID - int */
	if (! is_numeric($threadID)) {
		return NULL;  // These should always be numeric!
	}
    $sql = "SELECT * FROM post WHERE thread = " . $threadID ." ORDER BY id ASC limit 1";
    $result = mysql_query($sql);
    if ($result) {
        return mysql_fetch_object($result);
    } else {
        return NULL;
    }
}

function getForumPreferences($user){
	/* $user->id - int 
	 * Anything else make it in to SQL here?
	 */
	if (! is_numeric($user->id)) {
		$user->forum_preferences=0;	// Non-numeric user IDs are bad
		return $user;				// Just say they have no prefs
	}
	
    $sql = "SELECT * FROM forum_preferences WHERE userid = '".$user->id."'";
    $result = mysql_query($sql);
    if (mysql_num_rows($result)>0) {
        $prefs=mysql_fetch_object($result);

	// Combine the $user and $prefs objects
        $data = array_merge((array)$user, (array)$prefs); 
        $user = (object)$data;
	$user->forum_preferences = 1;

        //Set defaults in certain cases:
        if ($user->low_rating_threshold==0 and $user->high_rating_threshold==0){
            $user->low_rating_threshold=DEFAULT_LOW_RATING_THRESHOLD;
            $user->high_rating_threshold=DEFAULT_HIGH_RATING_THRESHOLD;
        }
    } else {
        mysql_query("INSERT INTO forum_preferences SET userid='".$user->id."'");
        $user->forum_preferences=0;
    }
    return $user;
}

function setHasRated($user, $postID){
	/* $user->id - int
	 * $postID - int
	 */
	if (! (is_numeric($user->id) && is_numeric($postID))) {
		return NULL; // Nothing to see here.
	}
    mysql_query("UPDATE forum_preferences SET rated_posts = concat('|$postID',rated_posts) WHERE userid = '".$user->id."'");
    return mysql_error();
}

function setSortStyle($user,$place,$new_style){
	/* $user->id - int
	 * $user->sorting - string (?)
	 */	
    if ($user->id!="" && is_numeric($user->id)){	// Dealing with a logged in user.
        list($forum,$thread,$faq,$answer)=explode("|",$user->sorting);
        $$place=$new_style;
        $user->sorting=quickDBClean(implode("|",array($forum,$thread,$faq,$answer)));
        // TODO: Check each value of the array to make sure it's one of the valid prefs
        $sql = "UPDATE forum_preferences SET sorting = '".$user->sorting."' where userid = '".$user->id."'";
        mysql_query($sql);
    } else {	// Dealing with a non-logged-in user (so we use cookies)
        list($forum,$thread,$faq,$answer)=explode("|",$_COOKIE['sorting']);
        $$place=$new_style;
        setcookie('sorting', implode("|",array($forum,$thread,$faq,$answer)), time()+3600*24*365);
    }
    return 0; // Functions should always return *something* in my opinion.
}

function getThreadLastVisited($user, $thread){
	/* $user->id - int
	 * $thread->id - int
	 */
    if (!$user) return $user;
    if ($user->id==""){                                        //Disable read/unread stuff for users that are not logged in
        $user->thread_last_visited=time();                //Always display as visited
        return $user;
    }
    if (!(is_numeric($user->id) && is_numeric($thread->id))) {
    	return $user;	// Return the same as above. (and don't get to SQL)
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
	/* $user->id - int
	 * $thread->id - int
	 * $timestamp - int (?)
	 */
	if (!(is_numeric($user->id) && is_numeric($thread->id))) {
		return NULL; // Those had better be numeric!
	}
	if ($timestamp==""){$timestamp=time();};
	$timestamp = quickDBClean($timestamp);
    $sql = "REPLACE DELAYED into forum_logging set userid='".$user->id."', threadid='".$thread->id."',  timestamp='$timestamp'";
    mysql_query($sql);
}


function incThreadViews($threadID) {
	/* $threadID - int */
	if (! is_numeric($threadID)) {
		return NULL;
	}
    $sql = "UPDATE thread SET views = views + 1 WHERE id = " . $threadID . " LIMIT 1";
    mysql_query($sql);
}

function cleanup_forum_log(){
	/* No external variables here, really... everything has already been in the
	 * database and wasn't created by somebody external.  It should be all good.
	 */
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
	/* $forumID - int
	 * $ownerID - int
	 * $title - string
	 * $content - string (not used here)
	 * $add_signature - bool (not used here)
	 */
    $content = substr($content,0,64000); // Shorten content to avoid cut-off html tags when inserting LARGE posts.
    $content = addslashes(sanitize_html(stripslashes($content)));

	if (! (is_numeric($forumID) && is_numeric($ownerID))) {
		return NULL;
	}
    $title = strip_tags(trim($title));
    if (strlen($title) == 0) {
        echo "Empty Title\n";
        return 0;
    }

	$title = quickDBClean($title);
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
	/* $threadID - int
	 * $userID - int
	 * $content - string (not used in sql here)
	 * $parent_post - int (not used in sql here)
	 * $add_signature - bool (not used in sql here)
	 * $thread->id - int (should be safe, but we'll check it anyway)
	 * $thread->forum - int (should be safe, but we'll check it anyway)
	 */
	if (! (is_numeric($threadID) && is_numeric($userID))) {
		return NULL; // Check thread and user ID before continuing
	}
    $thread = getThread($threadID);

	// Now let's make sure $thread is sane...
	if (! (is_numeric($thread->id) & is_numeric($thread->forum))) {
		return NULL;	// The thread ID and forum ID should always be numeric
	}					// ...if not, something is majorly wrong. (these came from the DB)
    $content = substr($content,0,64000); //Avoid cut-off html tags when posting LARGE texts
    $content = addslashes(sanitize_html(stripslashes($content)));

    addPost($threadID, $userID, $parent_post, $content, $add_signature);

    $sql = "UPDATE forum_preferences SET posts = posts + 1, last_post = ".time()." WHERE userid = " . $userID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE thread SET replies = replies + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $threadID . " LIMIT 1";
    mysql_query($sql);

    $sql = "UPDATE forum SET posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = " . $thread->forum . " LIMIT 1";
    mysql_query($sql);
    
    return true;  // Functions should always return something.
}

function addPost($threadID, $userID, $parentID, $content, $add_signature=false) {
	/* $threadID - int
	 * $userID - int
	 * $parentID - int (which may not exist)
	 * $content - mother of all strings
	 * $add_signature - bool (not directly inserted in to sql)
	 */
	if (! (is_numeric($userID) && is_numeric($threadID))) {
		return false; // Won't post.
	}
	$content = quickDBClean($content);
    if ($add_signature){$sig=1;} else {$sig=0;};
    if ($parentID) {
    	if (! is_numeric($parentID)) {
    		return false;
    	}
        $sql = "INSERT INTO post (thread, user, timestamp, content, parent_post, signature) VALUES (" . $threadID . ", " . $userID . ", UNIX_TIMESTAMP(), '" . $content . "', " . $parentID . ", ".$sig.")";
    } else {
        $sql = "INSERT INTO post (thread, user, timestamp, content, signature) VALUES (" . $threadID . ", " . $userID . ", UNIX_TIMESTAMP(), '" . $content . "', ".$sig.")";
    }
    $result = mysql_query($sql);
    if (!$result) return false;
    return true;
}

function updatePost($postID, $content) {
	/* $postID - int
	 * $content - mother of all strings
	 */
	if (! is_numeric($postID)) {
		return false; // That really needs to be numeric.
	}
    $content = substr($content,0,64000); //Avoid cut-off html tags when inserting LARGE texts
    $x = addslashes(sanitize_html(stripslashes($content)));
    $x = quickDBClean($x);
    $sql = "UPDATE post SET content = \"$x\", modified = UNIX_TIMESTAMP() WHERE id = " . $postID;
    $result = mysql_query($sql);
    if (!$result) return false;
    return true;
}

function updateThread($threadID, $title) {
	/* $threadID - int
	 * $title - string
	 */
	if (! is_numeric($threadID)) {
		return false;
	}
    $title = strip_tags(trim($title));
    $title = quickDBClean($title);
    if (strlen($title) == 0) {
        return false;
    }
    $sql = "UPDATE thread SET title = \"$title\" WHERE id = " . $threadID;
    $result = mysql_query($sql);
    if (!$result) return false;
    return true;
}

?>
