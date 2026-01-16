<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California, 2011 Daniel Lombraña González
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

NOT FINISHED.   DON'T USE

require 'openid.php';
include_once("../inc/boinc_db.inc");
include_once("../inc/util.inc");
include_once("../inc/email.inc");
include_once("../inc/user.inc");

function show_error($str) {
    page_head("Can't create account");
    echo "$str<br>\n";
    page_tail();
    exit();
}

try {
    $openid = new LightOpenID;
    echo "<pre>";
    if(!$openid->mode) {
        if(isset($_POST['openid_identifier'])) {
            $openid->identity = $_POST['openid_identifier'];
            $openid->required = array('namePerson/friendly', 'contact/email');
            $openid->optional = array('contact/country/home');
            header('Location: ' . $openid->authUrl());
        }
        if(isset($_GET['openid_identifier'])) {
            $openid->identity = $_GET['openid_identifier'];
            $openid->required = array('namePerson/friendly', 'contact/email');
            $openid->optional = array('contact/country/home');
            header('Location: ' . $openid->authUrl());
        }
    } elseif($openid->mode == 'cancel') {
        echo 'User has canceled authentication!';
    } else {
        echo 'User ' . ($openid->validate() ? $openid->identity . ' has ' : 'has not ') . 'logged in.';
        // Create the user in the DB
        $data = $openid->getAttributes();
        $email_addr = $data['contact/email'];
        $email_addr = strtolower($email_addr);
        $user_name = $data['namePerson/friendly'];


        $config = get_config();
        if (parse_bool($config, "disable_account_creation")
            || parse_bool($config, "no_web_account_creation")
        ) {
            error_page("Account creation is disabled");
        }

        // see whether the new account should be pre-enrolled in a team,
        // and initialized with its founder's project prefs
        //
        //$teamid = post_int("teamid", true);
        //if ($teamid) {
        //    $team = BoincTeam::lookup_id($teamid);
        //    $clone_user = BoincUser::lookup_id($team->userid);
        //    if (!$clone_user) {
        //        echo "User $userid not found";
        //        exit();
        //    }
        //    $project_prefs = $clone_user->project_prefs;
        //} else {
        //    $teamid = 0;
        //    $project_prefs = "";
        //}

        //if(defined('INVITE_CODES')) {
        //    $invite_code = post_str("invite_code");
        //    if (strlen($invite_code)==0) {
        //        show_error(tra("You must supply an invitation code to create an account."));
        //    }
        //    if (!preg_match(INVITE_CODES, $invite_code)) {
        //        show_error(tra("The invitation code you gave is not valid."));
        //    }
        //}

        $new_name = $data['namePerson/friendly'];
        if (!is_valid_user_name($new_name, $reason)) {
            show_error($reason);
        }
        $new_email_addr = $data['contact/email'];
        $new_email_addr = strtolower($new_email_addr);
        if (!is_valid_email_syntax($new_email_addr)) {
            show_error("Invalid email address:
                you must enter a valid address of the form
                name@domain"
            );
        }
        $user = BoincUser::lookup_email_addr($new_email_addr);
        if (!$user) {
            $passwd_hash = random_string();

            $country = $data['contact/country/home'];
            if ($country == "") {
                $country = "International";
            }
            if (!is_valid_country($country)) {
                echo "bad country";
                exit();
            }

            $user = make_user(
                $new_email_addr, $new_name, $passwd_hash,
                $country, '', $project_prefs="", $teamid=0
            );
            if (!$user) {
                show_error("Couldn't create account");
            }

            if(defined('INVITE_CODES')) {
                error_log("Account '$new_email_addr' created using invitation code '$invite_code'");
            }
        }

        // Log-in user in the web

        // In success case, redirect to a fixed page so that user can
        // return to it without getting "Repost form data" stuff

        $next_url = post_str('next_url', true);
        $next_url = sanitize_local_url($next_url);
        if ($next_url) {
            Header("Location: ".url_base()."$next_url");
        } else {
            Header("Location: ".url_base().HOME_PAGE);
            send_cookie('init', "1", true);
            send_cookie('via_web', "1", true);
        }
        send_cookie('auth', $user->authenticator, true);

    }
} catch(ErrorException $e) {
    error_page('bad request');
}


?>
