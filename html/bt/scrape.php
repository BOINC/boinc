<?php
/**
 * For multiple purposes it may be interesting for clients to know a little about
 * the network statistics for the entire Bittorrent swarm controlled by the tracker.
 * Scraping allows clients to get this information easily and quickly.
 */

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("config.php");

function trackerError($error){
    echo BDictionary::toEncoded(array("failure_reason"=>$error));
    exit;
}

$info_hash = rawurldecode($_GET["info_hash"]);
if (strlen($info_hash)!=20) throw new IllegalArgumentException("Malformed infohash key (length ".strlen($info_hash).")");

if (!$ip){
    $ip = $_SERVER["REMOTE_ADDR"];
}

// Is the IP banned?
db_init();
if (isIPBanned($ip)){
    trackerError("Banned IP: ".$ip);
}

// Check that the info_hash is one that we allow:
$queryHandle = mysql_query("SELECT * from bittorrent_files where info_hash=\"".process_user_text($info_hash)."\""); echo mysql_error();
if (!mysql_num_rows($queryHandle)){
    trackerError("The tracker does not allow tracking of this file:".$info_hash);
}
$infoHashObject = mysql_fetch_object($queryHandle);

// Get some statistical counts
$queryHandle = mysql_query("SELECT count(fileid) as complete from bittorrent_peers where fileid = '".$infoHashObject->id."' and status='completed'");
$data = mysql_fetch_object($queryHandle);
$complete = intval($data->complete);
$queryHandle = mysql_query("SELECT count(fileid) as incomplete from bittorrent_peers where fileid = '".$infoHashObject->id."' and status!='completed'");
$data = mysql_fetch_object($queryHandle);
$incomplete = intval($data->incomplete);

$out = BDictionary::toEncoded(array("interval"=>DEFAULT_CONNECTION_INTERVAL, "downloaded"=>$complete, "complete"=>$complete, "incomplete"=>$incomplete));

?>