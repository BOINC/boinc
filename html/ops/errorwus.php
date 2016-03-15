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

require_once("../inc/util_ops.inc");
require_once("../inc/cache.inc");

// User - configuarble variables
// seconds to cache this page
// this page runs a scan of the two largest tables, so this shouldn't be done more often than necessary
$cache_sec = 1800;
// Number that determines how many client errors are necessary for a WU to show up in this list.
// This number is added to min_quorum of the WU, so a value of 1 means that there must be more than
// (min_quorum + 1) errors for a WU to show up in this list.
$notification_level = 1;

start_cache($cache_sec);
admin_page_head("All-error Workunits");

function print_wu($id,$name,$quorum,$errors,$valerrors,$mask) {
  echo "<tr>\n";

  echo "<td align=\"left\" valign=\"top\">";
  echo "<a href=db_action.php?table=workunit&detail=high&id=";
  echo $id;
  echo ">";
  echo $id;
  echo "</a></td>\n";

  echo "<td align=\"left\" valign=\"top\">";
  echo $name;
  echo "</td>\n";

  echo "<td align=\"left\" valign=\"top\">";
  echo $quorum;
  echo "</td>\n";

  echo "<td align=\"left\" valign=\"top\">";
  if ($mask)
    echo wu_error_mask_str($mask);
  else
    echo $mask;
  echo "</td>\n";

  echo "<td align=\"left\" valign=\"top\">";
  echo "<a href=db_action.php?table=result&query=&outcome=3&sort_by=mod_time&detail=low&workunitid=";
  echo $id;
  echo ">";
  echo $errors;
  echo "</a></td>\n";

  echo "<td align=\"left\" valign=\"top\">";
  echo "<a href=db_action.php?table=result&query=&outcome=6&sort_by=mod_time&detail=low&workunitid=";
  echo $id;
  echo ">";
  echo $valerrors;
  echo "</a></td>\n";
  echo "</tr>\n";
}

$db = BoincDb::get();
$dbresult = $db->do_query("
  SELECT workunitid, outcome, workunit.name, min_quorum, error_mask
  FROM result INNER JOIN workunit ON workunit.id = workunitid
  WHERE server_state = 5
  ORDER BY workunitid, outcome DESC
;");

echo "<br><table border=\"1\">\n";
echo "<tr><th>WU ID</th><th>WU name</th><th>Quorum</th><th>Error mask</th><th>Client Errors</th><th>Validate Errors</th></tr>\n";

$rescount = 0;
$previd = -1;
$prevname = "";
$prevquorum = 1;
$prevmask = 0;
$valerrors = 0;
$clerrors = 0;

while ($res = $dbresult->fetch_object()) {
  $id = $res->workunitid;
  if ($id != $previd) {
    if (($clerrors  > $prevquorum + $notification_level) ||
	($valerrors > $prevquorum + $notification_level)) {
      print_wu($previd,$prevname,$prevquorum,$clerrors,$valerrors,$prevmask);
      $rescount++;
    }
    $prevmask = $res->error_mask;
    $previd = $id;
    $prevname = $res->name;
    $prevquorum = $res->min_quorum;
    $clerrors = 0;
    $valerrors = 0;
  }
  if ($res->outcome == 3) {
    $clerrors ++;
  }
  if ($res->outcome == 6) {
    $valerrors ++;
  }
  if ($res->outcome == 1) {
    $clerrors = 0;
    $valerrors = 0;
  }
}

$dbresult->free();

if (($clerrors  > $prevquorum + $notification_level) ||
    ($valerrors > $prevquorum + $notification_level)) {
  print_wu($id,$prevname,$prevquorum,$clerrors,$valerrors,$prevmask);
  $rescount++;
}

echo "</table>\n<br>";
echo $rescount;
echo " entries\n";

echo "<br><br>";

echo "Page last updated ";
echo time_str(time());

admin_page_tail();

end_cache($cache_sec);
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
