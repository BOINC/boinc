<?php
/**
 * The BList class is part of the Bittorrent serverside utility classes
 * It implements a BEncoded list - which always starts with l and ends with e.
 * In PHP lists are simply non-keyed (ie. standard keyed) arrays.
 */
 
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
 
class BList {
    private function __construct(){
    }
    
    /**
     * Returns the decoded array from $str
     * @throws an IllegalArgumentException in the case
     * that the string is malformed.
     */
    public static function toList($str){
        if (substr($str, 0, 1)!="l" || substr($str, -1, 1)!="e") throw new IllegalArgumentException("BEncoded list does not start with l or end with e.");
        $value = substr($str, 1,-1);
	
	// Scan through encoded list and decode everything
	$list = array();
	$i=0; $lastI = 0;
	while ($i<strlen($value)){
	    $curChar = substr($value, $i,1);
	    if ($curChar == "i"){ // We just saw an Integer
		$endPos = strpos($value, "e", $i); // Search for the end of it
		if ($endPos===false) throw new IllegalArgumentException("BEncoded list contains non-terminated integer");
		$list[] = BInteger::toInteger(substr($value, $i, $endPos-$i+1));
		$i=$endPos+1;
	    } elseif (is_numeric($curChar)){ // A number means we just saw a string starting
		$seperatorPos = strpos($value, ":", $i); // Search for the seperator
		if ($seperatorPos===false) throw new IllegalArgumentException("BEncoded list contains malformed string with no length specified");
		$totalLength = substr($value, $i, $seperatorPos-$i);
		$list[] = BString::toString(substr($value, $i, $seperatorPos-$i+$totalLength+1));
		$i=$seperatorPos+$totalLength+1;
	    } elseif ($curChar == "d" || $curChar == "l"){ // Found a dictionary or list
		// Scan for end of it
		$listLength = BList::getListLength(substr($value, $i));
		if ($curChar == "d"){
		    $list[] = BDictionary::toArray(substr($value, $i, $listLength));
		} else {
		    $list[] = BList::toList(substr($value, $i, $listLength));
		}
		$i+=$listLength;
	    }
	    if ($i==$lastI) throw new IllegalArgumentException("BEncoded list contains malfomed or unrecognized content");
	    $lastI = $i;
	}
        return $list;
    }				

    private static function getListLength($text){
	$i = 1; $lastI = 0; $foundEnd = false;
	while ($i<strlen($text)){
	    $curChar = substr($text, $i, 1);
	    if ($curChar == "i"){
		$i = strpos($text, "e", $i);
		if ($i===false) throw new IllegalArgumentException("BEncoded sublist/dictionary integer has no end");
		$i++;
	    } elseif (is_numeric($curChar)){
		$seperatorPos = strpos($text, ":", $i); // Search for the seperator
		if ($seperatorPos===false) throw new IllegalArgumentException("BEncoded sublist/dictionary contains a string with no length specified");
		$totalLength = substr($text, $i, $seperatorPos-$i);
		$i=$seperatorPos+$totalLength+1;		
	    } elseif ($curChar == "d" || $curChar == "l"){
		$i+=BList::getListLength(substr($text, $i));
	    } elseif ($curChar == "e"){
		$foundEnd = true;
		break;
	    }
	    if ($i==$lastI) throw new IllegalArgumentException("BEncoded sublist/dictionary contains malfomed or unrecognized content");
	    $lastI = $i;
	}
	if (!$foundEnd) throw new IllegalArgumentException("BEncoded sublist/dictionary had no end");
	return $i+1;
    }

    /**
     * Returns a BEncoded list
     */
    public static function toEncoded($list){
	$values = array_values($list);
	$text = "l";
	foreach ($values as $value){
	    if (is_integer($value)){
		$text.=BInteger::toEncoded($value);
	    } elseif (is_string($value)){
		$text.=BString::toEncoded($value);
	    } elseif ($value instanceof BElement){
		$text.=$value->toEncoded();
	    }
	}
	$text.= "e";
	return $text;
    }    
}


?>