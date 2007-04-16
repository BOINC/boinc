<?php

// look for an element in some XML text
//
function parse_element($xml, $tag) {
    $element = null;
    $x = strstr($xml, $tag);
    if ($x) {
        $y = substr($x, strlen($tag));
        $n = strpos($y, "<");
        if ($n) {
            $element = substr($y, 0, $n);
        }
    }
    return $element;
}

// look for a particular element in the config.xml file
//
function parse_config($tag) {
    $element = null;
    $fp = fopen("config.xml", "r");
    if (!$fp) return false;
    while (1) {
        $buf = fgets($fp, 1024);
        if ($buf == null) break;
        $element = parse_element($buf, $tag);
        if ($element) break;
    }
    fclose($fp);
    return $element;
}

function db_init() {
    $retval = mysql_pconnect();
    if (!$retval) return false;
    $db_name = parse_config("<db_name>");
    if (!$db_name) return false;
    return mysql_select_db($db_name);
}

?>
