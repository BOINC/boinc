<?php

require_once("util.inc");
require_once("db.inc");
require_once("project.inc");
require_once("profile.inc");

// TODO: Manually limit max size of stored "big" image.
// Would just need to make getImages scale the "big"
// image if it was above a particular size.

// TODO: Allow users to delete their profiles.
// Perhaps an empty profile submission (including selection
// of the "delete image" checkbox) should be considered
// a delete action for the profile itself?

db_init();
$user = get_logged_in_user(true);
show_profile_creation_page($user);

?>