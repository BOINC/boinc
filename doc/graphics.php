<?
require_once("docutil.php");
page_head("The BOINC graphics API");
echo"
<p>
Applications can optionally generate graphics,
which are displayed either in an application window,
or in full-screen mode (screensaver).
<p>
If an application is able to generate graphics, it should call
<pre>
    void boinc_init_opengl();
</pre>
at the start and
<pre>
    void boinc_finish_opengl();
</pre>
at the end.
It must also supply a function
<pre>
    bool app_render(int xs, ys, double time_of_day);
</pre>
which will be called periodically.
This should make openGL calls to generate the current graphic.
<tt>xs</tt> and <tt>ys</tt> are the X and Y sizes of the window,
and <tt>time_of_day</tt> is the relative time in seconds.
The function should return true if it actually drew anything.
It can refer to the user name, CPU time etc. obtained from
<tt>boinc_init()</tt>.
<p>
<b>Give example here</b>
<p>
Applications that don't do graphics must also supply a
dummy <tt>app_render</tt> to link with the API.
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
