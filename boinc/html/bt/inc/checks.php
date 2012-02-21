<?php
/**
 * Checks common to both the tracker and the scraping mechanism.
 */

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit


function isIPBanned($ip){
    $queryHandle = mysql_query("SELECT * from bittorrent_ipbans where ip=\"".process_user_text($ip)."\" and timestamp > ".time()); echo mysql_error();
    if (mysql_num_rows($queryHandle)){
	return true;
    } else {
	return false;
    }
} 
 
if (get_magic_quotes_gpc()) {
    function stripslashes_deep($value) {
        $value = is_array($value) ?
        array_map('stripslashes_deep', $value) :
        stripslashes($value);
        return $value;
    }
							    
    $_POST = array_map('stripslashes_deep', $_POST);
    $_GET = array_map('stripslashes_deep', $_GET);
    $_COOKIE = array_map('stripslashes_deep', $_COOKIE);
    $_REQUEST = array_map('stripslashes_deep', $_REQUEST);
}
?>