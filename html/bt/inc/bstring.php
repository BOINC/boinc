<?php
/**
 * The BString class represents BEncoded strings for the serverside Bittorrent system.
 * A BEncoded string always starts with the length, then a ":" and then the actual data.
 * Strings are native in PHP.
 */

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
 
class BString {
    private function __construct(){
    }
    
    /**
     * Returns the ordinary string decoded from $str
     * @throws an IllegalArgumentException in the case
     * that the string is malformed (ie. has more/less chars than specified
     * or does not correctly define a BEncoded string)
     */
    public static function toString($str){
	list($length, $data) = explode(":", $str);
	if (!is_numeric($length)) throw new IllegalArgumentException("BEncoded string has non-numeric length specification.");
	if ($length != strlen($data)) throw new IllegalArgumentException("BEncoded string length does not match actual length of string.");
	return $data;
    }

    /**
     * Returns a BEncoded string
     */
    public static function toEncoded($str){
	return strlen($str).":".$str;
    }    
}


?>