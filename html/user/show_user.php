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

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

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
        $user = lookup_user_auth($auth);        
        $show_hosts = true;
    } else {
        $user = lookup_user_id($id);
        $show_hosts = false;
    }
    if (!$user) xml_error(-136);

    show_user_xml($user, $show_hosts);
} else {
    db_init();  // need to do this in any case,
        // since show_user_summary_public() etc. accesses DB

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
        $user = lookup_user_id($id);
        BoincForumPrefs::lookup($user);
        $user = @get_other_projects($user);
        $community_links =  get_community_links_object($user);

        $data->user = $user;
        $data->clo = $community_links;
        set_cached_data(USER_PAGE_TTL, serialize($data), $cache_args);
    }
    if (!$user->id) {
        error_page("No such user");
    }

    $logged_in_user = get_logged_in_user(false);

    page_head($user->name);
    start_table();
    echo "<tr><td valign=top>";
    start_table();
    show_user_summary_public($user);
    end_table();
    project_user_summary($user);
    show_other_projects($user, false);
    echo "</td><td valign=top>";
    start_table();
    show_profile_link($user);
    community_links($community_links, $logged_in_user);
    end_table();
    echo "</td></tr></table>";
    page_tail(true);
}

?>
