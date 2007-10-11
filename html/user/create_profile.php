<?php

require_once("../inc/profile.inc");

db_init();

$user = get_logged_in_user(true);
if ($user->total_credit > 0) {
  show_profile_creation_page($user);
} else {
  page_head("Not available");
  echo "You must have returned results and received credit
        before you can create a profile.
  ";
  page_tail();
}

?>
