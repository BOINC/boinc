<? // -*-html -*-
require_once("docutil.php");
page_head("Building the BOINC Core Client GUI for Windows");
?>

See the <a href=software.php>Software Prerequisites</a>.

<h1>Build executables</h1>

Using MSVC6, build "boinc_GUI - Win32 Release" or "boinc_GUI - Win32 Debug"
versions as appropriate.  This should also build dependent libraries and
screen saver.

<h1>Build installation package</h1>

Open BOINC.ipr.  Update the version number:
<ul>
  <li>Readme.txt, license.txt
  <li>Resources
</ul>

Execute "Build Default Media".

<h2>Create self-extracting executable</h2>

Open BOINC.pfw.  Accept all defaults, updating version number.

<h1>Anti-virus</h1>

Run a virus checker over all the individual (uncompressed)
files as well as the final build


<?
   page_tail();
?>
