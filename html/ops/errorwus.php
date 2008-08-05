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
$cache_sec = 300;
// Number that determines how many client errors are necessary for a WU to show up in this list.
// This number is added to min_quorum of the WU, so a value of 1 means that there must be more than
// (min_quorum + 1) errors for a WU to show up in this list.
$notification_level = 1;

start_cache($cache_sec);
admin_page_head("All-error Workunits");

db_init();

function print_wu($id,$name,$quorum,$errors) {
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
  echo "<a href=db_action.php?table=result&query=&outcome=3&sort_by=mod_time&detail=low&workunitid=";
  echo $id;
  echo ">";
  echo $errors;
  echo "</a></td>\n";

  echo "</tr>\n";
}

$dbresult = mysql_query("
  SELECT workunitid, outcome, workunit.name, min_quorum
  FROM result, workunit
  WHERE workunit.id = workunitid AND server_state = 5
  ORDER BY workunitid, outcome DESC
;");

echo "<br><table border=\"1\">\n";
echo "<tr><th>WU ID</th><th>WU name</th><th>Quorum</th><th>Errors</th></tr>\n";

$rescount = 0;
$previd = -1;
$prevname = "";
$prevquorum = 1;
$errors = 0;

// The current version scans for client errors only.
// In case you want to include validate errors, add "|| (outcome = 6)" to "(outcome = 3)"

while ($res = mysql_fetch_object($dbresult)) {
  $id = $res->workunitid;
  if ($id != $previd) {
    if ($errors > $prevquorum + $notification_level) {
      print_wu($previd,$prevname,$prevquorum,$errors);
      $rescount++;
    }
    $previd = $id;
    $prevname = $res->name;
    $prevquorum = $res->min_quorum;
    $errors = 0;
  }
  if ($res->outcome == 3) {
    $errors ++;
  }
  if ($res->outcome == 1) {
    $errors = 0;
  }
}
mysql_free_result($dbresult);
if ($errors > $prevquorum) {
  print_wu($id,$prevname,$prevquorum,$errors);
  $rescount++;
}

echo "</table>\n<br>";
echo $rescount;
echo " entries\n";

admin_page_tail();

end_cache($cache_sec);
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
