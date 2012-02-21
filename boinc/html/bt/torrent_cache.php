<?php
/**
 * The torrent cache and generator script takes care of dynamically generating .torrents
 * when they are requested by clients.
 * To optimize performance .torrent-objects are cached in the cache directory so that
 * on any second request they can be directly sent without re-calculating the SHA1 strings.
 *
 * A file is requested by torrent_cache.php?file=/some_file.filetype
 *
 * Symlinks outside the $fileDirectory are not allowed. Either do hardlinking or make the
 * entire $fileDirectory a symlink somewhere.
 * 
 * The optional webseeds must be HTTP servers with a similar structure to $fileDirectory
 * For instance if you have a webseed called "http://burp.boinc.dk/download/" and a file
 * $fileDirectory/dir/file
 * then the webseed musst respond to queries on http://burp.boinc.dk/download/dir/file.
 *
 * Please see config.php for the setting of these variables.
 */
require_once("config.php");

function isSubDir($possibleSubDir, $parent){
    $realPossible = realpath($possibleSubDir);
    $realParent = realpath($parent);
    return (substr($realPossible, 0, strlen($realParent)) == $realParent);
}

// Get the file request and do some checkups
$file = $_GET["file"];
if (!$file) throw new IllegalArgumentException("No file specified");
if (strpos(urldecode($file), "..")!==false) throw new IllegalArgumentException("Cannot use '..' in path");


// See if we've got the file
while (!$fileModTime){
    if (($fileModTime = @filemtime($fileDirectory.$file)) === false){
	$pos = strpos($file, "/", 1);
	if ($pos === false){
	    throw new IllegalArgumentException("File does not exist");
	} else {
	    $file = substr($file, $pos);
	}	
    }
}

$file = $fileDirectory.$file;

if (!$fileFilter->isValid($file)) throw new IllegalArgumentException("File was not accepted by the server for tracking.");

// Everything's fine let's lookup the .torrent in the cache if needed:
$cache_args = "file=".$file."&modtime=".$fileModTime;
$cacheddata=get_cached_data(TORRENT_CACHE_TTL,$cache_args);
if ($cacheddata){ //If we have got the data in cache
    $torrent = unserialize($cacheddata); // use the cached data
} else { //if not do queries etc to generate new data
   for ($i=0; $i<sizeof($webseeds);$i++){
       $realWebseeds[] = $webseeds[$i].substr($file, strlen($fileDirectory));
   }
			   
    $torrent = new Torrent($file, $trackerURL, $realWebseeds);
    $torrent->ensureSHA1Loaded();
    db_init();
    $torrent->register();
    set_cache_data(serialize($torrent),$cache_args); //save data in cache
};

header("Content-type: application/x-bittorrent");
header("Content-Disposition: attachment; filename=\"".basename($file).".torrent\"");

echo $torrent->toEncoded();
?>