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

// use this to delete workunits that are not wanted

require_once("../inc/util_ops.inc");

admin_page_head("Cancel workunit(s)");

echo "<form action=\"cancel_wu_action.php\">
";
echo "<p>
	This form may be used to cancel unnecessary or unwanted workunits.
    We recommend that you stop the project before doing this.
    Note that the workunits and their corresponding
    results (if any) are NOT removed from the database.
    Instead, they are marked as 'no longer needed'.
    In most cases you should probably only remove workunits whose results
    are all unsent,
    since otherwise a user will not get credit
    for a result that they might return.
    <p>
";
// TODO: David, a query that shows all workunits that do not have all results unsent is:
// select distinct workunit.id,workunit.name from workunit join result where workunit.id=result.workunitid and result.server_state!=2 order by workunit.id
// What is the inverse of this query?  Ie select all workunits all of whose results are unsent.  This would
// be useful to incorporate into this page.

start_table();
row2("First Workunit (ID) to cancel", "<input size=\"32\" name=\"wuid1\"");
row2("Last  Workunit (ID) to cancel", "<input size=\"32\" name=\"wuid2\"");
row2("", "<input type=\"submit\" value=\"CANCEL WORKUNITS\">");
end_table();
echo "
    </form>
";
admin_page_tail();
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
