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
This is a special module that <code>configure</code> places in relevant
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
  <tr><td><code>boinc/sched/</code></td><td> <a href=tool_start.php><code>start</code></a> </td> <td>
      BOINC start / Super Cron program
  </td></tr>
  <tr><td><code>boinc/tools/</code></td><td> <a href=tool_add.php><code>add</code></a> </td> <td>
      Adds objects to the database.
      TODO: document
  </td></tr>
  <tr><td><code>boinc/tools/</code></td><td> <a href=single_host_server.php><code>make_project</code></a> </td> <td>
      Creates a project
  </td></tr>
  <tr><td><code>boinc/tools/</code></td><td> <a href=tool_update_versions.php><code>update_versions</code></a> </td> <td>
      Adds all relevant core client and application executables to download
      directory and database
  </td></tr>
  <tr><td><code>boinc/test/</code></td><td> <code>test*.py<br>cgiserver.py</code> </td> <td>
      Test scripts: see the <a href=test.php>testing framework</a>.
  </td></tr>
</table>

<h1>Python modules in <code>boinc/py/Boinc/</code></h1>
<table border=1>
  <tr><td><code>boinc_path_config.py.in</code></td><td>
      <code>Configure</code> puts <code>boinc_path_config.py</code> in all
      directories that need it; see above
  </td></tr>
  <tr><td><code>boinc_project_path.py</code></td><td>
      sets where <code>config.xml</code> et al can be found; see above.
  </td></tr>

  <tr><td><code>configxml.py</code></td><td>
      reads and writes <code>config.xml</code> and <code>run_state.xml</code>
      - see its pydoc for more information
  </td></tr>

  <tr><td><code>boinc_db.py</code></td><td>
      auto-generated file that contains database constant definitions,
      e.g. <code>RESULT_OUTCOME_SUCCESS = 1</code>
  </td></tr>

  <tr><td><code>setup_project.py</code></td><td>
      internal module for creating a project.  See <a href=
      tool_make_project.php><code>make_project</code></a> and test scripts.
  </td></tr>

  <tr><td><code>database.py</code></td><td>
      defines database backend functions and database operations; see below.
  </td></tr>
  <tr><td><code>db_mid.py</code></td><td>
      "middle-end": optional mix-in to ease debugging by allowing printing of
      database objects directly
  </td></tr>
  <tr><td><code>util.py</code></td><td>
      miscellaneous functions
  </td></tr>
  <tr><td><code>version.py.in</code></td><td>
      version and platform-specific definitions snarfed by <code>configure</code>
  </td></tr>

</table>

<h1>Python database access</h1>
<code>Database.py</code> defines database backend library and database table
and object relationships to allow easy data manipulation.
<p>

All <a href=database.php>database tables</a> have a corresponding class and
its rows have classes, where each column is a member of that class.  Ids are
automatically translated to and from objects.  To begin, import
the <code>database</code> module:
<pre>
  from Boinc import database
</pre>

Connect to the database:
<pre>
  database.connect_default_config()
</pre>

Table classes can be indexed using the [ ] operator to retrieve an object by
id; e.g.
<pre>
  # executes 'select * from project where id=1'.
  # exception is raised if project is not found
  project_with_id_1 = database.Projects[1]
</pre>

Table classes have a <code>find</code> function that builds and executes a
MySQL query based on its arguments:
<pre>
  # this could return any number (0, 1, 2, ...) of platforms
  # executes 'select * from platform where user_friendly_name="commodore 64"'
  list_of_platforms_called_c64 = database.Platforms.find(
      user_friendly_name = 'Commodore 64')
</pre>

Find can take any number of arguments; they are ANDed. For more advanced
usage such as custom SQL queries (anything is possible :) see the pydoc.
<pre>
  all_apps = database.Apps.find()
  finished_yeti_wus = database.Workunits.find(
      app = database.Apps.find(name='YETI@home')[0],
      assimilate_state = ASSIMILATE_DONE)
</pre>

Objects (table rows) have their column data as members so you can access and
modify them directly.

<pre>
  user_quarl = database.users.find(email_addr='quarl@quarl.org')[0]
  print "name =", user_quarl.name
  user_quarl.postal_code = 97404
</pre>

To create a new database object, create a Python object and give all values
as parameters to the initializer:
<pre>
  new_app = database.App(name='SPAGHETTI@home',
                         min_version=1,
                         create_time=time.time())
</pre>

To commit any changes (including a new object), call <code>commit()</code>
(the tool <code>boinc/tools/add.py</code> is a command-line interface to
this):
<pre>
  user_quarl.commit()  # executes an UPDATE
  new_app.commit()     # executes an INSERT
</pre>

To remove an object, call <code>remove()</code>:
<pre>
  team_eric_test = database.Teams(name="Eric's Test Team")[0]
  team_eric_test.remove()
  #                        OR
  for team in database.Teams(name="Eric's Test Team"):
      team.remove()
  #                        OR
  map(database.Team.remove,database.Teams(name="Eric's Test Team"))
</pre>

To access objects related by id, access the field name without "id" suffix:
(the <code>result</code> table has a column '<code>workunitid</code>')
<pre>
  wu_1234 = database.Workunits.find(name='1234.wu')[0]
  results_of_wu_1234 = database.Results.find(workunit=wu_1234)
  for result in results_of_wu_1234:
      os.system("echo 'you are crunching %s' | mail '%s'" %(
                 result.name, result.host.user.email_addr))
</pre>

<table border=1 width=100%>
  <tr><th>Table</th><th>Python table object</th><th>Python row object
      class</th></tr>
  <tr><td>project</td><td>Projects</td><td>Project</td></tr>
  <tr><td>platform</td><td>Platforms</td><td>Platform</td></tr>
  <tr><td>core_version</td><td>CoreVersions</td><td>CoreVersion</td></tr>
  <tr><td>app</td><td>Apps</td><td>App</td></tr>
  <tr><td>app_version</td><td>AppVersions</td><td>AppVersion</td></tr>
  <tr><td>user</td><td>Users</td><td>User</td></tr>
  <tr><td>team</td><td>Teams</td><td>Team</td></tr>
  <tr><td>host</td><td>Hosts</td><td>Host</td></tr>
  <tr><td>workunit</td><td>Workunits</td><td>Workunit</td></tr>
  <tr><td>result</td><td>Results</td><td>Result</td></tr>
  <tr><td>workseq</td><td>Workseqs</td><td>Workseq</td></tr>
</table>

<?
  page_tail();
?>

