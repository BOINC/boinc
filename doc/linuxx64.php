<?php

require_once("docutil.php");

page_head("Running BOINC on Linux x64");

echo "
The BOINC client software should work on:

<ul>
<li> Most x64 based distros.
</ul>

<p>
Note: 32-bit binaries don't just work on every 64-bit Linux.
<p>
If for example you install a fresh Ubuntu 6.10 or 7.04, 32-bit binaries won't work. 
They are not even recognized as valid executables. You first have to install the ia32 
package and dependent packages.
<p>
Further, for programs that link with the graphic library, you will manually have to 
copy a 32-bit libglut library to the usr/lib32 directory. If after this they still get 
client errors, tell them to find your exe in the projects directory and run ldd to 
see what libraries are missing.
<p>
";

page_tail();
?>
