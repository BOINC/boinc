<?php
    require_once("../inc/db.inc");
    require_once("../inc/user.inc");
    require_once("../inc/util.inc");

    db_init();
    $user = get_logged_in_user();

    $name = $HTTP_POST_VARS["user_name"];
    $url = $HTTP_POST_VARS["url"];
    $country = $HTTP_POST_VARS["country"];
    $postal_code = $HTTP_POST_VARS["postal_code"];
    //$signature = $HTTP_POST_VARS["signature"];

    $result = mysql_query("update user set name='$name', url='$url', country='$country', postal_code='$postal_code' where id=$user->id");
    if ($result) {
        Header("Location: home.php");
    } else {
        page_head("User info update");
        echo "Couldn't update user info.";
        page_tail();
    }

?>
