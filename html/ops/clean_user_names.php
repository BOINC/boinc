<?
// remove HTML from user names

require_once("../inc/db.inc");

set_time_limit(0);

db_init();
$result = mysql_query("select id, name, email_addr from user");
while ($user = mysql_fetch_object($result)) {
    if ($user->name != strip_tags($user->name)) {
        $x = strip_tags($user->name);
        echo "ID: $user->id
name: $user->name
stripped name: $x
email: $user->email_addr
-----
";

        mysql_query("update user set name='$x' where id=$user->id");
    }
}

?>
