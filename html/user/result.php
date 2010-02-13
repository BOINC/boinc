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

// show a result

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/result.inc");

$resultid = get_int("resultid");
$result = lookup_result($resultid);
if (!$result) {
    error_page(tra("No such task"));
}
page_head(tra("Task %1", $resultid));
show_result($result);
page_tail();

?>
