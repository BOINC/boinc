<? // -*-html -*-
require_once("docutil.php");
page_head("Building the BOINC Core Client GUI for Macintosh");
?>

See the <a href=software.php>Software Prerequisites</a>.

<p>
The build is same as for Unix.

<h3>Notes</h3>

CAUTION: Stuffit Expander has a default option to convert the format of text
files to use Macintosh line endings ('\r' only).  Many of the source files
involved here are cross-platform files, and have Unix line endings.  If you
use Stuffit Expander to expand compressed source files, be sure to turn off
this feature as follows: Double click on Stuffit Expander, and select File /
Preferences....  Under cross-platform, select NEVER Convert text files to
Macintosh format.

<?
   page_tail();
?>
