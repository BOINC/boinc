<? // -*- html -*-
   // $Id$
   require_once("docutil.php");
   page_head("Python Framework");
?>

See the section on Python in the <a href=software.php>Software
Prerequisites</a>.

<h1>Structure</h1>

The directory <code>boinc/py/Boinc</code> contains the <code>Boinc</code>
module.  This means if you have <code>boinc/py/</code> in your python path you can
write for example:
<blockquote>
<code>from Boinc.setup_project import *</code>
</blockquote>

To ensure <code>boinc/py/</code> is in your python path:
<blockquote>
<code>import boinc_path_config</code>
</blockquote>
This is a special kludge module that <code>configure</code> places in relevant
directories which then modifies <code>sys.path</code> appropriately.

<h2>Project-specific settings</h2>
The module <code>boinc_project_path</code> is imported to get the paths
for <code>config.xml</code> and <code>run_state.xml</code>.  The default
paths for these are the parent directory of the invocation script.  You can
override these defaults
<ol>
  <li> modify this file directly (if you have only one project on your server
    or have separate copies for each)
  <li> create a new boinc_project_path.py and place it earlier in PYTHONPATH
       than the default one
  <li> define environment variables
</ol>

Example <code>boinc_project_path.py</code>
<pre>
  config_xml_filename = '/etc/boinc/yetiathome/config.xml'
  run_state_xml_filename = '/var/lib/boinc/yetiathome/run_state.xml'
</pre>

See the source of file <code>boinc/py/Boinc/boinc_project_path.py</code> for
details.

<h2>Directories containing python scripts</h2>
<table border=1 width=100%>
  <tr><td><code>boinc/py/</code></td><td> <code>Boinc/*.py</code> </td> <td>
      Main BOINC python modules
  </td></tr>
  <tr><td><code>boinc/sched/</code></td><td> <code>start</code> </td> <td>
      BOINC start / Super Cron program
  </td></tr>
  <tr><td><code>boinc/tools/</code></td><td> <code>add</code> </td> <td>
      Adds objects to the database
  </td></tr>
  <tr><td><code>boinc/test/</code></td><td> <code>test*.py<br>cgiserver.py</code> </td> <td>
      Test scripts: see the <a href=test.php>testing framework</a>.
  </td></tr>
</table>



<?
   page_tail();
?>

