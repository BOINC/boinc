<?php

require_once("profile.inc");

$user = get_logged_in_user(true);
show_profile_creation_page($user);

?>
