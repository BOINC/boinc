<?
require_once("docutil.php");
page_head("Work buffering");
echo "
<p>
Each BOINC client
maintains an estimate of the amount of work remaining
(i.e. the time until one of its processors will be idle).
Each host also has two preferences:
the <b>minimum work</b> and the
<b>maximum work</b>
(these are part of your 'general preferences', discussed elsewhere). 
<p>
Normally the work remaining is between these two limits.
When the work remaining reaches minimum level,
the client contacts one or more scheduling servers,
and attempts to get enough work to exceed the maximum level.
<p>
This scheme allows hosts that are sporadically connected
(because they're portable or have modem-based connections)
to avoid becoming idle due to lack of work. 
";
page_tail();
?>
