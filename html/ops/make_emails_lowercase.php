<?php

echo "
       <!-- Script for repairing user database if some email addresses in lower case and/or some CPID=0 -->\n
       <!-- \$Id$ -->\n
";


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

for ($id=1; $id<=$number_of_users; $id++) {
  $query = "select email_addr,cross_project_id  from user where id=$id";
  if (!($result = mysql_query($query))) {
    echo "Entry with id=$id was not found in USER database table";
    exit(); // change to continue; if missing rows!
  }
  
  $user = mysql_fetch_object($result);
  mysql_free_result($result);
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
    $query="update user set email_addr='$new_email', cross_project_id='$newcpid' where id=$id";
    // modify line that follows to enable changes to user database: change (1) to (0)
    if (1)
      echo "QUERY WOULD BE [$id] $query <br/>[Modify html/ops/make_emails_lowercase.php to enable changes]<br/>";
    else {
      $result = mysql_query($query);
      echo "Doing $query<br/>";
    }
  }
}

echo "
      </BODY></HTML>\n 
";
