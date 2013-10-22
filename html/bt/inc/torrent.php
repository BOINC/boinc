<?php

/**
 * A .torrent is a metafile containing info about a P2P file
 * This class encapsulates all related .torrent information
 * for a single file.
 * We don't seem to need multifile support since the clients can
 * simply download a .torrent for each file they want to fetch.
 */ 

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
 
class Torrent {
    private $trackerURL; // String
    private $name;  // String
    private $pieceLength = 524288; // (512KB)
    private $filename; // String
    private $webseeds; // URL String Array
    
    private $concattedSHA1; // Imploded string of SHA1 hashes
    private $generatedPieceLength;
    private $infoHash;
    
    /**
     * Filename is the full path to the file to be encapsulated.
     */
    function __construct($filename, $trackerURL, $webseeds=array()){
	if (!file_exists($filename)) throw new IllegalArgumentException("No such file found");

	$this->filename = $filename;
	$this->trackerURL = $trackerURL;
	$this->webseeds = $webseeds;
	$this->name = basename($filename);
    }
    
    /** 
     * Scans the entire file generating SHA1 hashes for each piece
     */
    public function ensureSHA1Loaded(){
	if (!$this->concattedSHA1 || ($this->pieceLength!=$this->generatedPieceLength)){
	    $fh = fopen($this->filename, "rb");
	    if (!$fh) throw new Exception("No filehandle");
	    $fsize = filesize($this->filename);
	    $this->concattedSHA1 = "";
	    for ($i = 0; $i<ceil($fsize/$this->pieceLength); $i++){
		fseek($fh, $i*$this->pieceLength);
		$this->concattedSHA1 .= sha1(fread($fh,$this->pieceLength), true);
	    }
	    $this->generatedPieceLength = $this->pieceLength;
	    fclose($fh);
	    if (!$this->concattedSHA1) throw new Exception("No SHA gotten");
	}
	
    }
    
    /**
     * Fetches all relevant information from the file and generates a 
     * .torrent
     */
    public function toEncoded(){
	$infoArray["name"] = $this->name;
	$infoArray["piece length"] = $this->pieceLength;
	$infoArray["length"] = filesize($this->filename);
	$this->ensureSHA1Loaded();
	$infoArray["pieces"] = $this->concattedSHA1;
	$infoDictionary = new BElement(BDictionary::toEncoded($infoArray));
	$this->infoHash = sha1($infoDictionary->toEncoded(), true);
	$metainfoArray = array("announce"=>$this->trackerURL, "info"=>$infoDictionary);
	$metainfoArray["url-list"] = new BElement(BList::toEncoded($this->webseeds));
	$metainfoDictionary = BDictionary::toEncoded($metainfoArray);
	
	return $metainfoDictionary;
    }
    
    /**
     * Registers this torrent in the database
     */
    public function register(){
	$this->toEncoded();
	// Check if exists:
	$queryHandle = mysql_query("SELECT id from bittorrent_files where filename=\"".process_user_text($this->filename)."\"");
	if ($queryHandle && $data = mysql_fetch_object($queryHandle)){
	    $extra = "id=".$data->id.", ";
	} else {
	    $extra = "";
	}
	mysql_query("REPLACE into bittorrent_files set ".$extra."filename=\"".process_user_text($this->filename)."\", info_hash=\"".process_user_text($this->infoHash)."\", timestamp=".time());	    
    }
}