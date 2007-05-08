<?php
/**
 * This class is used as part of the Bittorrent .torrent creator and tracker system.
 * A BElement is used to contain BEncoded elements inside other BEncodings (like lists or dictonaries)
 */

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

class BElement {
    private $encodedText;
    
    /**
     * Constructs a new BElement with the $bEncoding as BEncoded
     * value to be used 
     */
    public function __construct($bEncoding){
	$this->encodedText = $bEncoding;
    }
    
    /**
     * Returns a BEncoded value
     */
    public function toEncoded(){
	return $this->encodedText;
    }    
}


?>