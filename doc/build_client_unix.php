<? // -*-html -*-
require_once("docutil.php");
page_head("Building the BOINC Core Client for Unix");
?>

See the <a href=software.php>Software Prerequisites</a>.

<h1>Build executable</h1>

<pre>
  cd boinc/client
  make
</pre>

<h1>Update project database</h1>

Copy boinc_x.yz_platform.gz to the boinc/apps directory and run
boinc/tools/update_versions.

<?
   page_tail();
?>
