<?
require_once("../inc/db.inc");

set_time_limit(0);

db_init();

function clean_user($user) {
    if ($user->name != strip_tags($user->name)) {
        $x = strip_tags($user->name);
        echo "ID: $user->id
name: $user->name
stripped name: $x
email: $user->email_addr
-----
";
        $x = boinc_real_escape_string($x);
        $x = trim($x);
        $query = "update user set name='$x' where id=$user->id";
        $retval = mysql_query($query);
        echo $query;
    }
}

$result = mysql_query("select id, name, email_addr from user");
while ($user = mysql_fetch_object($result)) {
    clean_user($user);
}

?>
