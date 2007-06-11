<?php
/**
 * A BInteger takes care of the BEncoding and BDecoding of integers for the 
 * Bittorrent features.
 * An integer starts with "i", has the value and then ends with "e".
 */
 
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
 
class BInteger {
    private function __construct(){
    }
    
    /**
     * Returns the ordinary value decoded from $str
     * @throws an IllegalArgumentException in the case
     * that the string is malformed.
     */
    public static function toInteger($str){
        if (substr($str, 0, 1)!="i" || substr($str, -1, 1)!="e") throw new IllegalArgumentException("BEncoded integer value does not start with i or end with e.");
        $value = substr($str, 1,-1);
        if (!is_numeric($value)) throw new IllegalArgumentException("BEncoded string has non-numeric length specification.");
        return $value;
    }				

    /**
     * Returns a BEncoded integer value
     */
    public static function toEncoded($value){
	return "i".$value."e";
    }    
}


?>