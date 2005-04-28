<?php
require_once("../inc/kses.php");

$allowed = array(
    'b' => array(),
    'i' => array(),
    'a' => array('href' => array('maxlen' => 150), 'title' => 1),
    'p' => array('align' => 1),
    'font' => array('face' => 1),
    'br' => array(),
    'strong' => array(),
    'ul' => array(),
    'ol' => array(),
    'li' => array(),
    'pre' => array(),
    'img' => array('src' => 1, 'border' => 1, 'height' => array('maxval' => 450)),
    'blockquote' => array(),
    'u' => array()
);


function sanitize_html($str) {
    global $allowed;
    return kses($str, $allowed);
}

?>
