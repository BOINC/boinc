<?php

require_once("project.inc");
require_once("profile.inc");

// Generates the html files which comprise the photo gallery.
//   $room: which gallery to generate (user, computer).
//   $width: the width of the table of images.
//   $height: the height of the table of images. 
function build_picture_pages($width, $height) {

  // TODO: Add support for a computer image gallery.

  // TODO: Should we eliminate the has_picture flag?  Doesn't really
  // seem necessary when we're building static pages- could just use
  // file_exists on the username...

  // TODO: Standardize "Last modified" string to a function call (util.inc).

  $query = "SELECT userid FROM profile WHERE has_picture = 1";
  $result = mysql_query($query);
  $numIds = mysql_num_rows($result);

  echo "Result has $numIds rows.<br>";

  // Build an array of IDs of all users with pictures in their profiles.
  while ($row = mysql_fetch_array($result, MYSQL_NUM)) {  // NUM is supposedly faster than ASSOC.
    $userIds[] = $row[0];
  }

  mysql_free_result($result);

  // Randomize the ordering of users.
  shuffle($userIds);

  $numPages = ceil(count($userIds)/($width * $height));
  
  echo "Generating $numPages pages.<br>";

  $count = 0;

  for ($page = 1; $page <= $numPages; $page++) {
    $file = PROFILE_PATH . "user_gallery_" . $page . ".html";
    $descriptor = fopen($file, "w");
    
    page_head("User Picture Gallery: Page $page of $numPages", null, $descriptor); 

    fwrite($descriptor, "<h2>User Profile Pictures</h2>Last updated " . gmdate("r") . " UTC\n<p>Browse the user profiles by picture. Only user profiles with pictures are listed here.");

    fwrite($descriptor, "<table border=1 cellpadding=5>\n");

    for ($row = 0; $row < $height; $row++) {
      fwrite($descriptor, "<tr>");
      for ($col = 0; $col < $width; $col++) {
	if ($count < $numIds) {

	  fwrite($descriptor, "<td><a href=\"" . MASTER_URL . "view_profile.php?userid=" . $userIds[$count] . "\"><img src=\"" . '../' . IMAGE_PATH . $userIds[$count] . '_sm.jpg' . "\"></a></td>");
	  $count++;
	}
      }
      fwrite($descriptor, "</tr>\n");
      if ($count == $numIds) {
	break;
      }
    }
    
    fwrite($descriptor, "</table>\n");

    // Previous and Next links
    
    write_page_links("user_gallery", $page, $numPages, $descriptor);

    page_tail($descriptor);

    //fwrite($descriptor, "</body></html>");
    fclose($descriptor);

  }
  echo "<br><br><a href=\"" . PROFILE_PATH . "user_gallery_1.html\">Go to the first generated page.</a>";
}

function build_country_pages() {

  $query = "SELECT * FROM profile";
  $result = mysql_query($query);
  $numIds = 0;

  // Build a multi-dimensional array of countries, each element of which contains an array
  // of the userids who belong to those countries.  Format: array[country][index] = userid.

  while ($row = mysql_fetch_assoc($result)) {
    $query2 = "SELECT * FROM user WHERE id = " . $row['userid'];
    $result2 = mysql_query($query2);
    $row2 = mysql_fetch_assoc($result2);

    if ($row2['country']) {
      $countryMembers[$row2['country']][] = $row2['id'];
      $numIds++;
    }
  }

  mysql_free_result($result);
  mysql_free_result($result2);

  echo "$numIds users have profiles AND non-null country entries.<br>";

  $countries = array_keys($countryMembers);

  // Build the pages.
  foreach ($countries as $country) {
    build_country_page($country, &$countryMembers[$country], 5); 
  }

  // Build the summary page linking to the individual country pages.
  // TODO: Pass $countryMembers by reference.
  build_country_summary_page($countryMembers);
  echo "<br><a href=\"" . PROFILE_PATH . "profile_country.html\">View Summary Page</a>";

  echo "<br><br>Done";

}

function build_country_page($name, $members, $rows) {
  
  $countryName = $name;

  // Make the country name a legal format for a filename.
  $name = get_legal_filename($countryName);

  $name = ereg_replace(',', '', $name);
  $name = ereg_replace(' ', '_', $name);

  $numMembers = count($members);
  $numPages = ceil(count($members) / (2 * $rows));
  echo "<br>$countryName has $numMembers members. - $numPages pages will be generated. <a href=", PROFILE_PATH , "profile_country_" , $name , "_1.html> VIEW</a>";
  
  $count = 0;

  for ($page = 1; $page <= $numPages; $page++) {

    $filename = PROFILE_PATH . "profile_country_" . $name . "_" . $page . ".html";
    $descriptor = fopen($filename, "w");

    // TODO: Page header.
    //fwrite($descriptor, "<html><body>\n");
    page_head("User Profiles from $countryName: Page $page of $numPages", null, $descriptor);

    fwrite($descriptor, "<h2>$countryName</h2>\n");
    fwrite($descriptor, "Last updated " . gmdate("r") . " UTC<p>\n");
    fwrite($descriptor, "<table border=1 cellpadding=5>\n");

    for ($row = 0; $row < $rows; $row++) {
      if ($count >= $numMembers) {
	break;
      }

      fwrite($descriptor, "<tr>\n"); 

      // Formatting is a table with two columns of user summaries.
      for ($col = 0; $col < 2; $col++) {
	if ($count < $numMembers) {
	  fwrite($descriptor, "<td width=7% height=64><center>");
	  
	  // Only link an image if one exists.
	  if (file_exists(IMAGE_PATH . $members[$count] . '_sm.jpg')) {
	    fwrite($descriptor, "<a href=\"" . MASTER_URL . "view_profile.php?userid=" . $members[$count] . "\"><img src=\"" . '../' . IMAGE_PATH . $members[$count] . '_sm.jpg' . "\"></a>");
	  } else {
	    fwrite($descriptor, "&nbsp;");
	  }
	  
	  fwrite($descriptor, "</center></td><td width=33% height=64>");
	  fwrite($descriptor, get_profile_summary($members[$count]));
	  fwrite($descriptor, "</td>");
	  $count++;
	}
	else {
	  // Empty entry.
	  fwrite($descriptor, "<td width=7% height=64></td><td width=33% height=64></td>");
	}
      }
      fwrite($descriptor, "</tr>\n");
    }
    fwrite($descriptor, "</table>\n");

    $tempFileName = "profile_country_" . $name;
    write_page_links($tempFileName, $page, $numPages, $descriptor);

    page_tail($descriptor);
    fclose($descriptor);
  }
  
}

function get_legal_filename($name) {
  $name = ereg_replace(',', '', $name);
  return ereg_replace(' ', '_', $name);
}

function get_profile_summary($userid) {
  $result = mysql_query("SELECT * FROM profile WHERE userid = $userid");
  $result2 = mysql_query("SELECT name FROM user WHERE id = $userid");

  if (!$result || !$result2) {
    echo "Database error!";  // Change this to a standard error page.
    exit();
  }

  $row = mysql_fetch_assoc($result);  
  $row2 = mysql_fetch_assoc($result2);

  mysql_free_result($result);
  mysql_free_result($result2);

  if (strlen($row['response1']) != 0) {
  
    $description = "(\"" . substr($row['response1'], 0, MAX_DESC_LENGTH);
    if (strlen($row['response1']) >= MAX_DESC_LENGTH) {
      $description = $description . "...";
    }
    $description = $description . "\")";
  }

  $summary = "<a href=\"" . MASTER_URL . "view_profile.php?userid=" . $userid . "\">" . $row2['name'] . "</a> " . $description;
  return $summary;
}

function write_page_links($filename, $currPageNum, $numPages, $descriptor) {
  fwrite($descriptor, "<p>Page $currPageNum of $numPages</p>");

  $nextPageNum = $currPageNum + 1;
  $prevPageNum = $currPageNum - 1;

  // Make the 'previous' and 'next' page links as appropriate.
  if ($currPageNum > 1) {
    fwrite($descriptor, "<a href=$filename" . "_" . $prevPageNum . ".html>Previous Page</a>");
    
    if ($currPageNum != $numPages) {
      fwrite($descriptor, " | ");
    }
  }
  if ($currPageNum != $numPages) {
    fwrite($descriptor, "<a href=$filename" . "_" . $nextPageNum . ".html>Next Page</a>");
  }

  fwrite($descriptor, "<p><table border=0>\n<tr>\n<td>Jump to Page:</td>\n");

  // Make the individual page links (or a bold non-link for the current page).
  for ($i = 1; $i <= $numPages; $i++) {
    if ($i != $currPageNum) {
      fwrite($descriptor, "<td><a href=$filename" . "_" . $i . ".html>$i</a></td>\n");
    } else {
      fwrite($descriptor, "<td><b>$i</b></td>\n");
    }
  }

  fwrite($descriptor, "</table>\n");
}

function build_country_summary_page($countryMembers) {
  $countries = array_keys($countryMembers);
  
  $filename = PROFILE_PATH . "profile_country.html";
  $descriptor = fopen($filename, "w");
  
  echo "<p>Building country summary page...<br>";

  page_head("User Profiles by Country", null, $descriptor);
  fwrite($descriptor, "<h2>User Profiles by Country</h2>Last updated " . gmdate("r") . " UTC<p>");
  
  fwrite($descriptor, "<table border=0>\n");
  fwrite($descriptor, "<tr><td><b>Country</b></td><td align=\"center\"><b>Profiles</b></td></tr>\n");

  foreach ($countries as $country) {
    $numMembers = count($countryMembers[$country]);
    $name = get_legal_filename($country);

    fwrite($descriptor, "<tr>\n<td><a href=\"profile_country_" . $name . "_1.html\">$country</a></td><td align=\"center\">$numMembers</td></td>\n");

  }

  fwrite($descriptor, "</table>");
  page_tail($descriptor);

  fclose($descriptor);
}
?>