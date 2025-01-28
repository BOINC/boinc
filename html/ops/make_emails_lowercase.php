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

// (0) This script corrects email addresses in the user database that are
//     not completely lowercase.  It also fixes cross_project_id values that
//     are zero.
// (1) these database error were probably introduced because of an omission
//     in create_account_action.php, that has been corrected in cvs.
// (2) accounts created in this way have cross_project_id set to zero.
//     This error is also fixed by the html/ops/make_emails_lowercase.php
//     script
// (3) script is safe to run multiple times and on databases with no errors
//     As supplied in cvs it is 'read only' and will only report problems
//     the user database.  It won't correct them, until it is edited by hand
//     to enable it to write changes to the database.
// (4) Just to be safe, back up your user database before running this script.


$cli_only = true;
require_once("../inc/util_ops.inc");
require_once("../inc/db_ops.inc");

// activate/deactivate script
if (1) {
  echo "
This script needs to be activated before it can be run.
Once you understand what the script does you can change the
if (1) to if (0) at the top of the file to activate it.
Be sure to deactivate the script after using it to make sure
it is not accidentally run.
";
  exit;
}

db_init();
$confirm = $_GET['confirm'];
$update_needed = false;

admin_page_head("Repair emails and CPID=0");

echo "<br>
    Script for repairing user database if some email addresses in lower case and/or some CPID=0<br>
    <b>Attention this runs an expensive query on the database</b><br><br>\n";

$query = "select id, email_addr,cross_project_id  from user";
$result = _mysql_query($query);

// loop over all users
while ($user = _mysql_fetch_object($result)) {

  $id=$user->id;
  $email_addr=$user->email_addr;
  $cpid=$user->cross_project_id;

  $new_email=strtolower(trim($email_addr));

  if (strcmp($email_addr, $new_email))
    echo "Problematic email address [$id] $email_addr becomes $new_email<br/>";

  if (!(strcmp($cpid,"0"))) {
    $newcpid=random_string();
    echo "Problematic CPID=0 for [$id] $email_addr gets CPID=$newcpid<br/>";
  }
  else
    $newcpid=$cpid;

  if (strcmp($email_addr, $new_email) || strcmp($newcpid,$cpid)) {
    $update="update user set email_addr='$new_email', cross_project_id='$newcpid' where id='$id'";
    if ($confirm != "yes") {
      echo "QUERY WOULD BE [$id] $query <br/>[click the link at the bottom to enable]<br/>";
      $update_needed = TRUE;
    }
    else {
      _mysql_query($update);
      echo "Doing $update<br/>\n";
    }
  }
}
if ($confirm != "yes" && $update_needed) {
    echo "You can enable the changes by <a href=\"make_emails_lowercase.php?confirm=yes\">click</a>";
} else {
    echo "No updates needed.";
}
_mysql_free_result($result);

admin_page_tail();
?>
