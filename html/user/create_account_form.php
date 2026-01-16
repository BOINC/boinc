<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

require_once('../inc/boinc_db.inc');
require_once('../inc/util.inc');
require_once('../inc/account.inc');
require_once('../inc/countries.inc');
require_once('../inc/translation.inc');
require_once('../inc/recaptchalib.inc');

check_get_args(array("next_url", "teamid"));

$next_url = sanitize_local_url(get_str('next_url', true));

$config = get_config();
if (parse_bool($config, "disable_account_creation")) {
    error_page("This project has disabled account creation");
}

if (parse_bool($config, "no_web_account_creation")) {
    error_page("This project has disabled Web account creation");
}

page_head(
    tra("Create account"), null, null, null, boinc_recaptcha_get_head_extra()
);

if (!NO_COMPUTING) {
    echo "<p class=\"lead\">"
        .tra(
            "If you already have an account and want to run %1 on this computer, %2 go here %3.",
            PROJECT,
            '<a href=join.php>',
            '</a>'
        )
        ."</p>
    ";
}

$teamid = get_int("teamid", true);
if ($teamid) {
    $team = BoincTeam::lookup_id($teamid);
    $user = BoincUser::lookup_id($team->userid);
    if (!$user) {
        error_page("Team $team->name has no founder");
        $teamid = 0;
    } else {
        echo "<b>".tra("This account will belong to the team %1 and will have the project preferences of its founder.", "<a href=\"team_display.php?teamid=$team->id\">$team->name</a>")."</b><p>";
    }
}

form_start("create_account_action.php","post");
create_account_form($teamid, $next_url);
if (recaptcha_public_key()) {
    form_general("", boinc_recaptcha_get_html(recaptcha_public_key()));
}
form_submit(tra("Create account"));
form_end();

page_tail();

?>
