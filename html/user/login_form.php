<?php
    require_once("util.inc");

    $next_url = $_GET["next_url"];
    
    $authenticator = init_session();
    db_init();

    $user = get_user_from_auth($authenticator);

    page_head("Log in", $user);
    print_login_form_aux($next_url);
    if ($user) {
        echo "<br><a href=logout.php>Log out</a>";
    }

    page_tail();
?>
