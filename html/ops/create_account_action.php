<?
    require_once("util_ops.inc");
    require_once("../html_user/util.inc");

    db_init();
    $email_addr = $_GET["email_addr"];

    // see if email address is taken
    $query = "select * from user where email_addr='$email_addr'";
    $result = mysql_query($query);
    $user = mysql_fetch_object($result);
    if ($user) {
        echo "That email address ($email_addr) is taken";
        exit();
    }
    $authenticator = random_string();
    $munged_email_addr = munge_email_addr($email_addr, $authenticator);
    $user_name = $_GET["user_name"];
    $t = time();
    $query = "insert into user (create_time, email_addr, name, authenticator) values ($t, '$munged_email_addr', '$user_name', '$authenticator')";
    $result = mysql_query($query);
    if (!$result) {
        echo "couldn't create account:".mysql_error();
        exit();
    }
    
    $url = URL_BASE."/account_created.php?email_addr=".urlencode($email_addr);
    mail($email_addr, PROJECT. " account created",
"This email confirms the creation of your ".PROJECT." account.

".PROJECT." URL:         ".MASTER_URL."

Your account key:          $authenticator\n

Please save this email.
You will need your account key to log in to the ".PROJECT." web site.

To finish registration, go to
$url
"
);

?>
