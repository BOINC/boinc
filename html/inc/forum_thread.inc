<?php

/**
 * This class is a forum thread in some forum
 **/

// Also use the defines from forum
define('POST_SCORE',7);
define('THREAD_SOLVED', 1);
unset ($thread_sort_styles);
$thread_sort_styles[CREATE_TIME_OLD] = "Least recent post first";
$thread_sort_styles[CREATE_TIME_NEW] = "Most recent post first";
$thread_sort_styles[POST_SCORE] = "Highest rated posts first";
 
 
class Thread {
    // Container for the variables in the thread
    var $dbObj;
    // Container for the posts in this thread
    var $posts;
    // Array to cache lookups of last visit timestamps for users
    var $last_visit;
    var $dbhandler;

    /**
     * Constructor
     **/
    function Thread($id, $optional_dbobj=""){
        global $mainFactory;
        $this->dbhandler = $mainFactory->getDatabaseHandler();
        if ($optional_dbobj){
            // If someone was nice and supplied us with the db object, then let's use that (batch queries for instance)
            $this->dbObj = $optional_dbobj;
        } else {
            // We can find the object ourselves if only the id was specified.
            $this->dbObj = $this->dbhandler->getThread($id);
        }
        if (!$this->dbObj) error_page("Thread with id $id created but nothing returned from DB layer");
    }
    
    /**
     * Return fields for the thread
     **/
    function getID(){
        return $this->dbObj->id;
    }
    function getOwner(){
        return newUser($this->dbObj->owner);
    }
    function getTitle(){
        return $this->dbObj->title;
    }
    
    /**
     * Update the title of the thread.
     **/
    function setTitle($text){
        return $this->dbhandler->updateThread($this, "title", $text);
    }
    
    /**
     * Return more fields for the thread
     **/
    function getPostCount(){
        return $this->dbObj->replies;
    }
    function getLastTimestamp(){
        return $this->dbObj->timestamp;
    }
    function getViewCount(){
        return $this->dbObj->views;
    }
    function getType(){
        if (!isset($this->type)){
            $forum = $this->getForum();
            $category = $forum->getCategory();
            $this->type = $category->getType();
        }
        return $this->type;
    }
    function getStatus(){
        return $this->dbObj->status;
    }
    function setStatus($int){
        $result = $this->dbhandler->updateThread($this, "status", intval($int));
        return $result;
    }


    function isHidden(){
        return $this->dbObj->hidden;
    }
    
    /**
     * Hide or unhide the thread.
     **/
    function setHidden($bool){
        $result = $this->dbhandler->updateThread($this, "hidden", $bool?1:0);
        $forum = $this->getForum();
        if ($result){
            if ($bool){
                $forum->decThreadCount();
            } else {
                $forum->incThreadCount();
            }
        }
        return $result;
    }
    function hide(){ return $this->setHidden(true);}
    function unhide(){ return $this->setHidden(false);}
    
    /**
     * Return the stickiness of the thread.
     **/
    function isSticky(){
        return $this->dbObj->sticky;
    }
    
    /**
     * Update the stickiness of the thread.
     **/
    function setSticky($bool){
        $result = $this->dbhandler->updateThread($this, "sticky", $bool?1:0);
        return $result;
    }
    
    /**
     * Return even more fields from the thread
     **/
    function getForum(){
        return new Forum($this->dbObj->forum);
    }
    function getRating(){
        return ceil($this->dbObj->score*$this->dbObj->votes);
    }
    
    /**
     * When did the given user last read this thread?
     * Returns the timestamp or an approximation if unknown
     **/
    function getLastReadTimestamp($user){
        if (!isset($this->last_visit[$user->getID()])){
            $timestamp = $this->dbhandler->getLastReadTimestamp($this, $user);
            $this->last_visit[$user->getID()] = max(time()-MAX_FORUM_LOGGING_TIME,$timestamp,$user->getMarkForumReadTimestamp());
        }
        return $this->last_visit[$user->getID()];
    }
    
    /**
     * If the user just read the thread, update the timestamp for them.
     **/
    function setLastReadTimestamp($user){
        $this->dbhandler->setLastReadTimestamp($this, $user, time());
        $this->last_visit[$user->getID()] = time();
    }

    /**
     * Is the user subscribed to this thread?
     **/
    function isSubscribed($user){
        return $this->dbhandler->checkThreadSubscription($this, $user);
    }
    
    /**
     * Subscribe the user to this thread.
     **/
    function subscribe($user){
        return $this->dbhandler->createThreadSubscription($this, $user);
    }
    
    /**
     * Unsubscribe the user from this thread.
     **/
    function unsubscribe($user){
        return $this->dbhandler->deleteThreadSubscription($this, $user);
    }
    
    /**
     * List the users subscribed to the thread.
     **/
    function getSubscribers($unnotified_only=false){
        return $this->dbhandler->getThreadSubscribers($this, $unnotified_only);
    }
    
    /**
     * Create a reply in this thread
     **/
    function createReply($content, $parent, $user, $signature){
        Post::createNewPost($content, $parent, $user, $this, $signature);
        // Send out a notification to all subscribers to this thread
        $subscriber_time_pair = $this->getSubscribers();
        if ($subscriber_time_pair){
            foreach ($subscriber_time_pair as $key => $sub){
		$subscriber = newUser($sub->userid);
		$visit_time = $this->getLastReadTimestamp($subscriber);
		if (($visit_time > $sub->notified_time) && ($user->getID()!=$subscriber->getID())){
		    // If the user has visited the thread since last notification, send an email
		    send_reply_notification_email($this, $subscriber);
		    // and update the time when an email was sent
		    $this->dbhandler->updateThreadSubscription($this, $subscriber);
		}
            }
        }
    }

    /**
     * Increase the number of views by 1
     **/
    function incViews(){
        return $this->dbhandler->updateThread($this, "views", ($this->getViewCount()+1));
    }
    
    /**
     * Increase the post count for the thread by one.
     **/
    function incPostCount(){
        $forum = $this->getForum();
        return ($this->dbhandler->updateThread($this, "replies", ($this->getPostCount()+1)) &&
                $this->dbhandler->updateThread($this, "timestamp", time()) &&
                $forum->incPostCount());
    }
    
    /**
     * Decrease the post count by one.
     **/
    function decPostCount(){
        $forum = $this->getForum();
        return ($this->dbhandler->updateThread($this, "replies", ($this->getPostCount()-1)) &&
                $forum->decPostCount());
    }
    
    /**
     * Return a list of posts for this thread
     **/
    function getPosts($sort_style, $show_hidden_posts){
        if (!$this->posts){
            $list = $this->dbhandler->getPosts($this, $sort_style, $show_hidden_posts);
            foreach ($list as $key => $post){
                $this->posts[] = new Post($post->id, $post);
            }
            if (true){ 
                // It is possible to optimize database queries by precaching the users in one query here
                // The thread will be generated faster (and lower DB load) but the response time is slightly higher.

                foreach ($this->posts as $key => $post){
                    // Get the ids from the posts
                    $userids[] = $post->getOwnerID();
                }
                // Make sure it is unique
                $userids = array_unique($userids);
                // Lookup these ids in the DB layer
                $owners = $this->dbhandler->getUsersAndPrefs($userids);
                foreach ($owners as $key => $owner){
                    // For each returned DB object create a pre-loaded new user and throw it away
                    newUser($owner->id, $owner, $owner);
                }
            }
        }
        return $this->posts;
    }
    
    /**
     * To create a new thread, use this function like Thread::createNewThread(...)
     */
    function createNewThread($title, $content, $user, $forum, $signature){
        $content = substr($content,0,64000); // Shorten content to avoid cut-off html tags when inserting LARGE posts.
        $title = strip_tags(trim($title));
        $new_id = $this->dbhandler->createThread($title, $user, $forum);
        // Get back the new thread from the database layer
        $thread = new Thread($new_id);
        $thread->createReply($content, 0, $user, $signature);
        $forum = $thread->getForum();
        $forum->incThreadCount();
        return $thread;
    }

    /**
     * Move the thread to a new forum.
     **/    
    function moveTo($destination_forum){
        $current_forum = $this->getForum();
        $result = $this->dbhandler->updateThread($this, "forum", $destination_forum->getID());
        if ($result){
            $current_forum->decThreadCount();
            $current_forum->decPostCount($this->getPostCount());
            $destination_forum->incThreadCount();
            $destination_forum->incPostCount($this->getPostCount());
        }
        return $result;
    } 
}

?>