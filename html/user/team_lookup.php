<?php

    require_once("util.inc");
    require_once("team.inc");
    require_once("db.inc");

    db_init();

    $team_name = $HTTP_POST_VARS["team_name"];
    $words = preg_split("/[\s,]+/", $team_name);
    $length = count($words);
    $name_lc = strtolower($team_name);
     
    $query = sprintf(
        "select * from team where name_lc like '%s'",
        "%$name_lc%"
    );
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
            echo "<a href=team_display.php?id=$team_possible->id>";
            echo "$team_possible->name</a></li>";
            $total++;
        }
        echo "</ul>";
        if ($too_many) {
            echo "This is only a partial list of the possible teams you ";
            echo "were searching for. You will need to narrow your search ";
            echo "criteria to get more accurate results.<br>";
        }
    }
    echo "End of results<br>";
    echo "If you cannot find the team you are looking for, you may create a team ";
    echo "by clicking <a href=team_create_form.php>here</a>.";
    page_tail();

?>
