<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");

function print_platform_select() {
    echo "
        <select name=platform>
    ";

    $result = mysql_query("select * from platform");
    while ($platform = mysql_fetch_object($result)) {
        echo"
            <option value=$platform->name>$platform->user_friendly_name
        ";
    }
    mysql_free_result($result);
    echo "</select>\n";
}

    db_init();

    $user = get_logged_in_user();

    page_head("Problem Report Form");

    echo "
        <h3>Problem Report Form</h3>
        <form method=post action=bug_report_action.php>
    ";
    start_table();
    row2_init("Computer type", "");
    print_platform_select();
    echo" </td></tr>\n";
    row2("Problem description",
        "<textarea name=problem rows=10 cols=80></textarea>"
    );
    row2("",
        "<input type=submit value=\"Submit problem report\">"
    );
    end_table();
    echo" </form> ";
    page_tail();

?>
