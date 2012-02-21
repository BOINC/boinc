<?php
require_once("docutil.php");
page_head("Credit and FLOPS");
echo "
The average FLOPS (floating point operations per second)
achieved by a computer or group of computers
can be estimated from its Recent Average Credit (RAC) as follows:
<pre>
GigaFLOPs = RAC/100
TeraFLOPS = RAC/100,000
</pre>
(Remember that a 1 GigaFLOP machine, running full time,
produces 100 units of credit in 1 day).
<p>
The credit figures for a particular day may be distorted
if a project is catching up or falling behind on validation
(the process or granting credit).
Thus to get accurate FLOPS estimates you should
look at average RAC over a period of a week or so.
";
page_tail();
?>
