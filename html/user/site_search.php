<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2016 University of California
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

require_once("../inc/util.inc");

function site_search_form($url) {
    page_head(tra("Site search"));
    echo '
        <form class="form-inline" method="get" action="https://google.com/search">
        <input type=hidden name=domains value="'.$url.'">
        <input type=hidden name=sitesearch value="'.$url.'">
        <div class="form-group">
        <input type="text" class="form-control input-sm" name="q" size="20" placeholder="keywords">
        <input class="btn btn-success form-control input-sm" type="submit" value='.tra("Search").'>
        </div>
        </form>
    ';
    page_tail();
}

site_search_form(master_url());

?>
