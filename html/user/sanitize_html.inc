<?php
require_once("kses.php");

$allowed = array(
    'b' => array(),
    'i' => array(),
    'a' => array('href' => array('maxlen' => 150), 'title' => 1),
    'p' => array('align' => 1),
    'font' => array('face' => 1),
    'br' => array(),
    'img' => array('src' => 1)
);


function sanitize_html($str) {
    global $allowed;
    return kses($str, $allowed);
}

?>
