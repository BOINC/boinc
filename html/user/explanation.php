<?php
require_once("project.inc");
require_once("util.inc");
 ?> 
<html>
<head>
<title>User Profile Voting Information</title>
</head>
<table border=0 cellpadding=0>
<?php

if ($_GET['val'] == "recommend") {

  row1("Recommending User Profiles");
  rowify("<br>");
  rowify("If you really like a profile, hit the \"recommend\" button. The " . PROJECT . " team reviews recommended profiles for various purposes.");
} else {

  row1("Voting to Reject a Profile");
  rowify("<br>");
  rowify("If you find a profile offensive, please click \"vote to reject\". This flags the profile for review by " . PROJECT . " staff.");
}

end_table();
?>

</body>
</html>
