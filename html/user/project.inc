<?php

// This is a sample project file.  Replace it with your own.

require_once("util.inc");

define("PROJECT", "Test Project");
define("MASTER_URL", "http://boinc.berkeley.edu/");
define("URL_BASE", "");
define('IMAGE_PATH', 'images/user_profile/');
define('PROFILE_PATH', 'user_profile/');
define('LANGUAGE_FILE', 'languages.txt');

function project_intro() {
    echo"
This is a test project of
the Berkeley Open Infrastructure for Network Computing (BOINC)
";
}

function project_banner($user, $fileDescriptor) {
    // Put your project title and logo here

  if ($fileDescriptor) {
    fwrite($fileDescriptor, "<table width=100%><tr><td><h2>".PROJECT."</h2></td><td align=right>\n");
    if ($user) {
      fwrite($fileDescriptor, "Logged in as $user->name");
    } else {
      fwrite($fileDescriptor, "<br>");
    }
    fwrite($fileDescriptor, "</td></tr></table><hr>\n");

  } else {

    echo "<table width=100%><tr><td><h2>".PROJECT."</h2></td><td align=right>\n";
    if ($user) {
        echo "Logged in as $user->name";
    } else {
      echo "<br>";
    }
    echo "</td></tr></table><hr>\n";
  }
}

// NOTE: These functions appear here and not in profile.inc because they contain
// project-specific information that needs to be set by the project administrators.

function show_profile_heading1() {
  row1("Your personal background.");
}

function show_profile_question1() {
  rowify("
Tell us about yourself. You could tell us where you're from, your age, occupation, hobbies, or anything else about yourself. HTML formatting tags are allowed (other tags will be removed), and responses are limited to 2000 characters.
");
}

function show_profile_heading2() {
  row1("Your opinions about " . PROJECT);
}

function show_profile_question2() {
  rowify("
Tell us your opinions about SETI and SETI@home.  Possible topics:<br>
<ol>
<li> Do you think extraterrestrial life exists?  If so, when and how will humans discover it?  What are the possible benefits and dangers of such a discovery?
<li>Should humans transmit a beacon for others to find?  If so, what information should we send?
<li>Why do you run SETI@home?  What are your views about the project?  Any suggestions?
</ol>
");
}

?>
