<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

    init_session();

    page_head("Download debugging files");
    echo "
    <h2>Download debugging files</h2>
    <p>
    <b>Windows users</b>:
    <p>
    If the SETI@home application crashes,
    it is very helpful if you mail us the stack trace
    (which shows exactly where the crash occurred).
    To do this, you will need to have the symbol file
    on your computer.
    Click here to download the symbol file for
    the 2.22 version of SETI@home:
    <p>
    <a href=http://setiboinc.ssl.berkeley.edu/ap/download/setiathome_2.22_windows_intelx86.pdb>setiathome_2.22_windows_intelx86.pdb</a>
    <p>
    Place this file in the same directory as the executable
    (usually <code>C:/Program Files/BOINC/projects/http_setiboinc.berkeley.edu_ap</code>).
    <p>
    Windows 2000 users: Uncompressing the DLL file in
    <a href=http://setiboinc.ssl.berkeley.edu/ap/download/dbghelp.zip>dbghelp.zip</a>
    to the 
    <b>BOINC\projects\setiboinc.ssl.berkeley.edu_ap</b> directory gets around the 
    <b>SymGetLineFromAddr(): GetLastError = 126</b> error.
    ";
    page_tail();
?>
