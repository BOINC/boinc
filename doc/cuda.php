<?php

require_once("docutil.php");

page_head("Use your NVIDIA GPU for scientific computing");

echo "
<img align=right width=264 src=images/NV_DesignedFor_CUDA_3D_sm.png>
Most computers are equipped with a
<b>Graphics Processing Unit (GPU)</b>
that handles their graphical output,
including the 3-D animated graphics used in computer games.
The computing power of GPUs has increased rapidly,
and they are now often much faster than
the computer's main processor, or CPU.
<p>
NVIDIA (a leading GPU manufacturer)
has developed a system called CUDA that uses GPUs for scientific computing.
With NVIDIA's assistance,
some BOINC-based projects have applications that run on NVIDIA GPUs using CUDA.
<b>These applications run from 2X to 10X faster than the CPU-only version.
We urge BOINC participants to use them if possible.</b>
<p>
Just follow these instructions:
<p>
<b>1) Check whether your computer has a CUDA-capable GPU</b>
<dd>
CUDA programs work on most newer NVIDIA GPUs.
To find out if your GPU is compatible:
<ul>
<li> Identify the model name of your GPU.
On Windows, right-click on your desktop, and select Properties / Settings / Advanced / Adapter.
   Note the Adapter Type and Memory Size.
<li> Check <a href=http://www.nvidia.com/object/cuda_learn_products.html>NVIDIA's list of CUDA-enabled products</a>.
If your GPU is listed here and has at least 256MB of RAM, it's compatible.
</ul>
</dd>
<p>
<b>2) Get the latest NVIDIA driver</b>
<dd>
<a href=http://www.nvidia.com/page/drivers.html>Download and install the latest driver</a> (a reboot will be required).
</dd>
<p>
<b>3) Get the latest BOINC software</b>
<dd>
<a href=download.php>Download and install</a>
version 6.4.5 or later of the BOINC software.
</dd>

<p>
You're done!
Now start up BOINC, and soon you'll be racking up big credit numbers.
<p>
Projects with CUDA applications:
<ul>
<li> <a href=http://gpugrid.net>GPUgrid.net</a>
<li> <a href=http://setiathome.berkeley.edu>SETI@home</a>
</ul>

Of course, you can attach to other projects too;
BOINC will keep both your CPU and GPU busy.
";

page_tail();
?>
