<?php

require_once('include/config.inc');

require_once('include/error.inc');
require_once('include/template.inc');

require_once('include/database.inc');

require_once('class/user.inc');

require_once('util.inc');

session_start();
if (!array_key_exists('lang', $_SESSION)) {
	$_SESSION['lang']['id'] = 1;
	$_SESSION['lang']['charset'] = 'ISO-8859-1';
}

?>
