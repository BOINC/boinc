<?php
    require_once("util.inc");
    require_once("user.inc");
    require_once("db.inc");

    $authenticator = init_session();
    db_init();
    $authenticator = trim($_POST["authenticator"]);
    //$email_addr = $_POST["email_addr"];
    //$password = $_POST["password"];
    if (strlen($authenticator)) {
        $query = "select * from user where authenticator='$authenticator'";
    //} else if (strlen($email_addr)) {
    //    $query = "select * from user where email_addr='$email_addr'";
    //} else {
    //    echo "NO SELECTION";
    }
    $result = mysql_query($query);
    if ($result) {
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
    }
    if (!$user) {
        page_head("Log in");
        echo "
            We have no account with the account key '$authenticator'.
            <br>Click <b>Back</b> to try again.
        ";
        page_tail();
    //} else if (strlen($password)) {
    //    page_head("Log in");
    //    if ($user->web_password != $_POST["existing_password"]) {
    //        echo "Bad password.";
    //    }
    } else {
        if (split_munged_email_addr($user->email_addr, $authenticator, $email)) {
            mysql_query("update user set email_addr='$email' where id=$user->id");
        }
        $_SESSION["authenticator"] = $user->authenticator;
        $next_url = $_POST["next_url"];
        if (strlen($next_url) == 0) $next_url = "home.php";
        Header("Location: $next_url");
    }
?>
