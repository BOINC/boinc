<?php

require_once('../include/database.inc');

class Category {
    var $id;
    var $name;

    function Category($id = -1, $name = '') {
        $vars = get_class_vars('Category');
        foreach ($vars as $var => $value)
            $this->$var = $$var;
    }

    function getForums() {
        $sql = 'SELECT * FROM forum WHERE category = '.$this->id.' ORDER BY orderID ASC';
        return sql_query($sql);
    }
}

class Forum {
    var	$id;
    var $category;
    var $orderID;
    var $title;
    var $description;
    var $timestamp;
    var $threads;
    var $posts;
    
    function Forum($id = -1, $category = -1, $orderID = -1, $title = '', $description = '', $timestamp = 0, $threads = 0, $posts = 0) {
        $vars = get_class_vars('Forum');
        foreach ($vars as $var => $value)
            $this->$var = $$var;
    }
    
    function getThreads($min = -1, $nRec = -1) {
        $sql = 'SELECT * FROM thread WHERE forum = '.$this->id.' ORDER BY timestamp DESC';
        if ($min > -1) {
            $sql .= ' LIMIT '.$min;
            if ($nRec > -1)
                $sql .= ', '.$nRec;
        } elseif ($nRec > -1)
              $sql .= ' LIMIT '.$nRec;
        return sql_query($sql);
    }
}

class Thread {
    var $id;
    var $forum;
    var $owner;
    
    var $title;
    
    var $timestamp;
    var $views;
    var $replies;
    
    function Thread($id = -1, $forum = -1, $owner = -1, $title = '', $timestamp = 0, $views = 0, $replies = 0) {
        $vars = get_class_vars('Thread');
        foreach ($vars as $var => $value)
            $this->$var = $$var;
    }
    
    function getPosts($min = -1, $nRec = -1) {
        $sql = 'SELECT * FROM post WHERE thread = '.$this->id.' ORDER BY id ASC';
        if ($min > -1) {
            $sql .= ' LIMIT '.$min;
            if ($nRec > -1)
                $sql .= ', '.$nRec;
        } elseif ($nRec > -1)
              $sql .= ' LIMIT '.$nRec;
        return sql_query($sql);
    }
    
    function getRootPostsArray() {
        $sql = 'SELECT * FROM post WHERE (thread = '.$this->id.') AND (parent_post IS NULL) ORDER BY id ASC';
        $result = sql_query($sql);
        
        if (mysql_num_rows($result) == 0) return NULL;
        
        while ($obj = mysql_fetch_object($result)) {
            $posts[] = $obj;
        }
        return $posts;
    }
    
    // Returns an array containing all posts in this thread ordered hierarchically.;
           
    function getOrderedPostArray() {
        $rootPosts = $this->getRootPostsArray();
        // Sort $rootPosts by date (most recent first);
        foreach($rootPosts as $post) {
            $childPosts = $post->getChildPostArray();
            $orderedPosts = array_merge($orderedPosts, $childPosts);
        }               
        return $orderedPosts;
    }
    
    function post($content) {
        $title = addslashes(htmlentities($this->title));
        $content = addslashes(htmlentities($content));
        
        $sql = "INSERT INTO thread (forum, owner, title, timestamp) VALUES (".$this->forum.", ".$this->owner.", '".$title."', UNIX_TIMESTAMP())";
        $result = sql_query($sql);
        if (!$result)
            return false;
        $this->id = sql_insert_id();
        
        $post = new Post();
        $post->thread = $this->id;
        $post->user = $this->owner;
        $post->content = $content;
        $postID = $post->insert();
        
        $sql = "UPDATE user SET posts = posts + 1 WHERE id = ".$this->owner." LIMIT 1";
        sql_query($sql);
        
        $sql = "UPDATE forum SET threads = threads + 1, posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = ".$this->forum." LIMIT 1";
        sql_query($sql);
        
        return $thread->id;
    }
    
    function reply($user = -1, $content = "", $parent_post = NULL) {
        $content = addslashes(htmlentities($content));
        
        $post = new Post();
        $post->thread = $this->id;
        $post->user = $user;
        $post->content = $content;
        $post->parent_post = $parent_post;
        $postID = $post->insert();
        
        $sql = "UPDATE user SET posts = posts + 1 WHERE id = ".$user." LIMIT 1";
        sql_query($sql);
                
        $sql = "UPDATE thread SET replies = replies + 1, timestamp = UNIX_TIMESTAMP() WHERE id = ".$this->id." LIMIT 1";
        sql_query($sql);
        
        $sql = "UPDATE forum SET posts = posts + 1, timestamp = UNIX_TIMESTAMP() WHERE id = ".$this->forum." LIMIT 1";
        sql_query($sql);
    }
    
    function incView() {
        $sql = "UPDATE thread SET views = views + 1 WHERE id = ".$this->id." LIMIT 1";
        sql_query($sql);
    }
}

class Post {
    var $id;
    var $thread;
    var $parent_post;
    var $user;
    var $timestamp;
    var $content;
    var $modified;
    
    function Post($id = -1, $thread = -1, $user = -1, $timestamp = 0, $content = "", $modified = NULL, $parent_post = NULL) {
        $vars = get_class_vars('Post');
        foreach ($vars as $var => $value)
            $this->$var = $$var;
    }
    
    function insert() {
        $sql = "INSERT INTO post (thread, user, timestamp, content) VALUES (".$this->thread.", ".$this->user.", UNIX_TIMESTAMP(), '".$this->content."')";
        $result = sql_query($sql);
        if (!$result)
            return false;
        return ($this->id = sql_insert_id());
    }
    
    function update($content) {
        $sql = "UPDATE post SET content = \"$content\", modified = UNIX_TIMESTAMP() WHERE id = " . $this->id;
        $result = sql_query($sql);
        if (!$result)
            return false;
        return true;
    }
    
    function getImmediateChildren() {
        $sql = "SELECT * FROM post WHERE parent_post = " . $this->id . " ORDER BY timestamp DESC";
        return sql_query($sql);
    }
    
    function getChildPostArray() {
        $sql = "SELECT * FROM post WHERE parent_id = ".$this->id;
        $result = sql_query($sql);
        
        // Base case
               if (mysql_num_rows($result) == 0) return array($this);
        
        while ($obj = mysql_fetch_object($result)) {
            $childPosts[] = $obj;
        }
        
        // Add ourselves to the array before our children
               $sortedArray[] = $this;
        // Sort $childPosts by date (most recent first);
        
        foreach ($childPosts as $post) {
            $childArray = $post->getChildPostArray();
            $sortedArray = array_merge($sortedArray, $childArray);
        }
        return $sortedArray;
    }
    
}

/* group database functions */

function getCategories() {
    $langID = (!empty($_SESSION['lang']['id']))?$_SESSION['lang']['id']:1;
    $sql = "SELECT * FROM category WHERE lang = ".$langID." ORDER BY orderID ASC";
    return sql_query($sql);
}

function getNextCategory($result) {
    $category = sql_fetch_array($result);
    if (!$category)
        return false;
    foreach ($category as $var => $value)
        $category[$var] = stripslashes($value);
    return new Category($category['id'], $category['name']);
}

function getNextForum($result) {
    $forum = sql_fetch_array($result);
    if (!$forum)
        return false;
    foreach ($forum as $var => $value)
        $forum[$var] = stripslashes($value);
    return new Forum($forum['id'], $forum['category'], $forum['orderID'], $forum['title'], $forum['description'], $forum['timestamp'], $forum['threads'], $forum['posts']);
}

function getNextThread($result) {
    $thread = sql_fetch_array($result);
    if (!$thread)
        return false;
    foreach ($thread as $var => $value)
        $thread[$var] = stripslashes($value);
    return new Thread($thread['id'], $thread['forum'], $thread['owner'], $thread['title'], $thread['timestamp'], $thread['views'], $thread['replies']);
}

function getNextPost($result) {
    $post = sql_fetch_array($result);
    if (!$post)
        return false;
    foreach ($post as $var => $value)
        $post[$var] = stripslashes($value);
    return new Post($post['id'], $post['thread'], $post['user'], $post['timestamp'], $post['content'], $post['modified'], $post['parent_post']);
}

/* specific database functions */

function getForum($forumID) {
    $sql = "SELECT * FROM forum WHERE id = ".$forumID;
    return getNextForum(sql_query($sql));
}

function getThread($threadID) {
    $sql = "SELECT * FROM thread WHERE id = ".$threadID;
    return getNextThread(sql_query($sql));
}

function getPost($postID) {
    $sql = "SELECT * FROM post WHERE id = ".$postID;
    return getNextPost(sql_query($sql));
}

/* display functions */

function show_posts($thread, $show_controls=true, $do_coloring=true) {
    global $logged_in_user;
    $n = 0;
    $posts = $thread->getPosts();
    while ($post = getNextPost($posts)) {
        show_post($post, $thread, $logged_in_user, $n, $show_controls);
	if ($do_coloring) $n = ($n+1)%2;
    } 
}

function show_post($post, $thread, $logged_in_user, $n, $show_controls=true) {
        $user = getUser($post->user);
        $can_edit = $logged_in_user && $user->id == $logged_in_user->id;
	
        echo "
	<tr class=\"row$n\" style=\"vertical-align:top\">
            <td>
                        <a name= $post->id ></a>
			<p style=\"font-weight:bold\">
	";
        if ($user->has_profile) { 
            echo "<a href=\"../view_profile.php?userid=$post->user\">$user->name</a>";
        } else { 
            echo $user->name; 
        }
        echo "
			</p>
			<p style=\"font-size:8pt\">
				Joined: ", date('M j, Y', $user->create_time),
				"<br>Posts: $user->posts
			</p>
		</td>
		<td>
			<p style=\"font-size:8pt\">
				Posted: ", date('D M j, Y g:i a', $post->timestamp);
        
        if ($post->parent_post) echo " in response to <a href=#$post->parent_post>Message ID $post->parent_post</a>.";
        if ($can_edit && $show_controls) echo "&nbsp;<a href=\"edit.php?id=$post->id\">[Edit this post]</a>"; 
        if ($post->modified) echo "<br>Last Modified: ", date('D M j, Y g:i a', $post->modified);
        echo "
			</p>
			<p>", nl2br(stripslashes($post->content)), "</p>";
        if ($show_controls) {
            echo "<p>[<a href=\"reply.php?thread=", $thread->id, "&post=", $post->id, "#input\">Reply to this post</a>]</p></td></tr>";
        }
}

?>