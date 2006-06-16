<?php

/**
 * This is the MySQL implementation of the database handler interface
 **/

class MySQLDatabaseHandler extends DatabaseHandler {
    // The real database handle used to talk to the MySQL database
    var $handle;
    var $print_debug;
    
    /**
     * Construct a database handler object
     **/
    function MySQLDatabaseHandler(){
        $this->connect();
        $this->print_debug = true;
    }
    
    /**
     * Connect to the database using the configuration found in
     * the configuration file
     **/
    function connect(){
        $config = get_config();
        $user = parse_config($config, "<db_user>");
        $pass = parse_config($config, "<db_passwd>");
        $host = parse_config($config, "<db_host>");
        $this->handle = mysql_pconnect($host, $user, $pass);
        mysql_select_db(parse_config($config, "<db_name>"));
    }
    
    function microtime_float() {
        list($usec, $sec) = explode(" ", microtime());
        return ((float)$usec + (float)$sec);
    }
    
    /**
     * Run a MySQL query and print debug if debug is enabled
     **/
    function query($query){
        //$fh=fopen("/mnt/misc/test/sql_time","a");
        //$start=$this->microtime_float();
        $result = mysql_query($query);
        if ($this->print_debug && (!$result)) echo mysql_error();
        //fputs($fh, ($this->microtime_float()-$start)." - ".$query."\n");
        //fclose($fh);
        return $result;
    }

    /**
     * Return a list of category IDs for the forum
     **/
    function getCategoryIDs(){
        $query = "SELECT id FROM category ORDER BY orderID ASC";
        $result = $this->query($query);
        while ($data = mysql_fetch_object($result)){$id_list[]=$data->id;};
        return $id_list;        
    }

    /**
     * Return a specific category dbobj with the given ID
     **/
    function getCategory($id){
        $query = "SELECT * FROM category where id=".intval($id);
        $result = $this->query($query);        
        return mysql_fetch_object($result);        
    }

    /**
     * Return a list of forum IDs for a given category
     **/
    function getForumIDs($category){
        $query = "SELECT id FROM forum WHERE category = ".intval($category->getID())." ORDER BY orderID ASC";
        $result = $this->query($query);
        while ($data = mysql_fetch_object($result)){$id_list[]=$data->id;};
        return $id_list;        
    }

    /**
     * Return a specific forum dbobj with the given ID
     **/
    function getForum($id){
        $query = "SELECT * FROM forum where id=".intval($id);
        $result = $this->query($query);        
        return mysql_fetch_object($result);        
    }

    /**
     * Update a specific column for the thread given
     **/
    function updateForum($forum, $column, $new_value){
        $query = "UPDATE forum set ".$column."='".mysql_escape_string($new_value)."' where id=".intval($forum->getID());
        return $this->query($query);        
    }


    /**
     * Return a list of thread IDs for the forum
     **/
    function getThreads($forum, $start=-1, $nRec=-1, $sort_style=MODIFIED_NEW, $show_hidden = 0, $sticky = 1) {
        /* Calling function:  Set $show_hidden to 1 if it is a moderator reading
         * Error page if this function returns NULL.
         * $forumID - int
         * $start - int
         * $nRec - int
         * $sort_style - string (checked by switch statement)
         * $show_hidden - bool (not directly passed to SQL)
         * $sticky - bool (not directly passed to SQL)
         */
        if (! (is_numeric($forum->getID()) && is_numeric($start) && is_numeric($nRec))) {
            return NULL;  // Something is wrong here.
        }

        $sql = 'SELECT * FROM thread WHERE forum = ' . $forum->getID();
        if ($sticky){
            $stickysql = "sticky DESC, ";
        }
        if (!$show_hidden) {
            $sql .= ' AND hidden = 0';
        }
        switch($sort_style) {
        case MODIFIED_NEW:
            $sql .= ' ORDER BY '.$stickysql.'timestamp DESC';
            break;
        case MODIFIED_OLD:
            $sql .= ' ORDER BY '.$stickysql.'timestamp ASC';
            break;
        case VIEWS_MOST:
            $sql .= ' ORDER BY '.$stickysql.'views DESC';
            break;
        case REPLIES_MOST:
            $sql .= ' ORDER BY '.$stickysql.'replies DESC';
            break;
        case CREATE_TIME_NEW:
            $sql .= ' ORDER by '.$stickysql.'create_time desc';
            break;
        case CREATE_TIME_OLD:
            $sql .= ' ORDER by '.$stickysql.'create_time asc';
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
        default:
            $sql .= ' ORDER BY '.$stickysql.'timestamp DESC';
            break;
        }
    
        if ($start > -1) {
            $sql .= ' LIMIT '.$start;
            if ($nRec > -1) {
                $sql .= ', '.$nRec;
            }
        } else if ($nRec > -1) {
            $sql .= ' LIMIT '.$nRec;
        }
        
        $data = $this->query($sql);
        while ($result = mysql_fetch_object($data)){ $list[]=$result;};
        return $list;
    }

    /**
     * Return a specific thread dbobj with the given ID
     **/
    function getThread($id){
        $query = "SELECT * FROM thread where id=".intval($id);
        $result = $this->query($query);        
        return mysql_fetch_object($result);        
    }

    /**
     * Find out when a particular thread was last read by a given user
     **/
    function getLastReadTimestamp($thread, $user){
        $query = "SELECT timestamp from forum_logging where userid='".$user->getID()."' and threadid='".$thread->getID()."'";
        $result = $this->query($query);
        $timestamp = mysql_fetch_object($result);
        return $timestamp->timestamp;
    }
    
    /**
     * Update the last read time for a spcific user's viewing of a thread
     **/
    function setLastReadTimestamp($thread, $user, $timestamp){
        $query = "REPLACE DELAYED into forum_logging set userid='".$user->getID()."', threadid='".$thread->getID()."',  timestamp='".intval($timestamp)."'";
        $result = $this->query($query);
    }

    /**
     * Is the given user subscribed to the given thread?
     * Returns something that evaluates to false if not
     **/
    function checkThreadSubscription($thread, $user){
        $query = "SELECT * FROM subscriptions WHERE userid = " . $user->getID() . " AND threadid = " . $thread->getID();
        $result = $this->query($query);
        return mysql_num_rows($result);
    }
    
    /**
     * Subscribes a user to a thread.
     **/
    function createThreadSubscription($thread, $user){
        $query = "INSERT INTO subscriptions SET userid = " . $user->getID() . ", threadid = " . $thread->getID();
        $this->query($query);
    }
    
    /**
     * Unsubscribes a user from a thread.
     **/
    function deleteThreadSubscription($thread, $user){
        $query = "DELETE FROM subscriptions where userid = " . $user->getID() . " AND threadid = " . $thread->getID();
        $this->query($query);
    }

    /**
     * Updates the time of the subscription for a user to a thread.
     **/
    function updateThreadSubscription($thread, $user){
        $query = "UPDATE subscriptions SET notified_time = ".time()." WHERE userid = " . $user->getID() . " AND threadid = " . $thread->getID();
        $this->query($query);
    }
    
    /**
     * Return a list of user IDs for users who are subscribed to a thread.
     **/
    function getThreadSubscribers($thread, $unnotified_only=false){
	if ($unnotified_only) $notified = " AND notified = 0";
        $query = "SELECT * FROM subscriptions WHERE threadid = " . $thread->getID().$notified;
        $data = $this->query($query);
        while ($result = mysql_fetch_object($data)){ $list[]=$result;};
        return $list;
    }

    /**
     * Update a specific column for the thread given
     **/
    function updateThread($thread, $column, $new_value){
        $query = "UPDATE thread set ".$column."='".mysql_escape_string($new_value)."' where id=".intval($thread->getID());
        $result = $this->query($query);        
        return $result;
    }

    /**
     * Create a thread, returning the thread ID for the new thread.
     **/
    function createThread($title, $user, $forum){
        $query = "insert into thread (forum, owner, title, create_time, timestamp, replies)
        VALUES (" . $forum->getID() . ", " . $user->getID() . ", '" . mysql_escape_string($title) . "', UNIX_TIMESTAMP(), UNIX_TIMESTAMP(), -1)";
        $this->query($query);
        return mysql_insert_id();
    }

    /**
     * Searches for ALL the keywords in the $keyword_list array, optionally filters
     * by forum or user if specified. If time is given, only posts newer than that
     * are returned.
     **/
    function searchThreadTitles($keyword_list, $forum="", $user="", $time="", $limit=200, $sort_style=CREATE_TIME_NEW){
        $search_string="%";
        foreach ($keyword_list as $key => $word) {
            $search_string.=mysql_escape_string($word)."%";
        }        
        $query = "select * from thread where title like '".$search_string."'";
        if ($forum!="" && $forum!="all") {
            $query.=" and forum = ".intval($forum->getID());
        }
        if ($user!="" && $user!="all") {
            $query.=" and owner = ".intval($user->getID());
        }
        if ($time!="" && $user!="all") {
            $query.=" and timestamp > ".intval($time);
        }
        switch($sort_style) {
        case MODIFIED_NEW:
            $query .= ' ORDER BY timestamp DESC';
            break;
        case VIEWS_MOST:
            $query .= ' ORDER BY views DESC';
            break;
        case REPLIES_MOST:
            $query .= ' ORDER BY replies DESC';
            break;
        case CREATE_TIME_NEW:
            $query .= ' ORDER by create_time desc';
            break;
        case CREATE_TIME_OLD:
            $query .= ' ORDER by create_time asc';
            break;
        case 'score':
            $query .= ' ORDER by score desc';
            break;
        default:
            $query .= ' ORDER BY timestamp DESC';
            break;
        }

        $query.= " limit ".intval($limit);
        $data = $this->query($query);
        while ($result = mysql_fetch_object($data)){ $list[]=$result->id;};
        return $list;
    }

    /**
     * Get a list of post IDs for a thread
     **/
    function getPosts($thread, $sort_style=CREATE_TIME_NEW, $show_hidden = false) {
        /* Calling function: Set $show_hidden = true when it is a moderator reading
         * error_page if this function returns NULL.
         * $sort_style - string (checked by switch statement)
         * $show_hidden - bool (not directly passed to SQL)
         */
        $sql = 'SELECT * FROM post WHERE thread = '. $thread->getID();
        if (!$show_hidden) {
            $sql .= ' AND hidden = 0';
        }
        switch($sort_style) {
        case CREATE_TIME_NEW:
            $sql .= ' ORDER BY timestamp desc';
            break;
        case CREATE_TIME_OLD:
            $sql .= ' ORDER BY timestamp asc';
            break;
        case POST_SCORE:
            $sql .= ' ORDER BY score DESC';
            break;
        default:
            $sql .= ' ORDER BY timestamp asc';
            break;        
        }
        $data = $this->query($sql);
        while ($result = mysql_fetch_object($data)){ $list[]=$result;};
        return $list;
    }

    /**
     * Return a specific post dbobj with the given ID
     **/
    function getPost($id){
        $query = "SELECT * FROM post where id=".intval($id);
        $result = $this->query($query);        
        return mysql_fetch_object($result);        
    }

    /**
     * Update a specific column for the post given
     * Should return false on error
     **/
    function updatePost($post, $column, $new_value){
        $query = "UPDATE post set ".$column."='".mysql_escape_string($new_value)."' where id=".intval($post->getID());
        $result = $this->query($query);        
        return $result;
    }

    /**
     * Add a post to the database.
     **/
    function createPost($content, $parent, $user, $thread, $add_signature){
        if ($add_signature){$sig=1;} else {$sig=0;};
        if ($parent){
            $query = "INSERT INTO post 
            (thread, user, timestamp, content, parent_post, signature) 
            VALUES (" . $thread->getID() . ", " . $user->getID() . ", 
            UNIX_TIMESTAMP(), '" . mysql_escape_string($content) . "', " . $parent->getID() . ", ".$sig.")";
        } else {
            $query = "INSERT INTO post (thread, user, timestamp, content, signature) 
            VALUES (" . $thread->getID() . ", " . $user->getID() . ", UNIX_TIMESTAMP(), '" . mysql_escape_string($content). "', ".$sig.")";
        }
        $this->query($query);
    }

    /**
     * Return a rating (if any) that the given user has given the given thread
     * If no rating given, return 0
     **/
    function getPostRatingByUser($post, $user){
        $query = "SELECT rating FROM post_ratings WHERE post=".$post->getID()." and user=".$user->getID();
        $result = $this->query($query);
        if ($result){
            $rating = mysql_fetch_object($result);
            return $rating;
        } else {
            return 0;
        }
    }

    /**
     * A user rates a post
     * Should return false on error
     **/
    function setPostRatingByUser($post, $user, $rating){
        $query = "INSERT INTO post_ratings set post=".$post->getID().", user=".$user->getID().", rating=".intval($rating);
        $result = $this->query($query);
        return $result;
    }

    /**
     * Searches for ALL the keywords in the $keyword_list array, optionally filters
     * by forum or user if specified. If time is given, only posts newer than that
     * are returned.
     **/
    function searchPosts($keyword_list, $forum="", $user="", $time="", $limit=200, $sort_style=CREATE_TIME_NEW){
        $search_string="%";
        foreach ($keyword_list as $key => $word){
            $search_string.=mysql_escape_string($word)."%";
        }
        if ($forum!="" && $forum!="all"){
            $optional_join = " LEFT JOIN thread ON post.thread = thread.id";
        }
        $query = "select *,post.id as postid from post".$optional_join." where content like '".$search_string."'";
        if ($forum!="" && $forum!="all"){
            $query.=" and forum = ".intval($forum->getID());
        }
        if ($user!="" && $user!="all"){
            $query.=" and post.user = ".intval($user->getID());
        }
        if ($time!="" && $user!="all"){
            $query.=" and post.timestamp > ".intval($time);
        }
        switch($sort_style) {
        case VIEWS_MOST:
            $query.= ' ORDER BY views DESC';
            break;
        case CREATE_TIME_NEW:
            $query .= ' ORDER by post.timestamp desc';
            break;
        case CREATE_TIME_OLD:
            $query .= ' ORDER by post.timestamp asc';
            break;
        case POST_SCORE:
            $query .= ' ORDER by post.score desc';
            break;
        default:
            $query .= ' ORDER BY post.timestamp DESC';
            break;
        }

        $query.= " limit ".intval($limit);
        $data = $this->query($query);
        while ($result = mysql_fetch_object($data)){ $list[]=$result->postid;};
        return $list;
    }

    /**
     * Return a specific user dbobj with the given ID
     **/
    function getUser($id){
        $query = "SELECT * FROM user where id=".intval($id);
        $result = $this->query($query);        
        return mysql_fetch_object($result);        
    }
    
    /**
     * Return a list with user/pref dbobj with the given IDs
     **/
    function getUsersAndPrefs($idlist){
        $sql = implode(" OR id=",$idlist);
        $query = "SELECT * FROM user JOIN forum_preferences ON user.id = forum_preferences.userid WHERE id=".$sql;
        $data = $this->query($query);        
        while ($result = mysql_fetch_object($data)){ $list[]=$result;};
        return $list;
    }

    /**
     * Return a specific user preferences dbobj for the user
     * with the given ID.
     **/
    function getUserPrefs($user){
        $query = "SELECT * FROM forum_preferences where userid=".intval($user->getID());
        $result = $this->query($query);        
        return mysql_fetch_object($result);        
    }

    /**
     * Insert a record into the forum_preferences table for the given user.
     **/
    function createUserPrefs($user){
        $query = "INSERT INTO forum_preferences set userid=".intval($user->getID());
        return $this->query($query);        
    }

    /**
     * Update a specific user preference for the user given
     **/
    function updateUserPrefs($user, $preference, $new_value){
        $query = "UPDATE forum_preferences set ".$preference."='".mysql_escape_string($new_value)."' where userid=".intval($user->getID());
        $result = $this->query($query);        
    }

    /**
     * Delete a specific user's preferences (used to reset them)
     **/
    function deleteUserPrefs($user){
        $query = "DELETE FROM forum_preferences where userid=".intval($user->getID());
        return $result = $this->query($query);        
    }

    /**
     * Return the posts posted by the specific user
     **/
    function getUserPosts($user){
        $query = "SELECT * FROM post where user=".intval($user->getID()." ORDER BY timestamp DESC");
        $data = $this->query($query);        
        while ($result = mysql_fetch_object($data)){ $list[]=$result->id;};
        return $list;
    }
}

?>