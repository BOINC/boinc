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

Easy :) The final target
is <code>boinc/client/boinc_VERSION_PLATFORM.gz</code>.

<?
   page_tail();
?>
