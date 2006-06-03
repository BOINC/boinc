<?php
require_once("docutil.php");
page_head("Energy considerations");
echo "
<h2>The energy cost and environmental impact of running BOINC</h2>
<p>
A computer running BOINC uses more energy than an idle computer.
The amount of additional energy depends on several factors:
<ul>
<li> How many additional hours per day you
    you leave your computer powered on to run BOINC.
<li> The energy consumption of your computer.
<li> Whether you use your computer's power-management features
to turn off your monitor when idle.
<li> Your marginal electricity cost.
</ul>
Using some typical values for energy usage and cost,
here are estimates of the monthly costs:
";
list_start();
list_heading_array(array(
    "Computer state<br>(24 hrs/day)",
    "Typical power usage",
    "Energy per month",
    "Cost per month<br><font size=-2>assuming USA average of 8 cents/kWh;
    <br>the cost in Europe is about 50% higher.</font>"
    ));
list_item_array(array("Off", "0 watts", "0 kWh", "$0"));
list_item_array(array("Idle", "100 watts", "73 kWh", "$5.84"));
list_item_array(array("Active", "150 watts", "110 kWh", "$8.80"));
list_end();
echo "
Under these assumptions, running BOINC costs about $3/month
relative to leaving your computer on but idle,
and about $8.80/month relative to leaving it off all the time.

<p>
There may also be an environmental cost.
If your electricity is produced by burning fossil fuels,
the extra electricity usage produces greenhouse gases
that contribute to global warming.
If this is the case, we recommend that you
not leave your computer on just to run BOINC,
or that you reduce your overall energy use to compensate.

<p>
You can measure the power usage of their computer using
a power consumption meter.
These are available for
<a href=http://www.energydudes.com/proddetail.php?prod=0001>American</a>
<a href=http://www.pat-training.co.uk/230V_electricity_meter.htm>UK-type</a>
electrical outlets.

<h2>How reduce BOINC's energy usage and cost</h2>
Some possibilities:
<ul>
<li> 
Set your <a href=prefs.php>general preferences</a> to allow BOINC
to compute while your computer is in use,
and turn your computer off when it's not in use.
<li> Use your computer's power-management features to
turn off your monitor when it's not in use,
or to enter a low-power mode.
A <a href=http://eetd.lbl.gov/EA/Reports/39466/>
technical document about computer power management</a>
is available from Lawrence Berkeley Laboratories.
<li> If your electricity costs vary according to time of day,
set your <a href=prefs.php>general preferences</a>
so that BOINC computes only during periods of low electricity costs.
";
page_tail();
?>
