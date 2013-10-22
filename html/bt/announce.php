<?php
/**
 * This file contains the main portion of the tracker.
 * The tracker is the central part of any Bittorrent system. The tracker coordinates the downloads
 * between peers and ensures that only valid files are tracked. It is queried multiple times during
 * any Bittorrent-enabled download - usually whenever a peer wants to expand its list of other peers.
 *
 * The tracker also keeps track of how the downloads are progressing.
 */
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("config.php");

function trackerError($error){
    echo BDictionary::toEncoded(array("failure_reason"=>$error));
    exit;
}

$info_hash = rawurldecode($_GET["info_hash"]);
if (strlen($info_hash)!=20) throw new IllegalArgumentException("Malformed infohash key (length ".strlen($info_hash).")");
$peer_id = $_GET["peer_id"];
if (strlen($peer_id)!=20) throw new IllegalArgumentException("Malformed peer ID (".strlen($peer_id).")");
$port = $_GET["port"];
if (!is_numeric($port)) throw new IllegalArgumentException("Non-numeric port supplied");
$event = $_GET["event"];
$ip = $_GET["ip"];
$uploaded = $_GET["uploaded"]; 
if (!$uploaded) $uploaded = 0;
if (!is_numeric($uploaded)) throw new IllegalArgumentException("Non-numeric upload amount specified");
$downloaded = $_GET["downloaded"]; if (!$downloaded) $downloaded = 0;
if (!is_numeric($downloaded)) throw new IllegalArgumentException("Non-numeric download amount specified");

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
    trackerError("The tracker does not allow tracking of this file:".$info_hash . " [] ".$_GET["info_hash"]);
}
$infoHashObject = mysql_fetch_object($queryHandle);

// If the peer is actively doing something let's update the DB
if ($event=="started" || $event=="stopped" || $event=="completed"){
    mysql_query("REPLACE into bittorrent_peers SET fileid=".$infoHashObject->id.", peerid=\"".process_user_text($peer_id)."\", ip=\"".process_user_text($ip)."\", port=\"".process_user_text($port)."\", status=\"".$event."\", uploaded=".process_user_text($uploaded).", downloaded=".process_user_text($downloaded).", timestamp=".time());
    echo mysql_error();
} else {
    mysql_query("REPLACE delayed into bittorrent_peers SET fileid=".$infoHashObject->id.", peerid=\"".process_user_text($peer_id)."\", ip=\"".process_user_text($ip)."\", port=\"".process_user_text($port)."\", uploaded=".process_user_text($uploaded).", downloaded=".process_user_text($downloaded).", timestamp=".time());
    echo mysql_error();
}

// Always send back a random selection of peers who are downloading a file with the same info_hash
$queryHandle = mysql_query("SELECT * from bittorrent_peers WHERE fileid = ".$infoHashObject->id." order by RAND() limit ".MAX_INFO_HASH_PEERS); echo mysql_error();
$peerList = array();
while ($dbPeer = mysql_fetch_object($queryHandle)){
    $peer = array("peer id"=>$dbPeer->peerid, "ip"=>$dbPeer->ip, "port"=>intval($dbPeer->port));
    $peerList[] = new BElement(BDictionary::toEncoded($peer));
}

// Get some statistical counts
$queryHandle = mysql_query("SELECT count(fileid) as complete from bittorrent_peers where fileid = '".$infoHashObject->id."' and status='completed'");
$data = mysql_fetch_object($queryHandle);
$complete = intval($data->complete);
$queryHandle = mysql_query("SELECT count(fileid) as incomplete from bittorrent_peers where fileid = '".$infoHashObject->id."' and status!='completed'");
$data = mysql_fetch_object($queryHandle);
$incomplete = intval($data->incomplete);


$peersElement = new BElement(BList::toEncoded($peerList));
$out = BDictionary::toEncoded(array("interval"=>DEFAULT_CONNECTION_INTERVAL, "complete"=>$complete, "incomplete"=>$incomplete, "peers"=>$peersElement));

// Echo the answer to stdout
echo $out;
$fh = fopen(TRACKER_LOGFILE, "a");
fputs($fh, date(DATE_ATOM, time())." ".$_SERVER["REMOTE_ADDR"]." - ".$event."\n");
fclose($fh);

// ------------------------------------------------------
// Check if the database needs cleaning
$cache_args = "tracker_timer";
$cacheddata=get_cached_data(DB_CLEAN_TTL,$cache_args);
if ($cacheddata){ //If we have got the timer in cache
    // Do nothing
} else { //if not do queries etc to clean DB
    // TODO: update the bittorrent_statistics table here before deleting entries
    mysql_query("DELETE from bittorrent_files where timestamp<".(time()-TORRENT_TTL)); echo mysql_error();
    mysql_query("DELETE from bittorrent_peers where timestamp<".(time()-PEER_TTL)); echo mysql_error();
    mysql_query("DELETE from bittorrent_ipbans where timestamp<".time()); echo mysql_error();
    // And reset the timer in the cache
    set_cache_data(serialize(time()),$cache_args); //save data in cache
};
				     
?>