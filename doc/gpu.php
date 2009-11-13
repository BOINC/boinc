<?php

require_once("docutil.php");

page_head("Use your GPU for scientific computing");

echo "
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
<a href=http://developer.amd.com/gpu/ATIStreamSDK/pages/ATIStreamSystemRequirements.aspx>here</a>.
</ul>
<p>
<h3>2) Get the latest BOINC software</h3>
<ul>
<li> <a href=download.php>Download and install</a>
version 6.6.36 or later of the BOINC software.
<li>ATI GPUs: you'll need 6.10.3 or later.
</ul>
<p>
<h3>3) Get the latest driver</h3>
Run BOINC, and look at the Messages.
If BOINC reports a GPU, your current driver is OK.
Otherwise
<ul>
<li>
<a href=http://www.nvidia.com/page/drivers.html>Get latest NVIDIA driver</a>
<li>
<a href=http://support.amd.com/us/gpudownload/Pages/index.aspx>Get latest ATI driver</a>.
</ul>
<h3>4) Attach to projects with GPU applications</h3>
<p>
Projects with NVIDIA applications:
<ul>
<li> <a href=http://gpugrid.net>GPUgrid.net</a>
<li> <a href=http://setiathome.berkeley.edu>SETI@home</a>
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
";

page_tail();
?>
