<? // -*-html -*-
require_once("docutil.php");
page_head("Building the BOINC Core Client");
?>

<h1>Build clients for all platforms</h1>
For each platform your project supports (e.g.: i686-pc-linux,
sun-sparc-solaris2.7, sun-sparc-solaris2.8, intelx86_windows), you must
compile the core client.

<ul>
  <li><a href=build_client_unix.php>Building the core client for Unix</a>
  <li><a href=build_client_win.php>Building the core client GUI for Windows</a>
  <li><a href=build_client_mac.php>Building the core client GUI for Macintosh</a>
</ul>

<h1>Update project database</h1>

The BOINC server keeps track of core client platforms and versions so that
users may download the latest version. For each client compiled, put the
package in the master server's <code>boinc/apps/</code> directory and
run <code>boinc/tools/update_versions</code>.  For Windows and Macintosh, the
files are self-extracting executables; for Unix they are .gz files which can
be run directly after ungzipping.

<?
   page_tail();
?>
