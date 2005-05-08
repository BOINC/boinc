<?php
    require_once("../inc/util.inc");
    require_once("../inc/user.inc");
    require_once("../inc/db.inc");

    db_init();
    $user = get_logged_in_user(true);
    page_head("Payment success");
echo "<p>Your payment was successfully transfered to the project's account. Thank you very much!<br />
    Please ensure that your payment was registered: Go to <a href=\"home.php\">\"Your account\"</a> - the payment should show at the very bottom of the page.";
page_tail();
?>