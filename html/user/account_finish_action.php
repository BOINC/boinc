<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/email.inc");

db_init();

function show_error($str) {
    page_head("Can't update account");
    echo "$str<br>\n";
    echo mysql_error();
    echo "<p>Click your browser's <b>Back</b> button to try again.\n<p>\n";
    page_tail();
    exit();
}

$auth = process_user_text(post_str("auth"));

$name = process_user_text(post_str("name"));
if (strlen($name)==0) {
    show_error("You must supply a name for your account");
}
if ($new_name != strip_tags($new_name)) {
    show_error("HTML tags not allowed in name");
}

$country = post_str("country");
if (!is_valid_country($country)) {
    show_error( "bad country");
}

$postal_code = strip_tags(process_user_text(post_str("postal_code")));

$query = "update user set name='$name', country='$country', postal_code='$postal_code' where authenticator='$auth'";
$retval = mysql_query($query);
if (!$retval) {
    show_error("database error");
}

Header("Location: home.php");

?>
