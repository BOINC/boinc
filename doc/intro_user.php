<?php

require_once("docutil.php");

page_head("Getting started");

echo "
    It's easy to participate in a BOINC project.
    <ul>
    <li> <b>Create an account.</b>
    Go to the project's web site, click on <b>Create Account</b>,
    and fill out the form.
    You will receive an email containing
    your <b>account ID</b> (a long random string).
    Save this email.
    <li> <b>Download and install BOINC.</b>
    Download BOINC for your type of computer,
    install it, and run it.
    You will be asked to enter the project's URL and your account ID.
    </ul>
    That's it!
    <p>
    <h2>How it works</h2>
    When you run BOINC on your PC,
    it works as follows (see below):
    <br>
    <center>
    <img hspace=10 vspace=8 src=http://boinc.berkeley.edu/comm_simple.png>
    </center>
    <br>
    <ol>
    <li> Your PC gets a set of instructions from the project's
    <b>scheduling server</b>.
    The instructions depend on your PC: for example, 
    the server won't give it work that requires more RAM than you have.
    The instructions may include many multiple pieces of work.
    Projects can support several <b>applications</b>,
    and the server may send you work from any of them.
    <li>
    Your PC downloads executable and input files
    from the project's <b>data server</b>.
    If the project releases new versions of its applications,
    the executable files are downloaded automatically to your PC.
    <li> Your PC runs the application programs, producing output files.
    <li> Your PC uploads the output files to the data server.
    <li>
    Later (up to several days later, depending on your
    <a href=prefs.php>work buffer preferences</a>)
    your PC reports the completed results to the scheduling server,
    and gets instructions for more work.
    </ol>
    This cycle is repeated indefinitely.
    BOINC does this all automatically; you don't have to do anything.
    <h2>Credit</h2>
    The project's server keeps track of how much work
    your computer has done; this is called <b>credit</b>.
    To ensure that credit is granted fairly, BOINC works as follows:
    <ul>
    <li> Each work unit may be sent to several computers.
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

    Please keep in mind:
    <ul>
    <li> 
    There may be a delay of several days between
    when your computer reports a result
    and when it is granted credit for the result.
    Your <a href=home.php>User page</a> shows you how much credit is 'pending'
    (claimed but not granted).
    <li>
    The credit-granting process starts when your computer reports
    a result to the server
    (not when it finishes computing the result
    or uploading the output files).
    <li>
    In rare cases (e.g. if errors occur on one or more computers)
    you may never receive credit for a computation.
    </ul>
";
page_tail();

?>
