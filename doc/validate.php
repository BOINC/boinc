<?php
require_once("docutil.php");
page_head("Validation");
echo "
<p>
In BOINC, the process of <b>validation</b> does two things:
<ul>
<li> it compares redundant results
and decides which ones are to be considered correct;
<li> it decides how much credit to grant to each correct result.
</ul>
<p>
A <b>validator</b> is a daemon program.
You must supply a validator for each application in your project,
and include it in the &lt;daemons&gt; section of your
<a href=configuration.php>project configuration file</a>.

<p>
Depending on various factors,
you may be able to use a standard validator that comes with BOINC,
or you may have to develop a custom validator.
<ul>
<li>
If your application generates exactly matching results
(either because it does no floating-point arithmetic, or because you use
<a href=homogeneous_redundancy.php>homogeneous redundancy</a>)
then you can use the 'sample bitwise validator' (see below).
<li>
If you are using BOINC for 'desktop grid' computing
(i.e. you trust all the participating hosts)
then you can use the 'sample trivial validator' (see below).
<li>
Otherwise, you'll need to develop a custom validator for your application.
BOINC supplies a <a href=validate_simple.php>simple validator framework</a>
in which you plug in three short application-specific functions.
This is sufficient for most projects.
If you need more control over the validation process,
you can use BOINC's <a href=validate_advanced.php>advanced validator framework</a>.
</ul>

<p>
Two standard validators are supplied:
<ul>
<li>
The <b>sample_bitwise_validator</b> requires a strict majority,
and regards results as equivalent only if they agree byte for byte.
<li>
The <b>sample_trivial_validator</b>
regards any two results as equivalent if their CPU time
exceeds a given minimum.
</ul>
<p>

<h3>Command-line arguments</h3>
A validator (either standard or custom) has the following command-line arguments:
";
list_start();
list_item("-app appname", "Name of the application");
list_item("[ -one_pass_N_WU N ]", "Validate at most N WUs, then exit");
list_item("[ -one_pass ]", "Make one pass through WU table, then exit");
list_item("[ -mod n i ]",
    "Process only WUs with (id mod n) == i.
    This option lets you run multiple instances of the validator
    for increased performance."
);
list_item("[ -max_claimed_credit X ]",
    "If a result claims more credit than this, mark it as invalid."
);
list_item("<nobr>[ -max_granted_credit X ]</nobr>",
    "Grant no more than this amount of credit to a result."
);
list_item("[ -grant_claimed_credit ]",
    "If set, grant the claimed credit,
    regardless of what other results for this workunit claimed.
    These is useful for projects where
    different instances of the same job
    can do much different amounts of work."
);

list_end();
page_tail();
?>
