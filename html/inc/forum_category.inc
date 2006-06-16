<?php

/**
 * This class serves to represent a forum category
 **/
 
class Category {
    // Container for the variables in a category
    var $dbObj;
    // Container for the forums in this category
    var $forums;
    var $dbhandler;

    /**
     * Constructor
     **/
    function Category($id){
        global $mainFactory;
        $this->dbhandler = $mainFactory->getDatabaseHandler();
        $this->dbObj = $this->dbhandler->getCategory($id);
        if (!$this->dbObj) error_page("Category with id $id created but nothing returned from DB layer");
    }
    
    /**
     * Spit out the category ID.
     **/
    function getID(){
        return $this->dbObj->id;
    }
    
    /**
     * Spit out the name of the category.
     **/
    function getName(){
        return $this->dbObj->name;
    }
    
    /**
     * Spit out the type of category (Helpdesk or normal)
     **/
    function getType(){
        return $this->dbObj->is_helpdesk;
    }
    
    /**
     * Return a list of forums for this category
     **/
    function getForums(){
        if (!$this->forums) {
            $list = $this->dbhandler->getForumIDs($this);
            foreach ($list as $key => $forum){
                $this->forums[] = new Forum($forum);
            }
        }
        return $this->forums;
    }
}

?>