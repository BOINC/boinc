<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
page_head("Getting started");

echo "
    It's easy to participate in ".PROJECT."
    <ul>
    <li> <b>Create an account.</b>
    Go to the <a href=create_account_form.php>Create account</a> page
    and fill out the form.
    You will receive an email containing
    your <b>account ID</b> (a long random string).
    Save this email.
    <li> <b>Download and install BOINC.</b>
    Go to the <a href=http://boinc.berkeley.edu/download.php>download page</a>,
    download the BOINC software for your type of computer,
    install it, and run it.
    You will be asked to enter the project URL
    (".MASTER_URL.") and your account ID.
    </ul>
    That's it!
    <table cellpadding=8 ><tr><td bgcolor=ffffcc>
    <h2>How it works</h2>
    When you run BOINC on your PC,
    it works as follows (see below):
    <br>
    <center>
    <img hspace=10 vspace=8 src=http://boinc.berkeley.edu/comm_simple.png>
    </center>
    <br>
    <ol>
    <li> Your PC gets a list of instructions from the project's
    <b>scheduling server</b>.
    The instructions depend on your PC: for example, 
    the server won't give it work that requires more RAM than you have.
    The instructions may include many separate pieces of work.
    Projects can support several <b>applications</b>,
    and the server may send you work from any of them.
    <li> Your PC downloads executable and input files
    from the project's <b>data server</b>.
    If the project releases new versions of its applications,
    the executable files are downloaded automatically to your PC.
    <li> Your PC runs the application programs, producing output files.
    <li> Your PC uploads the output files to the data server.
    <li> Your PC reports the completed results to the scheduling server,
    and gets instructions for more work.
    This cycle is repeated indefinitely.
    </ol>
    BOINC does this all automatically;
    you don't have to do anything.
    </td></tr></table>
    <h2>Credit</h2>
    The project's server keeps track of how much work
    each participant has contributed;
    this is called <b>credit</b>.
    The following system is used
    to ensure that credit is granted fairly:
    <ul>
    <li> Each work unit is sent to at least two computers.
    <li> When a computer reports a result,
        it claims a certain amount of credit,
        based on how much CPU time was used.
    <li> When at least two results have been returned,
        the server compares them.
        If the results agree, then users are granted
        the smaller of the claimed credits.
    </ul>
    <br>
    <center>
    <img src=http://boinc.berkeley.edu/credit.png>
    </center>
    <br>
    There may be a delay of several days between
    when your computer reports a result
    and when it is granted credit for the result.
    Your <a href=home.php>User page</a> shows you how much credit is 'pending'
    (claimed but not granted).
    <br><br>
    <table cellpadding=8 width=100%><tr><td bgcolor=ffffcc>
    <h2>More information</h2>
    More detailed information about participating in BOINC projects
    is <a href=http://boinc.berkeley.edu/participate.php>here</a>.
    </td></tr></table>
";
page_tail();

?>
