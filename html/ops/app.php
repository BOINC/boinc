<?php
    require_once("util.inc");
    require_once("db.inc");

    parse_str(getenv("QUERY_STRING"));

    db_init();

    $first = 1;

    print_page_header("Apps");

    $query = "select * from app";
    $english_query = "Show all apps";

    if (strlen($id)) {
        $query = append_sql_query( $query, "id = $id", $first );
        $english_query = append_sql_query( $english_query, "id is $id", $first );
        $first = 0;
    }
    echo "<p>Query is: <b>$english_query</b><p>";

    $result = mysql_query($query);
    while ($app = mysql_fetch_object($result)) {
        show_app($app);
    }

    print_page_end();
?>
