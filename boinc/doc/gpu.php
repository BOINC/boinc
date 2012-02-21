<?php

require_once("docutil.php");

page_head("Use your GPU for scientific computing");

echo "

THIS PAGE IS DEPRECATED: USE
http://boinc.berkeley.edu/wiki/GPU_computing
instead
<p>
<img align=right src=images/cuda.png>
Most computers are equipped with a
<b>Graphics Processing Unit (GPU)</b>
that handles their graphical output,
including the 3-D animated graphics used in computer games.
The computing power of GPUs has increased rapidly,
and they are now often much faster than
the computer's main processor, or CPU.
<p>
Some BOINC-based projects have applications that run on GPUs.
<b>These applications run from 2X to 10X faster than the CPU-only version.
We urge BOINC participants to use them if possible.</b>
Just follow these instructions:
<p>
<h3>1) Check whether your computer has a capable GPU</h3>
<img align=right src=images/ati-stream.jpg>
<ul>
<li> Identify the model name of your GPU.
On Windows, right-click on your desktop, and select Properties / Settings / Advanced / Adapter.
   Note the Adapter Type and Memory Size.
<li>
To find out if your NVIDIA GPU is compatible:
check <a href=http://www.nvidia.com/object/cuda_learn_products.html>NVIDIA's list of CUDA-enabled products</a>.
If your GPU is listed here and has at least 256MB of RAM, it's compatible.
<li>ATI GPUs: you need a platform based on the AMD R600 GPU or later.
R600 and newer GPUs are found with ATI Radeon HD2400,
HD2600, HD2900 and HD3800 graphics board.
A full list is
<a href=http://developer.amd.com/gpu/ATIStreamSDK/Pages/default.aspx#two>here</a>.
</ul>
<p>
<h3>2) Get the latest BOINC software</h3>
<ul>
<li> <a href=download.php>Download and install</a>
the latest version of the BOINC software.
</ul>
<p>
<h3>3) Get the latest driver</h3>
Run BOINC, and look at the Messages.
If BOINC reports a GPU, your current driver is OK.
Otherwise
<ul>
<li>
<a href=http://www.nvidia.com/page/drivers.html>Get latest NVIDIA driver</a>
(Mac users: you need a special
 <a href=http://www.nvidia.com/object/cuda_get.html>CUDA Driver</a>).
<li>
<a href=http://support.amd.com/us/gpudownload/Pages/index.aspx>Get latest ATI driver</a>.
</ul>
<h3>4) Attach to projects with GPU applications</h3>
<p>
Projects with NVIDIA applications:
<ul>
<li> <a href=http://gpugrid.net>GPUgrid.net</a>
<li> <a href=http://setiathome.berkeley.edu>SETI@home</a>
<li> <a href=http://milkyway.cs.rpi.edu/milkyway/>Milkyway@home</a>
<li> <a href=http://aqua.dwavesys.com/>AQUA@home</a>
<li> <a href=http://boinc.umiacs.umd.edu/>Lattice</a>
<li> <a href=http://boinc.thesonntags.com/collatz/>Collatz Conjecture</a>
</ul>

Projects with ATI applications:
<ul>
<li> <a href=http://boinc.thesonntags.com/collatz/>Collatz Conjecture</a>
<li> <a href=http://milkyway.cs.rpi.edu/milkyway/>Milkyway@home</a> (coming soon)
</ul>

<p>
You're done!
Soon you'll be racking up big credit numbers.
Of course, you can attach to other projects too;
BOINC will keep both your CPU and GPU busy.

<h3>Some things to be aware of</h3>

<ul>
<li> By default, your GPU will be used only when you're
not using the computer;
otherwise graphical updates become jerky.
If you want to use the GPU all the time, you must change your
<a href=http://boinc.berkeley.edu/wiki/Preferences>preferences</a>.
<li> You can <a href=http://boinc.berkeley.edu/wiki/Client_configuration#Options>configure BOINC</a> to not use GPUs when particular applications are running.
<li> You can <a href=http://boinc.berkeley.edu/wiki/Client_configuration#Options>configure BOINC</a> to not use specific GPUs on a multi-GPU system.
<li> If you have questions or problems related to GPUs,
check the page at
at <a href=http://boincfaq.mundayweb.com/index.php?language=1&view=471>BOINCFAQ</a>.
</ul>
";

page_tail();
?>
