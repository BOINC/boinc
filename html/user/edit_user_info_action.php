<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

    $authenticator = init_session();
    db_init();
    $user = get_user_from_auth($authenticator);
    require_login($user);

    page_head("User info update");
    $name = $HTTP_POST_VARS["user_name"];
    $country = $HTTP_POST_VARS["country"];
    $postal_code = $HTTP_POST_VARS["postal_code"];

    $result = mysql_query("update user set name='$name', country='$country', postal_code='$postal_code' where id=$user->id");
    if ($result) {
        echo "User info updated successfully.";
    } else {
        echo "Couldn't update user info.";
    }

    page_tail();

?>
