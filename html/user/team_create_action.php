<?php

    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/team.inc");

    db_init();

    $user = get_logged_in_user();
    
    $name = process_user_text(strip_tags(post_str("name"))); 
    if (strlen($name) == 0) {
    	error_page("Must set team name");
    }
    $name_lc = strtolower($name);
    $url = process_user_text(strip_tags(post_str("url", true)));
    if (strstr($url, "http://")) {
    	$url = substr($url, 7);
    }
    $type = process_user_text(strip_tags(post_str("type", true))); 
    $name_html = process_user_text(post_str("name_html", true));
    $description = process_user_text(post_str("description", true));
    $country = process_user_text(post_str("country", true));
    
    $query = sprintf(
        "insert into team (userid, create_time, name, name_lc, url, type, name_html, description, country, nusers) values(%d, %d, '%s', '%s', '%s', %d, '%s', '%s', '%s', %d)",
        $user->id,
        time(),
        $name,
        $name_lc,
        $url,
        $type,
        $name_html,
        $description,
        $country,
        0
    );

    $result = mysql_query($query);
    if ($result) {
        $teamid = mysql_insert_id();
        $team_result = mysql_query("select * from team where id = $teamid");
        $new_team = mysql_fetch_object($team_result);
        mysql_free_result($team_result);
        user_join_team($new_team, $user);
        Header("Location: team_display.php?teamid=$teamid");
    } else {
    	error_page("Could not create team - please try later.  <br>" .
    			"You may need to try a diffrent team name.");
    }
?>
