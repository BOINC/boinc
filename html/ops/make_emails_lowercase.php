<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

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


require_once("../inc/util_ops.inc");
require_once("../inc/db_ops.inc");

db_init();
$confirm = $_GET['confirm'];
$update_needed = false;

admin_page_head("Repair emails and CPID=0");

echo "<br>
    Script for repairing user database if some email addresses in lower case and/or some CPID=0<br>
    <b>Attention this runs an expensive query on the database</b><br><br>\n";
$query = "select count(*) from user";
if (!($result = mysql_query($query))) {
  echo "No rows found in USER database table";
  exit();
}

$users_array = mysql_fetch_array($result);
mysql_free_result($result);
$number_of_users=$users_array[0];
echo "Found $number_of_users users<br/>";

$query = "select id, email_addr,cross_project_id  from user";
$result = mysql_query($query);

// loop over all users
while ($user = mysql_fetch_object($result)) {

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
      mysql_query($update);
      echo "Doing $update<br/>\n";
    }
  }
}
if ($confirm != "yes" && $update_needed) {
    echo "You can enable the changes by <a href=\"make_emails_lowercase.php?confirm=yes\">click</a>";
} else {
    echo "No updates needed.";
}
mysql_free_result($result);

admin_page_tail();
?>
