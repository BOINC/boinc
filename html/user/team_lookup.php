<?php

    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/team.inc");

    db_init();
    init_session();

    $team_name = $_GET["team_name"];
    $name_lc = strtolower($team_name);
    $name_lc = escape_pattern($name_lc);
     
    $query = "select * from team where name like '%$name_lc%'";
    $result_list = mysql_query($query);
    page_head("Search Results");
    if ($result_list) {
        $total = 0;
        echo "<h2>Search results for '$team_name'</h2>";
        echo "<p>";
        echo "You may view these teams' members, statistics, and information.";
        echo "<ul>";
        while ($team_possible = mysql_fetch_object($result_list)) {
            if ($total >= 100) {
                $too_many = true;
                break;
            }
            echo "<li>";
            echo "<a href=team_display.php?teamid=$team_possible->id>";
            echo "$team_possible->name</a></li>";
            $total++;
        }
        echo "</ul>";
        if ($too_many) {
            echo "
                More than 100 teams match your search.
                The first 100 are shown.<br>
            ";
        }
    }
    echo "End of results<br>";
    echo "If you cannot find the team you are looking for, you may create a team ";
    echo "by clicking <a href=team_create_form.php>here</a>.";
    page_tail();

?>
