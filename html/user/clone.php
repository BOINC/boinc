<?php
    require_once("db.inc");
    require_once("util.inc");

    db_init();
    $user = get_logged_in_user();

    page_head("Create a custom signup page");
    echo "
        <h2>Create a custom signup page for ".PROJECT."</h2>
        If you like, you can create a custom signup page
        for ".PROJECT.".
        The URL for this page is
        <br>
        <pre>
        ".MASTER_URL."create_account_form.php?userid=$user->id
        </pre>
        Accounts created through this URL will
        be initialized similarly to yours.
        Specifically:
        <ul>
        <li> They will belong to the same team as you;
        <li> They will have the same ".PROJECT." preferences as you.
        </ul>
        For example, you can create a team for your school or company,
        and set up your preferences to show an appropriate logo
        in the screensaver graphics.
        Then get your friends and coworkers to register for
        ".PROJECT." through the URL shown above,
        and they will automatically belong to your team
        and will see the same screensaver graphics.
    ";
?>
