<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("user.inc");

    init_session();
    db_init();
    $id = $_GET["userid"];

    $result = mysql_query("select * from user where id = $id");
    $user = mysql_fetch_object($result);
    mysql_free_result($result);

    if ($user) {
      	page_head("Account data for $user->name");
        start_table();
    	show_user_summary_public($user);
        end_table();
        page_tail();
    } else {
      	page_head("Can't find user");
	echo "There is no account with that id.\n<p>";
        page_tail();
    }
?>
