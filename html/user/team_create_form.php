<?php

require_once("util.inc");
require_once("login.inc");
require_once("team.inc");
db_init();

$user = get_user_from_cookie();

if ($user == NULL) {
    print_login_form();
} else {
    print_team_create_form();
}
?>
