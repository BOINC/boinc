<?php

require_once('../inc/forum.inc');
require_once('../inc/util.inc');

if (array_key_exists('id', $_GET) && $_GET['id'] > 0) {
	$lang = getLanguage($_GET['id']);
	$_SESSION['lang']['id'] = $lang->langID;
	$_SESSION['lang']['charset'] = $lang->charset;
}

header('Location: http://'.$_SERVER['HTTP_HOST'].substr($_SERVER['PATH_INFO'], 0, strrpos($_SERVER['PATH_INFO'], '/')).'/index.php');
?>
