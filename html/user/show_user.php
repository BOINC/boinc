<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// This page shows basic account details about a given user
// The page can be output as either XML or HTML.
// If output as xml the user may optionally
// also get a list of hosts in case the user provides his/her authenticator.
// Object-caching and full-file caching is used to speed up queries
// for data from this page.


require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/xml.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/user.inc");
require_once("../inc/forum.inc");
require_once("../project/project.inc");

$auth = get_str("auth", true);
if (!$auth) {
    $id = get_int("userid");
}
$format = get_str("format", true);

if ($format=="xml"){
    // don't do caching for XML
    xml_header();
    $retval = db_init_xml();
    if ($retval) xml_error($retval);
    if ($auth){
        $user = BoincUser::lookup_auth($auth);
        $show_hosts = true;
    } else {
        $user = BoincUser::lookup_id($id);
        $show_hosts = false;
    }
    if (!$user) xml_error(ERR_DB_NOT_FOUND);

    show_user_xml($user, $show_hosts);
} else {
    // The page may be presented in many different languages,
    // so here we cache the data instead
    //
    $cache_args="userid=".$id;
    $cached_data = get_cached_data(USER_PAGE_TTL, $cache_args);
    if ($cached_data){
        // We found some old but non-stale data, let's use it
        $data = unserialize($cached_data);
        $user = $data->user;
        $community_links = $data->clo;
    } else {
        // No data was found, generate new data for the cache and store it
        $user = BoincUser::lookup_id($id);
        if (!$user) {
            error_page("No such user $id");
        }
        if (strstr($user->authenticator, "deleted")) {
            error_page("No such user");
        }
        BoincForumPrefs::lookup($user);
        $user = @get_other_projects($user);
        $community_links =  get_community_links_object($user);

        $data = new StdClass;
        $data->user = $user;
        $data->clo = $community_links;
        set_cached_data(USER_PAGE_TTL, serialize($data), $cache_args);
    }
    if (!$user->id) {
        error_page("No such user");
    }

    $logged_in_user = get_logged_in_user(false);
    $myself = $logged_in_user && ($logged_in_user->id == $user->id);

    page_head($user->name);
    grid(
        false,
        function() use ($data, $myself) {
            panel(
                tra("Account information"),
                function() use ($data) {
                    start_table("table-striped");
                    show_user_summary_public($data->user);
                    project_user_summary($data->user);
                    end_table();
                }
            );
            show_other_projects($data->user, $myself);
        },
        function() use ($data, $logged_in_user) {
            panel(
                tra("Community"),
                function() use ($data, $logged_in_user) {
                    start_table("table-striped");
                    show_badges_row(true, $data->user);
                    if (!DISABLE_PROFILES) {
                        show_profile_link($data->user);
                    }
                    community_links($data->clo, $logged_in_user);
                    end_table();
                }
            );
        }
    );
    page_tail(true);
}

?>
