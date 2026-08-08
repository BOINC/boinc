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

// cancel a WU:
// - mark unsent results as OVER, outcome DIDNT_NEED
// - set CANCELLED bit in WU error mask
//

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");

db_init();

$wuid1 = get_int('wuid1');
$wuid2 = get_int('wuid2');
$unsent_only = get_str('unsent_only', true);

if ($wuid1<1 || $wuid2<$wuid1) {
    admin_error_page(
        "<h2>Workunit IDs fail to satisfy the conditions:<p> 0 < ID1 <= ID2"
    );
}

if ($unsent_only) {
    cancel_wus_if_unsent($wuid1, $wuid2);
} else {
    cancel_wus($wuid1, $wuid2);
}

admin_page_head("Cancel jobs");
echo " canceled jobs with $wuid1 <= workunit ID <= $wuid2</h2>";
admin_page_tail();

?>
