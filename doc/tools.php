<?
require_once("docutil.php");
page_head("Operational Tools");
echo "
BOINC provides tools for creating and operating projects:
<p>
<table border=1 cellpadding=8 width=100%>
  <tr><td><code>boinc/tools/</code></td>
      <td><a href=make_project.php><code>make_project</code></a> </td>
      <td>Creates the server complex for a new project.</td>
  </tr>
  <tr><td><code>boinc/sched/</code></td><td> <a
  href=tool_start.php><code>start, stop, status</code></a> </td> <td>
      BOINC start / Super Cron program
  </td></tr>
  <tr><td><code>boinc/tools/</code></td><td> <a href=tool_add.php><code>add</code></a> </td> <td>
      Adds objects to the database through command-line. 
  </td></tr>
  <tr><td><code>boinc/tools/</code></td><td> <a href=tool_xadd.php><code>xadd</code></a> </td> <td>
      Adds objects to the database through project.xml.
  </td></tr>
  <tr><td><code>boinc/tools/</code></td><td> <a href=tool_update_versions.php><code>update_versions</code></a> </td> <td>
      Adds core and application versions to the database.
  </td></tr>
  <tr><td><code>boinc/tools/</code></td><td> <a href=><code>upgrade</code></a> </td> <td>
      Upgrades an existing installation
  </td></tr>
</table>
<p>

Projects can create their own tools using the Python API (see <a
href=python.php>Python framework</a>) or the C++ API (<code>db/db.h</code>)
<p>
Instructions for compiling and releasing the core client are
<a href=ssl_client_release_instructions.txt>here</a>.

";
   page_tail();
?>

