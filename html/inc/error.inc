<?php

require_once('template.inc');

function doError($msg, $info = '') {
	doHeader('Error');

	echo '<p class="title">'.$msg.'</p>';
	echo '<p>The error has been recorded, and the website administrator will investigate the issue as soon as possible.  We sincerely apologize for the inconvenience.</p>';

	doFooter();
	exit();
}

?>