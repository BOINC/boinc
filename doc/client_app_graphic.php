<?php
require_once("docutil.php");
page_head("Screensaver/core/app interaction (graphics)");
echo "

TO BE WRITTEN

<p>
The graphics API uses a file <b>graphics.xml</b>
that is created and occasionally modified by the
core client or screensaver.
This file has the format
<pre>
    &lt;graphics_info>
        &lt;do_graphics/>
        &lt;xsize>500&lt;/xsize>
        &lt;ysize>400&lt;/ysize>
        &lt;full_screen/>
    &lt;/graphics_info>
</pre>

<p>
The graphics API implementation uses a 60 Hz timer.
Every 0.5 sec, it sees if graphics.xml has been modified, and if so parses it.
Every 1/60 sec, it sees if it's time for a new frame,
and if so calls <tt>app_render()</tt>.
<p>
Explain graphics modes of apps
<p>
Explain graphics use of shmem
<p>
Explain screensaver module
<p>
Explain screensaver logic in core
";
page_tail();
?>
