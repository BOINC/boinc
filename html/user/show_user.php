<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

    init_session();
    db_init();
    $id = $HTTP_GET_VARS["id"];

    $result = mysql_query("select * from user where id = $id");
    $user = mysql_fetch_object($result);
    mysql_free_result($result);

    if ($user) {
      	page_head("User stats for $user->name");
    	show_user_stats($user);
        page_tail();
    } else {
      	page_head("Can't find user");
	echo "There is no account with that id.\n<p>";
        page_tail();
    }
?>
