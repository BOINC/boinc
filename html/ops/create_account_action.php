<?php
    require_once("../inc/util_ops.inc");
    require_once("../inc/util.inc");

    db_init();
    $email_addr = trim($_GET["email_addr"]);
    $email_addr = strtolower($email_addr);

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
    $cross_project_id=random_string();
    $t = time();
    $query = "insert into user (create_time, email_addr, name, authenticator, cross_project_id) values ($t, '$munged_email_addr', '$user_name', '$authenticator', '$cross_project_id')";
    $result = mysql_query($query);
    if (!$result) {
        echo "couldn't create account:".mysql_error();
        exit();
    }
    
   $url = URL_BASE."/account_created.php?email_addr=".urlencode($email_addr);
   echo "
          <h2>Account created for:<br/>
          $user_name<br/>
          at email address:<br/>
          $email_addr</h2><br/>
          Note: The user (not YOU, the admin) must confirm by visiting:<br/>
          <a href=\"$url\"> $url </a>
         ";


    mail($email_addr, PROJECT. " account created",
"This email confirms the creation of your ".PROJECT." account.

".PROJECT." URL:         ".MASTER_URL."

Your account ID:          $authenticator\n

Please save this email.
You will need your account Id to log in to the ".PROJECT." web site.

To finish registration, you must confirm. Please go to:
$url
"
);



?>
