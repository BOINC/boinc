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
    If the BOINC application crashes, it is very helpful if you mail us the stack trace
    (which shows exactly where the crash occurred).
    To do this, you will need to have the symbol file on your computer.
    <p>
    <b>BOINC core client</b>
    <p>
    The zipped symbol file(s) are for the BOINC core client 2.25
    (both GUI and CLI versions) are here:
    <a href=http://setiboinc.ssl.berkeley.edu/ap/download/boinc_225_pdb.zip>boinc_225_pdb.zip</a>
    <p>
    Place the extracted file(s) in the same directory as the executable(s)
    (usually <code>C:/Program Files/BOINC</code>).
    <p>
    <p>
    <h2>Sending Debug Results</h2>
    <p>
    The files we are interested in are user.dmp and drwtsn32.log, they can be found in the Dr. Watson folder
    (usually <code>C:/Documents and Settings/All Users/Application Data/Dr Watson</code>).
    <p>
    We would perfer the files to be zipped up with a compression program like winzip or gzip.
    <p>
    Be sure to include which version of BOINC and the project applications you are using in the email.
    <p>
    Thanks for helping make BOINC a better product.
    <p>
    <p>
";

    page_tail();
?>
