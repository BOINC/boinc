<?php

require_once("../html_user/project_specific/project.inc");
require_once("../html_user/profile.inc");
require_once("../html_user/util.inc");
require_once("../html_user/sanitize_html.inc");

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

  //echo "Result has $numIds rows.<br>";

  // Build an array of IDs of all users with pictures in their profiles.
  while ($row = mysql_fetch_array($result, MYSQL_NUM)) {  // NUM is supposedly faster than ASSOC.
    $userIds[] = $row[0];
  }

  mysql_free_result($result);

  if (count($userIds) > 0) {
    // Randomize the ordering of users.
    shuffle($userIds);
  }

  $numPages = ceil(count($userIds)/($width * $height));

  // Make sure that a page is generated even when no profiles with pictures
  // exist in order to avoid 404 errors from the profile_menu page.

  if ($numPages == 0) {
    $numPages = 1;
  }

  //echo "Generating $numPages pages.<br>";

  $count = 0;

  for ($page = 1; $page <= $numPages; $page++) {
    $file = "../html_user/" . PROFILE_PATH . "user_gallery_" . $page . ".html";
    $descriptor = fopen($file, "w");

    page_head("User Picture Gallery: Page $page of $numPages", null, $descriptor);

    fwrite($descriptor, "<h2>User Profile Pictures</h2>Last updated " . gmdate("r") . " UTC\n<p>Browse the user profiles by picture. Only user profiles with pictures are listed here.");

    fwrite($descriptor, "<table border=1 cellpadding=5>\n");

    for ($row = 0; $row < $height; $row++) {
      fwrite($descriptor, "<tr>");
      for ($col = 0; $col < $width; $col++) {
	if ($count < $numIds) {

	  fwrite($descriptor, "<td><a href=\"" . URL_BASE . "view_profile.php?userid=" . $userIds[$count] . "\"><img src=\"" . '../' . IMAGE_PATH . $userIds[$count] . '_sm.jpg' . "\"></a></td>");
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
  //echo "<br><br><a href=\"" . "../html_user/" .PROFILE_PATH . "user_gallery_1.html\">Go to the first generated page.</a>";
}

// Creates pages grouping user profiles by country.  Filenames are of the
// format "profile_country_<country name>_<page number>.html"
// Also creates a summary page listing all countries which have profiled
// members, the number of such members, and links to the created pages for
// each country.

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

  //echo "$numIds users have profiles AND non-null country entries.<br>";

  $countries = array_keys($countryMembers);

  // Build the pages.
  // TODO: Define a constant for the desired number of rows per page.

  foreach ($countries as $country) {
      $baseFileName =  "profile_country_" . get_legal_filename($country);
      $filePath = "../html_user/" . PROFILE_PATH;
      build_profile_pages($countryMembers[$country], "User Profiles from $country", $country, 5, 2, $filePath, $baseFileName, "../html_user/");

  }

  // Build the summary page linking to the individual country pages.
 
  build_country_summary_page($countryMembers);
  
  //echo "<br><a href=\"" . "../html_user/" . PROFILE_PATH . "profile_country.html\">View Summary Page</a>";
  //echo "<br><br>Done";

}

// Creates pages grouping users by the first letter of their names.

function build_alpha_pages() {
    
    $query = "SELECT * FROM profile";
    $result = mysql_query($query);
    $numIds = 0;
    
    while ($row = mysql_fetch_assoc($result)) {
        $query2 = "SELECT * FROM user WHERE id = " . $row['userid'];
        $result2 = mysql_query($query2);
        $row2 = mysql_fetch_assoc($result2);
        
        if ($row2['name']) {
            $members[$row2['name'][0]][] = $row2['id'];
            $numIds++;
        }
    }
    
    mysql_free_result($result);
    mysql_free_result($result2);
    
    //echo "$numIds users have profiles AND non-null country entries.<br>";
    
    $letters = array_keys($members);
    
    foreach ($letters as $letter) {
        // TODO: Make sure array indexing is not case sensitive.
        $letter = strtoupper($letter);
        $filePath = "../html_user/" . PROFILE_PATH;
        build_profile_pages($members[$letter], "User Profiles - Names beginning with $letter", "Names beginning with $letter", 5, 2, $filePath, "profile_$letter", "../html_user/");
    }
    
    build_alpha_summary_page($letters);
}

// A generalized function to produce some number of pages summarizing a
// set of user profiles.

function build_profile_pages($members, $pageHead, $pageTitle, $rowsPerPage, $colsPerPage, $filePath, $baseFileName, $pathMod=NULL) {
    $numMembers = count($members);
    $numPerPage = $rowsPerPage * $colsPerPage;
    $numPages = ceil(count($members) / $numPerPage);
    
    $count = 0;

    for ($page = 1; $page <= $numPages; $page++) {

        $filename = $filePath . $baseFileName . "_" . $page . ".html";
        $descriptor = fopen($filename, "w");
        
        $head = $pageHead . ": Page $page of $numPages";
        page_head($pageHead, null, $descriptor);
        
        fwrite($descriptor, "<h2>$pageTitle</h2>\n");
        fwrite($descriptor, "Last updated " . gmdate("r") . " UTC<p>\n");
        
        $offset = (($page-1) * $rowsPerPage * $colsPerPage);
        
        show_user_table($members, $offset, $numPerPage, $colsPerPage, $descriptor, $pathMod);
 
        write_page_links($baseFileName, $page, $numPages, $descriptor);
        
        page_tail($descriptor);
        fclose($descriptor);
    }
    
}

function build_country_summary_page($countryMembers) {
  $countries = array_keys($countryMembers);

  $filename = "../html_user/" . PROFILE_PATH . "profile_country.html";
  $descriptor = fopen($filename, "w");

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

function build_alpha_summary_page($characters) {
    // OK, so it's not quite the alphabet- alphanumerabet is a bad variable name.
    $alphabet = array('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9');
    
    $filename = "../html_user/profile_alpha.html";
    $descriptor = fopen($filename, "w");
    
    // TODO: Add "Other" to handle non-alphanumeric first characters.  Will involve removing elements from $characters as we finish them.
    
    foreach ($alphabet as $character) {
        if (in_array($character, $characters)) {
            fwrite($descriptor, "<a href=" . PROFILE_PATH . "profile_" . $character . "_1.html>$character</a>&nbsp;");
            
        } else {
            fwrite($descriptor, "$character ");
        }
    }
    fclose($descriptor);
}


?>
