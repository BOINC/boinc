<?php
    require_once("../inc/util.inc");
    require_once("../inc/userw.inc");
    require_once("../inc/db.inc");
   // require_once("../inc/trickle.inc");
    require_once("../inc/wap.inc");
 
    // show the home page of app user from envvar

    $userid = $_GET['id'];
    if (!$userid) {
        echo "User id (userw.php?id=###) missing!";
        exit(); // can't do much without a userid!
    }

    db_init();
    $res = mysql_query("select * from user where id = $userid") or die("error in query");
    $user = mysql_fetch_object($res) or die("error in fetch_object");
    show_user_wap($user);
    mysql_free_result($res);
?>
