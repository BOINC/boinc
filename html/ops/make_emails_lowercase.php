<?php

echo "
       <!-- Script for repairing user database if some email addresses in lower case and/or some CPID=0 -->\n
       <!-- \$Id$ -->\n
";

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
require_once("../inc/util.inc");

echo "
      <HTML><HEAD><TITLE>User database repair script. Lowercae(email_addr) and set CPID!=0</TITLE></HEAD><BODY>\n
";

db_init();

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
    // modify line that follows to enable changes to user database: change (1) to (0)
    if (1)
      echo "QUERY WOULD BE [$id] $query <br/>[Modify html/ops/make_emails_lowercase.php to enable changes]<br/>";
    else {
      mysql_query($update);
      echo "Doing $update<br/>\n";
    }
  }
}
mysql_free_result($result);

echo "
      </BODY></HTML>\n 
";
