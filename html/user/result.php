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

require_once("../inc/util.inc");
require_once("../inc/result.inc");

$x = get_int("resultid", true);
if ($x) {
    $result = BoincResult::lookup_id($x);
} else {
    $x = get_str("result_name");
    $result = BoincResult::lookup_name($x);
}

if (!$result) {
    error_page(tra("No such task:")." ".htmlspecialchars($x));
        // the htmlspecialchars prevents XSS
}
page_head("Job instance ".htmlspecialchars($x));
show_result($result);
page_tail();

?>
