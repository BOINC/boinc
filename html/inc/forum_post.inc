<?php

/**
 * This class serves to represent a post in some thread
 **/
 
class Post {
    // Container for the variables in the post
    var $dbObj;
    // This post's ID in the database layer:
    var $id;
    var $dbhandler;

    /**
     * Constructor
     **/
    function Post($id, $optional_dbobj=""){
        global $mainFactory;
        $this->dbhandler = $mainFactory->getDatabaseHandler();
        // Not all referenced posts are actually displayed. To optimize things a bit
        // let's wait with populating the dbObj untill it is actually required.
        // Instead just store what our ID is.
        $this->id = $id;
        // If, by chance, we got our dbobj from the calling function (batch queries) just set it using that instead:
        if ($optional_dbobj){
            $this->dbObj = $optional_dbobj;
        }
    }
    
    /**
     * The following functions return various fields of the post in question
     **/
    function getID(){
        return $this->id;
    }
    function getTimestamp(){
        $this->ensureLoaded();
        return $this->dbObj->timestamp;
    }
    function getModificationTimestamp(){
        $this->ensureLoaded();
        return $this->dbObj->modified;
    }
    function getOwner(){
        $this->ensureLoaded();
        return newUser($this->dbObj->user);
    }
    /** In case someone want to do external loading for optimization: **/
    function getOwnerID(){
        $this->ensureLoaded();
        return $this->dbObj->user;
    }
    function getParentPostID(){
        $this->ensureLoaded();
        return $this->dbObj->parent_post;
    }
    function getContent(){
        $this->ensureLoaded();
        return $this->dbObj->content;
    }
    
    /**
     * Update the content of the post
     **/
    function setContent($text){
        $this->dbhandler->updatePost($this, "content", $text);
        $this->dbhandler->updatePost($this, "modified", time());
        $this->ensureLoaded();                
    }
    
    /**
     * Return the post's rating
     **/
    function getRating(){
        $this->ensureLoaded();
        return round($this->dbObj->score*$this->dbObj->votes);
    }
    
    /**
     * Determine if a particular user has rated this post
     **/
    function hasRated($user){
        return ($this->dbhandler->getPostRatingByUser($this, $user)!=0);
    }
    
    /**
     * Has this user reported this post before?
     */
    function hasReported($user){
        //This function is a STUB implementation - please make it if you get the time
        return false;
    }

    /**
     * Rate the post
     **/
    function rate($user, $rating){
        $this->ensureLoaded();
        $rating = intval($rating);
        $success = $this->dbhandler->setPostRatingByUser($this, $user, $rating);
        $success = $success && $this->dbhandler->updatePost($this, "votes", $this->dbObj->votes+1);
            if ($this->dbObj->votes!=0){ $score = (($this->dbObj->score*$this->dbObj->votes) + $rating) / ($this->dbObj->votes+1);} else {$score=$rating;}
        $success = $success && $this->dbhandler->updatePost($this, "score", $score);
        return $success;
    }
    
    /**
     * Return the parent thread (object)
     **/
    function getThread(){
        $this->ensureLoaded();
        return new Thread($this->dbObj->thread);
    }
    
    /**
     * Is this post hidden?
     **/
    function isHidden(){
        $this->ensureLoaded();
        return $this->dbObj->hidden;
    }
    
    /**
     * Hide or unhide the post.
     **/
    function setHidden($bool){
        $thread = $this->getThread();
        $result = $this->dbhandler->updatePost($this, "hidden", $bool?1:0);
        if ($result){
            if ($bool){
                $thread->decPostCount();
            } else {
                $thread->incPostCount();
            }
        }
        return $result;
    }
    function hide(){ return $this->setHidden(true);}
    function unhide(){ return $this->setHidden(false);}

    /**
     * Return a bool, true if there's a signature on this post, false otherwise.
     **/
    function hasSignature(){
        $this->ensureLoaded();
        return $this->dbObj->signature;
    }

    /**
     * When any attribute that actually needs database access is used
     * make sure that the dbObj is loaded
     **/
    function ensureLoaded(){
        if (!$this->dbObj){
            $this->dbObj = $this->dbhandler->getPost($this->id);
            if (!$this->dbObj) error_page("Post with id ".$this->id." requested but DB layer returned nothing.");
        }
    }
    
    /**
     * Move this post to another thread by
     * - decrementing the postcount in this thread by 1
     * - Moving the post
     * - incrementing the destination thread postcount by 1
     */
    function move($thread){
        $thread = $this->getThread();
        return ($thread->decPostCount() &&
                $this->dbhandler->updatePost($this, "thread", $thread->getID()) &&
                $thread->incPostCount());
    }
    
    /**
     * Use this function to create a new post in the given thread
     * Since the post does not exist simply call as a class function: Post::createNewPost(...)
     **/
    function createNewPost($content, $parent, $user, $thread, $signature){
        $content = substr($content,0,64000); //Avoid cut-off html tags when posting LARGE texts
        
        $this->dbhandler->createPost($content, $parent, $user, $thread, $signature);
        $user->incPostCount();
        $thread->incPostCount();
    }

}

?>