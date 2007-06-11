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
 
?>