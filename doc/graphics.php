<?
require_once("docutil.php");
page_head("The BOINC graphics API");
echo"
<p>
BOINC applications can optionally generate graphics.
Graphics are displayed either in an application window
or in a full-screen window (when acting as a screensaver).
Applications that do graphics must call
<pre>
    void boinc_init_graphics();
</pre>
at the start and
<pre>
    void boinc_finish_opengl();
</pre>
prior to exiting.
<p>
The main application thread is called the <b>worker thread</b>.
<code>boinc_init_graphics()</code> creates a second thread,
called the <b>graphics thread</b>.
The two threads communicate through application-defined
shared memory structures.
Typically these structures contain information about the computation,
which is used to generate graphics.
<img vspace=10 src=graphics.png>
<br>
The worker must initialize the shared data structure
before calling <code>boinc_init_graphics()</code>.
<p>
Graphical applications must supply the following functions:
<pre>
    bool app_graphics_render(int xs, ys, double time_of_day);
</pre>
This will be called periodically in the graphics thread.
It should generate the current graphic.
<code>xs</code> and <code>ys</code> are the X and Y sizes of the window,
and <tt>time_of_day</tt> is the relative time in seconds.
The function should return true if it actually drew anything.
It can refer to the user name, CPU time etc. obtained from
<code>boinc_get_init_data()</code>.
Applications that don't do graphics must also supply a
dummy <code>app_graphics_render()</code> to link with the API.
<pre>
    void app_graphics_init();
</pre>
This is called in the graphics thread when a window is created.
It must make any calls needed to initialize graphics in the window.
<pre>
    void app_graphics_resize(int x, int y);
</pre>
Called when the window size changes.

<pre>
    void app_graphics_reread_prefs();
</pre>
This is called, in the graphics thread, whenever
the user's project preferences change.
It can call
<pre>
    boinc_parse_init_data_file();
    boinc_get_init_data(APP_INIT_DATA&);
</pre>
to get the new preferences.

<p>

<h3>Limiting frame rate</h3>
<p>
The following global variables control frame rate:
<p>
<b>boinc_max_fps</b> is an upper bound on the number of frames per second
(default 30).
<p>
<b>boinc_max_gfx_cpu_frac</b> is an upper bound on the fraction
of CPU time used for graphics (default 0.5).

";
page_tail();
?>
