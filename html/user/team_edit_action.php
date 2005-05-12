<?php

    require_once("../inc/db.inc");
    require_once("../inc/util.inc");
    require_once("../inc/team.inc");

    db_init();
    $user = get_logged_in_user();
    $teamid = $_POST["teamid"];

    $team = lookup_team($teamid);
    require_founder_login($user, $team);

    $team_url = process_user_text(strip_tags(post_str("url", true)));
    $x = strstr($team_url, "http://");
    if ($x) {
        $team_url = substr($team_url, 7);
    }
    $team_name = process_user_text(strip_tags(post_str("name")));
    $team_name_lc = strtolower($team_name);
    $team_name_html = process_user_text(post_str("name_html", true)); //Do we really not want to
    $team_description = process_user_text(post_str("description", true)); //scrub out bad HTML tags?
	$type = process_user_text(post_str("type", true));
	$country = process_user_text(post_str("country", true));

	if (! is_numeric($teamid)) {
		error_page("Team ID must be numeric.");
	}

	if (strlen($team_name) == 0) { // Should be caught up with the post_str("name"),
		error_page("Must specify team name"); // but you can never be too safe.
	}

    $query_team_table = sprintf(
        "update team set name = '%s',
        name_lc = '%s',
        name_html = '%s',
        url = '%s',
        description = '%s',
        type = %d,
        country='%s'
        where id = %d",
        $team_name,
        $team_name_lc,
        $team_name_html,
        $team_url,
        $team_description,
        $type,
        $country,
        $team->id
    );
    $result = mysql_query($query_team_table);
    if ($result) {
        Header("Location: team_display.php?teamid=$team->id");
    } else {
    	error_page("Could not update team - please try later.");
    }

?>
