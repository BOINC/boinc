<?php
    require_once("../inc/util.inc");

    page_head("Team-based account creation");
    echo "
        A <b>Create team account</b> URL is shown on every team page.
        Accounts created through this URL
        <ul>
        <li> will belong to that team and
        <li> will have the same ".PROJECT." preferences as its founder.
        </ul>
        For example, you can create a team for your school or company,
        and set up your preferences to show an appropriate logo
        in the screensaver graphics.
        Then get your friends and coworkers to register for
        ".PROJECT." through the URL shown in the team page,
        and they will automatically belong to your team
        and will have the logo in their screensaver graphics.
    ";
?>
