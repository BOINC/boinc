<?php
/**
 * A dictionary is a mapping from strings to elements. When BEncoded a dictionary is
 * two lists interleaved. In PHP dictionaries are represented as arrays with the 
 * string as index and value as entry at that index.
 * In BEncoded form a dictionary starts with "d" and is followed directly by the
 * contents of the interleaved data list and ends with "e".
 *
 * d3:cow3:moo4:spam4:eggse represents the dictionary { "cow" => "moo", "spam" => "eggs" } 
 */

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

class BDictionary {
    private function __construct(){
    }
    
    /**
     * Returns the decoded array from $str
     * @throws an IllegalArgumentException in the case
     * that the string is malformed.
     */
    public static function toArray($str){
        if (substr($str, 0, 1)!="d" || substr($str, -1, 1)!="e") throw new IllegalArgumentException("BEncoded dictionary does not start with d or end with e.");
	// An array is simply two lists encoded in an alternating list
        $arrays = BList::toList("l".substr($str, 1));
	
	$i=0;
	for ($i=0;$i<sizeof($arrays); $i+=2){
	    $array[$arrays[$i]] = $arrays[$i+1];
	}
        return $array;
    }				

    /**
     * Returns a BEncoded dictonary
     */
    public static function toEncoded($array){
	ksort($array);

	// An array is simply two lists encoded as an alternating list
	$list = array();
	foreach ($array as $key => $value){
	    $list[] = $key;
	    $list[] = $value;
	}
	
	return "d".substr(BList::toEncoded($list), 1);
    }    
}


?>