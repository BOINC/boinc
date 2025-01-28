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

admin_page_head("Cancel jobs");

echo "<form action=\"cancel_wu_action.php\">
";
echo "<p>
    We recommend that you stop the project before canceling jobs.
    <p>
    Canceled jobs are not removed from the database.
    Instead, they are marked as 'no longer needed'.
    <p>
    <p>
";
// TODO: David, a query that shows all workunits that do not have all results unsent is:
// select distinct workunit.id,workunit.name from workunit join result where workunit.id=result.workunitid and result.server_state!=2 order by workunit.id
// What is the inverse of this query?  Ie select all workunits all of whose results are unsent.  This would
// be useful to incorporate into this page.

start_table();
row2("Workunit ID of first job to cancel", "<input size=\"32\" name=\"wuid1\"");
row2("Workunit ID of last job to cancel", "<input size=\"32\" name=\"wuid2\"");
row2(
    "Cancel only jobs with no instance in progress
        <br><p class=\"text-muted\">
        You can cancel jobs with instances that are in progress,
        but if you do so, users will not get credit for these instances.
        </p>
    ",
    "<input type=checkbox name=unsent_only>"
);
row2("", "<input class=\"btn btn-default\" type=\"submit\" value=\"Cancel jobs\">");
end_table();
echo "
    </form>
";
admin_page_tail();
?>
