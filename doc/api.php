<?
require_once("docutil.php");
page_head("The BOINC application programming interface (API)");
echo "
<p>
The BOINC API is a set of C++ functions.
Unless otherwise specified,
the functions return an integer error code; zero indicates success.
On an error return, the application should exit with that status.
The BOINC graphics API is described <a href=graphics.php>separately</a>.

<h3>Initialization and termination</h3>
The application must call
<pre>
    int boinc_init(bool standalone=false);
</pre>
before calling other BOINC functions or doing I/O.
If <code>standalone</code> is true,
the application will function independently of the BOINC core client
(this is useful for testing).
<p>
When the application has completed it must call
<pre>
    int boinc_finish(int status);
</pre>
<code>status</code> is nonzero if an error was encountered.
This call does not return.

<h3>Resolving file names</h3>
Applications that use named input or output files must call
<pre>
    int boinc_resolve_filename(char *logical_name, char *physical_name, int len);
</pre>
or
<pre>
    int boinc_resolve_filename(char *logical_name, string& physical_name);
</pre>
to convert logical file names to physical names.
For example, instead of
<pre>
    f = fopen(\"my_file\", \"r\");
</pre>
</p>
the application might use
<pre>
    string resolved_name;
    retval = boinc_resolve_filename(\"my_file\", resolved_name);
    if (retval) fail(\"can't resolve filename\");
    f = fopen(resolved_name.c_str(), \"r\");
</pre>
<code>boinc_resolve_filename()</code> doesn't need to be used for temporary files.

<h3>Checkpointing</h3>

Computations that use a significant amount of time
per work unit may want to periodically write the current
state of the computation to disk.
This is known as <b>checkpointing</b>.
The state file must include everything required
to start the computation again at the same place it was checkpointed.
On startup, the application
reads the state file to determine where to begin computation.
If the BOINC client quits or exits,
the computation can be restarted from the most recent checkpoint.
<p>
Frequency of checkpointing is a user preference
(e.g. laptop users might want to checkpoint infrequently).
An application must call
<pre>
    bool boinc_time_to_checkpoint();
</pre>
whenever it reaches a point where it is able to checkpoint.
If this returns true,
the application should write the state file and flush all output files,
then call
<pre>
    void boinc_checkpoint_completed();
</pre>
<code>boinc_time_to_checkpoint()</code> is fast,
so it can be called frequently (hundreds or thousands of times a second).

<h3>Atomic file update</h3>
<p>
To facilitate atomic checkpoint, an application can write to output and
state files using the <code>MFILE</code> class.
<pre>
class MFILE {
public:
    int open(char* path, char* mode);
    int _putchar(char);
    int puts(char*);
    int printf(char* format, ...);
    size_t write(const void* buf, size_t size, size_t nitems);
    int close();
    int flush();
};
</pre>
MFILE buffers data in memory
and writes to disk only on <code>flush()</code> or <code>close()</code>.
This lets you write output files and state files more or less atomically.

<h3>Communicating with the core client</h3>
<p>
The core client GUI displays the percent done of workunits in progress.
To keep this display current, an application should periodically call
<pre>
   boinc_fraction_done(double fraction_done);
</pre>
The <code>fraction_done</code> argument is a rough estimate of the
workunit fraction complete (0 to 1).
This function is fast and can be called frequently.

<p>
The following functions get information from the core client;
this information may be useful for graphics.
<pre>
    int boinc_get_init_data(APP_INIT_DATA&);

    struct APP_INIT_DATA {
        char project_preferences[4096];
        char user_name[256];
        char team_name[256];
        double user_total_credit;
        double user_expavg_credit;
        double team_total_credit;
        double team_expavg_credit;
    };
</pre>
to get the following information:
";
list_start();
list_item("project_preferences", "An XML string containing
the user's project-specific preferences.");
list_item("user_name", " the user's 'screen name' on this project.");
list_item("team_name", " the user's team name, if any.");
list_item("user_total_credit", " user's total work for this project.");
list_item("user_expavg_credit", " user's recent average work per day.");
list_item("team_total_credit", " team's total work for this project.");
list_item("team_expavg_credit", " team's recent average work per day.");
list_end();
echo "
<p>
An application may call
<pre>
    int boinc_wu_cpu_time(double &cpu_time);
</pre>
to get its total CPU time
(from the beginning of the work unit, not just since the last restart).
This excludes CPU time used to render graphics.

<h3>Multi-program applications</h3>
Some applications consist of multiple programs:
a <b>main program</b> that acts as coordinator,
and one or more subsidiary programs.
Each program should use the BOINC API as described above.
<p>
Each program should have its own state file;
the state file of the coordinator program records
which subsidiary program was last active.
<p>
To correctly implement fraction done,
the main program should pass information to subsidiary programs
(perhaps as command-line arguments) indicating the starting and ending
fractions for that program.
<p>
The coordinator must call
<pre>
    void boinc_child_start();
</pre>
prior to forking a child process.
When the child is done, the coordinator
must get the child's CPU time, then call
<pre>
    void boinc_child_done(double total_cpu);
</pre>
before forking the next child process.
";
page_tail();
?>
