<?php

require_once("../inc/util_ops.inc");
require_once("../inc/user.inc");
require_once("../inc/user_util.inc");

admin_page_head("Add User");

$email = get_str('email', true);
$name  = get_str('name', true);

if ($email && $name) {
    $user = make_user_ldap(urldecode($email), urldecode($name));
    if ($user) {
        echo "User created with ID ";
        echo $user->id;
    } else {
        echo "ERROR: couldn't create user, probably email address already in DB";
    }
} else {
    $page = $_SERVER["REQUEST_URI"];
    echo "<form action=\"$page\" method=\"get\" enctype=\"application/x-www-form-urlencoded\">\n";
    echo '<p>User name: ';
    echo '<input name="name" type="text" size="100" maxlength="200">';
    echo '</p><p>Email address:: ';
    echo '<input name="email" type="text" size="100" maxlength="200">';
    echo '</p>';
    echo '<input type="submit" value="Create user">';
    echo "</form>\n";
}
?>
