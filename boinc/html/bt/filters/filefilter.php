<?php
/**
 * Any file filter must implement the following interface
 */

$cvs_version_tracker[]="\$Id: checks.php 12885 2007-06-11 18:27:24Z jbk $";  //Generated automatically - do not edit


abstract class FileFilter {
    abstract function isValid($file);    
}

?>