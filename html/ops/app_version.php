<?php
    require_once("util.inc");
    require_once("db.inc");

    parse_str(getenv("QUERY_STRING"));

    db_init();

    $first = 1;

    print_page_header("App Versions");

    $query = "select * from app_version";
    $english_query = "Show all application versions";

    if (strlen($plat_id)) {
        $query = append_sql_query( $query, "platformid = $plat_id", $first );
        $english_query = append_sql_query( $english_query, "platform is ".platform_name_by_id($plat_id), $first );
        $first = 0;
    }
    if (strlen($app_id)) {
        $query = append_sql_query( $query, "appid = $app_id", $first );
        $english_query = append_sql_query( $english_query, "application is ".app_name_by_id($app_id), $first );
        $first = 0;
    }

    printf(
        "<form method=get action=app_version.php>\n"
        . "<p>\n"
        . "<input type=checkbox name=show_xml_docs"
        . (strlen($show_xml_docs) ? " checked" : "") . ">"
        . "Show XML Docs\n"
        . "<p>\n"
        . "<input type=submit value=\"Query\">\n"
        . "</form>"
    );

    echo "<p>Query is: <b>$english_query</b><p>";

    $result = mysql_query($query);
    while ($app_version = mysql_fetch_object($result)) {
        show_app_version($app_version,$show_xml_docs);
    }

    print_page_end();
?>
