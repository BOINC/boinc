<?php

/**
 * A file filter that simply accepts any file whatsoever
 */

$cvs_version_tracker[]="\$Id: checks.php 12885 2007-06-11 18:27:24Z jbk $";  //Generated automatically - do not edit

 
class AllFilesFileFilter extends FileFilter {
    function isValid($file){
	return true;
    }
}

?>