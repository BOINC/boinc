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
    void boinc_finish_graphics();
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
<br>
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

<h3>Handling input</h3>
<p>
The application must supply the following input-handling functions:
<pre>
void boinc_app_mouse_move(
    int x, int y,       // new coords of cursor
    bool left,          // whether left mouse button is down
    bool middle,
    bool right
);

void boinc_app_mouse_button(
    int x, int y,       // coords of cursor
    int which,          // which button (0/1/2)
    bool is_down        // true iff button is now down
);

void boinc_app_key_press(
    int, int            // system-specific key encodings
)

void boinc_app_key_release(
    int, int            // system-specific key encodings
)
</pre>
<h3>Limiting frame rate</h3>
<p>
The following global variables control frame rate:
<p>
<b>boinc_max_fps</b> is an upper bound on the number of frames per second
(default 30).
<p>
<b>boinc_max_gfx_cpu_frac</b> is an upper bound on the fraction
of CPU time used for graphics (default 0.5).

<h3>Support classes</h3>
<p>
Several graphics-related classes were developed for SETI@home/BOINC.
They may be of general utility.

<dl>
<dt>
REDUCED_ARRAY
<dd>
Represents a two-dimensional array of data,
which is reduced to a smaller dimension by averaging or taking extrema.
Includes member functions for drawing the reduced data as a 3D graph.
<dt>
PROGRESS and PROGRESS_2D
<dd>
Represent progress bars, depicted in 3 or 2 dimensions.

<dt>
RIBBON_GRAPH
<dd>
Represents of 3D graph of a function of 1 variable.

<dt>
MOVING_TEXT_PANEL
<dd>
Represents a flanged 3D panel, moving cyclically in 3 dimentions,
on which text is displayed.
<dt>
STARFIELD
<dd>
Represents a set of randomly-generated stars
that move forwards or backwards in 3 dimensions.

<dt>
TEXTURE_DESC
<dd>
Represents an image (JPEG, Targa, BMP or PNG)
displayed in 3 dimensions.
</dl>
";
page_tail();
?>
