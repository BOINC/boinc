<?php

require_once('../include/database.inc');

define('NO_CONTROLS', 0);
define('FORUM_CONTROLS', 1);
define('HELPDESK_CONTROLS', 2);

define ('SOLUTION', 'This solved my problem');
define ('SUFFERER', 'I also have this problem');
define ('OFF_TOPIC', 'Off-topic');

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
    var $is_helpdesk;
    
    function Forum($id = -1, $category = -1, $orderID = -1, $title = '', $description = '', $timestamp = 0, $threads = 0, $posts = 0, $is_helpdesk = 0) {
        $vars = get_class_vars('Forum');
        foreach ($vars as $var => $value)
            $this->$var = $$var;
    }
    
    // TODO: Could define a function to take a $sort_style and return the associated ORDER BY sql;
    // TODO: Make sure sorting still works properly when there are enough threads to span multiple pages.
                 
    function getThreads($min = -1, $nRec = -1, $sort_style='modified-new') {
        $sql = 'SELECT * FROM thread WHERE forum = '.$this->id; 
        switch($sort_style) {
        	case 'modified-new':
            	$sql = $sql . ' ORDER BY timestamp DESC';
            	break;
        	case 'modified-old':
            	$sql = $sql . ' ORDER BY timestamp ASC';
            	break;
        	case 'views-most':
	            $sql = $sql . ' ORDER BY views DESC';
            	break;
        	case 'replies-most':
	            $sql = $sql . ' ORDER BY replies DESC';
            	break;
        	case 'activity-most':
	            $sql = $sql . ' ORDER by activity DESC, timestamp DESC';
            	break;
           	case 'help-activity-most':
           		$sql = $sql . ' ORDER by activity DESC, sufferers DESC';
           		break;
    	}
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
    var $sufferers;
    
    function Thread($id = -1, $forum = -1, $owner = -1, $title = '', $timestamp = 0, $views = 0, $replies = 0, $sufferers = 0) {
        $vars = get_class_vars('Thread');
        foreach ($vars as $var => $value)
            $this->$var = $$var;
    }
    
    // TODO: Constant for the defauly sort style.
    function getPosts($min = -1, $nRec = -1, $sort_style="date-old") {
        $sql = 'SELECT *, (score * votes) AS rating FROM post WHERE thread = '.$this->id;
        switch($sort_style) {
        case 'date-old':
            $sql = $sql . ' ORDER BY timestamp ASC';
            break;
        case 'date-new':
            $sql = $sql . ' ORDER BY timestamp DESC';
            break;
        case 'score-high':
        	$sql = $sql . ' ORDER BY score DESC';
        	break;
        case 'rating-high':
            $sql = $sql . ' ORDER BY rating DESC';
            break;
        }
            
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
    
    function reply($user = -1, $content = "", $parent_post=NULL) {
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
    var $votes;
    var $score;
    
    function Post($id = -1, $thread = -1, $user = -1, $timestamp = 0, $content = "", $modified=NULL, $parent_post=NULL, $votes=NULL, $score=NULL) {
        $vars = get_class_vars('Post');
        foreach ($vars as $var => $value)
            $this->$var = $$var;
    }
    
    function insert() {
        if ($this->parent_post) {
            $sql = "INSERT INTO post (thread, user, timestamp, content, parent_post) VALUES (".$this->thread.", ".$this->user.", UNIX_TIMESTAMP(), '".$this->content."', ".$this->parent_post.")";
        } else {
            $sql = "INSERT INTO post (thread, user, timestamp, content) VALUES (".$this->thread.", ".$this->user.", UNIX_TIMESTAMP(), '".$this->content."')";
        }
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
    return new Forum($forum['id'], $forum['category'], $forum['orderID'], $forum['title'], $forum['description'], $forum['timestamp'], $forum['threads'], $forum['posts'], $forum['is_helpdesk']);
}

function getNextThread($result) {
    $thread = sql_fetch_array($result);
    if (!$thread)
        return false;
    foreach ($thread as $var => $value)
        $thread[$var] = stripslashes($value);
    return new Thread($thread['id'], $thread['forum'], $thread['owner'], $thread['title'], $thread['timestamp'], $thread['views'], $thread['replies'], $thread['sufferers']);
}

function getNextPost($result) {
    $post = sql_fetch_array($result);
    if (!$post)
        return false;
    foreach ($post as $var => $value)
        $post[$var] = stripslashes($value);
    return new Post($post['id'], $post['thread'], $post['user'], $post['timestamp'], $post['content'], $post['modified'], $post['parent_post'], $post['votes'], $post['score']);
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

// Returns the post that started the thread with id = $threadId
function getFirstPost($threadID) {
	$sql = "SELECT * FROM post WHERE thread = " . $threadID ." ORDER BY id ASC";
	$result = mysql_query($sql);
	return mysql_fetch_object($result);
}

/* display functions */

function show_posts($thread, $sort_style, $filter, $show_controls=true, $do_coloring=true, $is_helpdesk=false) {
    global $logged_in_user;
    $n = 0;
    
    if ($show_controls && !$is_helpdesk) {
        $controls = FORUM_CONTROLS;
    } else if ($show_controls && $is_helpdesk) {
        $controls = HELPDESK_CONTROLS;
    } else {
        $controls = NO_CONTROLS;
    }
    
    $posts = $thread->getPosts(-1, -1, $sort_style);
    $firstPost = getFirstPost($thread->id);
    
    if ($is_helpdesk) {
        if ($firstPost) {
            show_post($firstPost, $thread, $logged_in_user, $n, $controls, true);
    	}
    }
    
    while ($post = getNextPost($posts)) {
        if ($post->score >= $filter) {
            if (!$is_helpdesk || ($is_helpdesk && $post->id != $firstPost->id)) {
            	show_post($post, $thread, $logged_in_user, $n, $controls, false);
            	if ($do_coloring) $n = ($n+1)%2;
            }
        }
    } 
}

function show_post($post, $thread, $logged_in_user, $n, $controls=FORUM_CONTROLS, $separate=false) {
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
				Joined: ", date('M j, Y', $user->create_time), "<br>Posts: $user->posts
			</p>
		</td>
		<td>
		";
		if ($controls == FORUM_CONTROLS || $controls == HELPDESK_CONTROLS) {
			echo "<form action=\"rate.php?post=", $post->id, "\" method=\"post\">";
		}
		echo "
			<table width=100% cellspacing=0 cellpadding=0>
				<tr valign=\"top\">
					<td align=\"left\" style=\"border:0px\"><p style=\"font-size:8pt\">
						Posted: ", date('D M j, Y g:i a', $post->timestamp);
        
        if ($post->parent_post) echo " in response to <a href=#$post->parent_post>Message ID $post->parent_post</a>.";
        if ($can_edit && $controls != NO_CONTROLS) echo "&nbsp;<a href=\"edit.php?id=$post->id\">[Edit this post]</a>"; 
        if ($post->modified) echo "<br>Last Modified: ", date('D M j, Y g:i a', $post->modified);
        echo "
			        </p>
                            </td>
        ";
        if ($controls == FORUM_CONTROLS) {
            echo "
                        <td align=\"right\" style=\"border:0px\">Rate this post:
                          <select name=\"rating\">
                              <option value=\"2\">Very helpful (+2)</option>
                              <option value=\"1\">Helpful (+1)</option>
                              <option selected value=\"0\">Neutral</option>
                              <option value=\"-1\">Unhelpful (-1)</option>
                              <option value=\"-2\">Off topic (-2)</option>
                          </select>
                          <input type=\"submit\" value=\"Rate\">
                        </td>
            ";
        } else if ($controls == HELPDESK_CONTROLS && $separate) {
	     	echo "
				<td align=\"right\" style=\"border:0px\">
					<input type=\"submit\" name=\"submit\" value=\"", SUFFERER, "\">
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
        echo "
                    </tr></table>";
        if ($controls == FORUM_CONTROLS || $controls == HELPDESK_CONTROLS) {            
			echo "</form>";
		}
		echo "<p>", nl2br(stripslashes($post->content)), "</p>";
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
            echo "<td align=\"right\" style=\"border:0px\">[<a href=\"reply.php?thread=" . $thread->id . "&post=" . $post->id . "#input\">Reply to this post</a>]</td>";
		} else if ($controls == HELPDESK_CONTROLS && !$separate) {
			echo "<td align=\"right\" style=\"border:0px\">[<a href=\"reply.php?thread=" . $thread->id . "&post=" . $post->id . "&helpdesk=1#input\">Reply to this answer</a>]</td>";
		}	
        echo "</tr></table></td></tr>";
        
        if ($separate) {
	        echo "
	        	</table>
	        	<br><br>
	        	<table class=\"content\" border=\"0\" cellpadding=\"5\" cellspacing=\"0\" width=\"100%\">
	        		<tr>
	        			<th style=\"width: 150px\">Author</th>
	        			<th>Answers</th>
	        ";
        }
}

?>