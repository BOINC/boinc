<?php
    require_once("util.inc");
    require_once("db.inc");

    parse_str(getenv("QUERY_STRING"));

    db_init();

    $first = 1;

    print_page_header("Users");

    $query = "select * from user";
    $english_query = "Show all users";

    if (strlen($team_id)) {
        $query = append_sql_query( $query, "teamid = $team_id", $first );
        $english_query = append_sql_query( $english_query, "team is " . team_name_by_id($team_id), $first );
        $first = 0;
    }

    echo "<p>Query is: <b>$english_query</b><p>";

    $result = mysql_query($query);
    while ($user = mysql_fetch_object($result)) {
        show_user($user);
    }

    print_page_end();
?>
