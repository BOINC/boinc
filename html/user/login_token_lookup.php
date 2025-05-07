<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2017 University of California
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

// RPC handler for looking up a login token

require_once("../inc/boinc_db.inc");
require_once("../inc/xml.inc");

function main() {
    $config = get_config();
    $user_id = get_str("user_id");
    $token = get_str("token");
    $user = BoincUser::lookup_id($user_id);
    if (!$user) {
        xml_error("user not found");
    }
    if ($user->login_token != $token) {
        xml_error("bad token");
    }
    if (time() - $user->login_token_time > 86400) {
        xml_error("token timed out");
    }
    $uname = htmlentities($user->name);
    echo "<login_token_reply>\n";
    if (parse_bool($config, "account_manager")) {
        echo "   <user_name>$uname</user_name>\n";

        // the following for pre-7.12 clients; can be removed later
        //
        echo "   <login_name>$user->email_addr</login_name>\n";
        echo "   <passwd_hash>$user->passwd_hash</passwd_hash>\n";

        // the following for later clients
        //
        echo "   <authenticator>$user->authenticator</authenticator>\n";
    } else {
        // the following for pre-7.12 clients; remove soon
        //
        echo "   <authenticator>$user->authenticator</authenticator>\n";
        echo "   <user_name>$uname</user_name>\n";
    }
    if ($user->teamid) {
        $team = BoincTeam::lookup_id($user->teamid);
        if ($team) {
            $tname = htmlentities($team->name);
            echo "    <team_name>$tname</team_name>\n";
        }
    }
    echo "</login_token_reply>\n";
}

main();

?>
