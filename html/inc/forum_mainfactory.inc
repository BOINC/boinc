<?php
/**
 * The purpose of this file is to act as a factory for objects that need to
 * be accessible from every file in the web frontend. This file is included
 * as standard
 **/

class MainFactory {
    // Private variable to store the current database handler
    var $databaseHandler;

    /**
     * The root of the forum tree is the categories. These will be returned
     * as objects in a list by this function
     **/
    function getCategories(){
        $dbhandler = $this->getDatabaseHandler();
        $category_list = $dbhandler->getCategoryIDs();
        $i=0;
        while (isset($category_list[$i])){
            // For each category id create the coresponding Category object
            $categories[]=new Category($category_list[$i]);
            $i++;
        }
        return $categories;
    }

    /**
     * The database is accessed through a database handler. The default
     * database handler is returned by this function.
     * (The default depends on project preferences)
     **/
    function getDatabaseHandler(){
        // STUB: The default at the moment is the MySQLDatabaseHandler as it
        // is the only handler. Theoretically this should be read from a
        // config file instead of being hardcoded.
                
        // Singleton pattern. Only one database handler instance needs to exist
        if (!$this->databaseHandler) {
            require_once("../inc/forum_dbh.inc");
            require_once("../inc//forum_mysql_dbh.inc");
            $this->databaseHandler = new MySQLDatabaseHandler();
        }
        return $this->databaseHandler;
    }
}

// This object is globally accessible as $mainFactory
$mainFactory = new MainFactory();

?>
