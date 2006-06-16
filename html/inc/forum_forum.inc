<?php

/**
 * This class serves to represent a forum under some category
 **/
define('THREADS_PER_PAGE',50);
// Forum sorting styles available
define('MODIFIED_NEW', 1);
define('MODIFIED_OLD',2);
define('VIEWS_MOST',3);
define('REPLIES_MOST',4);
define('CREATE_TIME_NEW',5);
define('CREATE_TIME_OLD',6);
unset ($forum_sort_styles);
$forum_sort_styles[MODIFIED_NEW] = "Most recent reply first";
//$forum_sort_styles[MODIFIED_OLD] = "Least recent reply first";
$forum_sort_styles[VIEWS_MOST] = "Most views first";
$forum_sort_styles[REPLIES_MOST] = "Most posts first";
$forum_sort_styles[CREATE_TIME_NEW] = "Most recently created first";
//$forum_sort_styles[CREATE_TIME_OLD] = "Least recently created first";
 
 
class Forum {
    // Container for the variables in the forum
    var $dbObj;
    // Container for the threads in this forum
    var $threads;
    var $dbhandler;
    /**
     * Constructor
     **/
    function Forum($id){
        global $mainFactory;
        $this->dbhandler = $mainFactory->getDatabaseHandler();
        $this->dbObj = $this->dbhandler->getForum($id);
        if (!$this->dbObj) error_page("Forum with id $id created but nothing returned from DB layer");
    }
    
    /**
     * The following functions simply return the fields for the forum. 
     **/
    function getID(){
        return $this->dbObj->id;
    }
    function getTitle(){
        return $this->dbObj->title;
    }
    function getDescription(){
        return $this->dbObj->description;
    }
    function getPostCount(){
        return $this->dbObj->posts;
    }
    function getThreadCount(){
        return $this->dbObj->threads;
    }
    function getLastTimestamp(){
        return $this->dbObj->timestamp;
    }
    function getPostMinTotalCredit(){
        return $this->dbObj->post_min_total_credit;
    }
    function getPostMinExpavgCredit(){
        return $this->dbObj->post_min_expavg_credit;
    }
    function getPostMinInterval(){
        return $this->dbObj->post_min_interval;
    }
    function getRateMinTotalCredit(){
        return $this->dbObj->reate_min_total_credit;
    }
    function getRateMinExpavgCredit(){
        return $this->dbObj->reate_min_expavg_credit;
    }
    function getCategory(){
        return new Category($this->dbObj->category);
    }
    
    /**
     * Increase the post count for the forum (default by one)
     **/
    function incPostCount($amount=1){
        return ($this->dbhandler->updateForum($this, "posts", ($this->getPostCount()+$amount)) && 
                $this->dbhandler->updateForum($this, "timestamp", time()));
    }
    
    /**
     * Decrease the post count for the forum (default by one)
     **/
    function decPostCount($amount=1){
        return $this->dbhandler->updateForum($this, "posts", ($this->getPostCount()-$amount));
    }
    
    /**
     * Increase the thread count for the forum by one
     **/
    function incThreadCount(){
        return $this->dbhandler->updateForum($this, "threads", ($this->getThreadCount()+1));
    }
    
    /**
     * Decrease the thread count for the forum by one
     **/
    function decThreadCount(){
        return $this->dbhandler->updateForum($this, "threads", ($this->getThreadCount()-1));
    }
        
    /**
     * Return a list of threads for this forum
     **/
    function getThreads($start, $count, $sort_style=MODIFIED_NEW, $show_hidden=false, $sticky_first=true){
        if (!$this->threads){
            $list = $this->dbhandler->getThreads($this, $start, $count, $sort_style, $show_hidden, $sticky_first);
            foreach ($list as $key => $thread){
                $this->threads[] = new Thread($thread->id, $thread);
            }
        }
        return $this->threads;
    }
    
    /**
     * Create a thread in this forum
     **/
    function createThread($title, $content, $user, $signature){
        return Thread::createNewThread($title, $content, $user, $this, $signature);
    }

}

/** 
 * This function returns a string containing a paged navigation bar 
 * for the given forum.  The default start place is 0. 
 **/
function show_page_nav($forum, $start=0){
    // How many pages to potentially show before and after this one:
    $preshow = 5; $postshow = 10;
    
    if ($forum->getThreadCount() > THREADS_PER_PAGE) {
        $total = ceil($forum->getThreadCount() / THREADS_PER_PAGE);
        $curpage = ceil($start / THREADS_PER_PAGE);
        // If this is not the first page, display "previous"
        if ($curpage > 0){
            $navbar = '<a href="forum_forum.php?id='.get_int("id").'&start='.(($curpage-1)*THREADS_PER_PAGE);
            $sort = get_int("sort",true);
            if ($sort) $navbar.='&sort='.$sort;
            $navbar.= '"> &lt;-- Previous</a> ';
        }
        
        // Display a list of pages surrounding this one
        for ($i=$curpage-$preshow;$i<($curpage+$postshow);$i++){
            if ($i<1) continue;
            if ($i>$total) break;
            if ($i == $curpage+1){$navbar.="<em>";} // If this is the current page, emphasize it.
            $navbar.='<a href="forum_forum.php?id='.get_int("id").'&start='.(($i-1)*THREADS_PER_PAGE);
            $sort = get_int("sort",true);
            if ($sort) $navbar.='&sort='.$sort;
            $navbar.='">'.$i.' |</a> ';
            if ($i == $curpage+1){$navbar.="</em>";}
        }

        // If there is a next page
        if ($curpage+1 < $total){
            $navbar.= '<a href="forum_forum.php?id='.get_int("id").'&start='.(($curpage+1)*THREADS_PER_PAGE);
            $sort = get_int("sort",true);
            if ($sort) $navbar.='&sort='.$sort;
            $navbar.= '"> Next --&gt;</a>';
        }

    }
    return $navbar;
}
?>