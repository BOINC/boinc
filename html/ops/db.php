<?php
    require_once("util.inc");
    require_once("db.inc");

    db_init();

    function show_platform_table() {
        echo "<hr>Platforms";
        $result = mysql_query("select * from platform");
        while ($platform = mysql_fetch_object($result)) {
            show_platform($platform);
        }
    }

    function show_app_table() {
        echo "<hr>Apps";
        $result = mysql_query("select * from app");
        while ($app = mysql_fetch_object($result)) {
            show_app($app);
        }
    }

    function show_app_version_table() {
        echo "<hr>App versions";
        $result = mysql_query("select * from app_version");
        while ($app_version = mysql_fetch_object($result)) {
            show_app_version($app_version);
        }
    }

    function show_host_table() {
        echo "<hr>Hosts";
        $result = mysql_query("select * from host");
        while ($host = mysql_fetch_object($result)) {
            show_host($host);
        }
    }

    function show_workunit_table() {
        echo "<hr>Workunits";
        $result = mysql_query("select * from workunit");
        while ($workunit = mysql_fetch_object($result)) {
            show_workunit($workunit);
        }
    }

    function show_result_table() {
        echo "<hr>Results";
        $result = mysql_query("select * from result");
        while ($res = mysql_fetch_object($result)) {
            show_result($res);
        }
    }

    function show_user_table() {
        echo "<hr>Users";
        $result = mysql_query("select * from user");
        while ($user = mysql_fetch_object($result)) {
            show_user($user);
        }
    }

    parse_str(getenv("QUERY_STRING"));
    if ($show=="platform") {
        show_platform_table();
    } else if ($show=="app") {
        show_app_table();
    } else if ($show=="app_version") {
        show_app_version_table();
    } else if ($show=="host") {
        show_host_table();
    } else if ($show=="workunit") {
        show_workunit_table();
    } else if ($show=="result") {
        show_result_table();
    } else if ($show=="user") {
        show_user_table();
    } else {
        echo "<br><a href=db.php?show=platform>Platform</a>";
        echo "<br><a href=db.php?show=app>App</a>";
        echo "<br><a href=db.php?show=app_version>App Version</a>";
        echo "<br><a href=db.php?show=host>Host</a>";
        echo "<br><a href=db.php?show=workunit>Workunit</a>";
        echo "<br><a href=db.php?show=result>Result</a>";
        echo "<br><a href=db.php?show=user>User</a>";
    }
?>
