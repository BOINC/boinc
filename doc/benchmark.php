<?php
require_once("docutil.php");
page_head("More about benchmarks");
echo "
<h2>How benchmarks are calculated</h2>

<p>
'Whetstone' is the name of the benchmark that is reported on your
[Show computers] web page as 'Measured floating point speed'.
Dhrystone is the name of the benchmark used for 'Measured integer speed'.
Floats can have fractional parts (like 1.48283 or 3.141592);
integers are whole numbers like 1, 2, 938283 or 2004.

Whetstone does 8 different groups of tests (repeatedly of course),
times how long they took to finish, and produces a number,
[ops performed]/[time].
These tests all use floating point math operations of the CPUs being tested.
Some of them are simple math (addition, multiplication, division)
while others compute trigonometric and exponential functions
(sine, cosine, tangent, exponent).


Dhrystone checks repeated integer operations
and several operating system file handling operations.

<p>
Neither of the tests really checks how well/fast a system can access memory,
and SETI@home (for example) accesses memory a lot.

<p>
Here is an example of memory introducing a delay:
A Pentium 4 CPU of any speed can calculate the sine of an angle in
approximately 170 ticks of its internal clock.
It could have performed 170 regular integer additions in this time.

<p>
But if it wanted to do an integer addition on a number somewhere out in memory
(say it was working on a table of numbers), the
CPU might have to wait as much as 260 ticks
for this memory integer to be delivered to the CPU.
So a badly timed integer+memory
operation would take far longer than a sine calculation.

<p>
This is where Celeron CPUs can really slow down.
Pentium has many features to predict when the CPU might be getting memory,
and begins getting it long before the CPU actually calculates with it.
Thus there is much less delay for most memory operations.

<h2>Why 'predicted time' can be wrong</h2>
Each WU delivered to your machine includes an estimated number of
floating point (FP) calculations.
BOINC divides this by the FP
benchmark number to estimate completion time.
SETI@home's WUs estimate number is currently always 27.9 trillion (american),
however the actual number of FP ops varies greatly which is why
WUs take different amounts of time to finish.


<p>
SETI@home uses almost all single-precision floating point math,
while Whetstone is all double-precision math.
On Intel x86 processors
the speed difference in calculating single vs. double isn't very large.

<p>
SETI@home uses mostly add, sub, multiply and divide.
About 20% of its time is spent in trigonometry.
Almost all the time in Whetstone is used for trigonometry.

<p>
Memory access speed and trigonometry are the two major reasons that
the benchmark results and SETI@home processing speed don't match
up on many systems.

<br><br>
<i>Thanks to Ben Herndon for this writeup</i>
";
page_tail();
?>
