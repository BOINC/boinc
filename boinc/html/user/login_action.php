<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/user.inc");

    init_session();
    db_init();
    $authenticator = trim($_POST["authenticator"]);
    if (strlen($authenticator)) {
        $query = "select * from user where authenticator='$authenticator'";
    }
    $result = mysql_query($query);
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    if (!$user) {
        page_head("Log in");
        echo "
            We have no account with the account ID '$authenticator'.
            <br>Click <b>Back</b> to try again.
        ";
        page_tail();
    } else {
        if (split_munged_email_addr($user->email_addr, $authenticator, $email)) {
            mysql_query("update user set email_addr='$email' where id=$user->id");
        }
        $_SESSION["authenticator"] = $authenticator;
        $next_url = $_POST["next_url"];
        if (strlen($next_url) == 0) $next_url = "home.php";
        Header("Location: $next_url");
        if ($_POST['send_cookie']) {
            setcookie('auth', $authenticator, time()+3600*24*365);
        }
    }
?>
