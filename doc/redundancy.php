<?
require_once("docutil.php");
page_head("Redundancy and errors");
echo "
A BOINC 'result' abstracts an instance of a computation,
possibly not performed yet.
Typically, a BOINC server sends 'results' to clients,
and the clients perform the computation and replies to the server.
But many things can happen to a result:
<ul>
<li> The client computes the result correctly and returns it.
<li> The client computes the result incorrectly and returns it.
<li> The client fails to download or upload files.
<li> The application crashes on the client.
<li> The client never returns anything because it breaks
or stops running BOINC.
<li> The scheduler isn't able to send the result because it
requires more resources than any client has.
</ul>

<p>
BOINC provides a form of redundant computing
in which each computation is performed on multiple clients,
the results are compared,
and are accepted only when a 'consensus' is reached.
In some cases new results must be created and sent.
<p>
BOINC manages most of the details;
however, there are two places where the application developer gets involved:

<ul>
<li> <b>Validation:</b>
This performs two functions.
First, when a sufficient number (a 'quorum') of successful results
have been returned, it compares them and sees if there is a 'consensus'.
The method of comparing results (which may need to take into
account platform-varying floating point arithmetic)
and the policy for determining consensus (e.g., best two out of three)
are supplied by the application.
If a consensus is reached, a particular result is designated
as the 'canonical' result.
Second, if a result arrives after a consensus has already been reached,
the new result is compared with the canonical result;
this determines whether the user gets credit.

<li> <b>Assimilation:</b>
This is the mechanism by which the project is notifed of the completion
(success or unsuccessful) of a work unit.
It is performed exactly once per work unit.
If the work unit was completed successfully
(i.e. if there is a canonical result)
the project-supplied function reads the output file(s)
and handles the information, e.g. by recording it in a database.
If the workunit failed,
the function might write an entry in a log,
send an email, etc.
</ul>

<hr>
In the following example,
the project creates a workunit with
<br>
min_quorum = 2
<br>
target_nresults = 3
<br>
max_delay =  10
<p>
BOINC automatically creates three results,
which are sent at various times.
At time 8, two successful results have returned
so the validater is invoked.
It finds a consensus, so the work unit is assimilated.
At time 10 result 3 arrives;
validation is performed again,
this time to check whether result 3 gets credit.
<pre>
time        0   1   2   3   4   5   6   7   8   9   10  11  12  13  14

            created                          validate; assimilate
WU          x                                x  x
                created sent            success
result 1        x       x---------------x
                created sent                success
result 2        x       x-------------------x
                created     sent                    success
result 3        x           x-----------------------x
</pre>
<hr>
In the next example,
result 2 is lost (i.e., there's no reply to the BOINC scheduler).
When result 3 arrives a consensus is found
and the work unit is assimilated.
At time 13 the scheduler 'gives up' on result 2
(this allows it to delete the canonical result's output files,
which are needed to validate late-arriving results).
<pre>
time        0   1   2   3   4   5   6   7   8   9   10  11  12  13  14

            created                                  validate; assimilate
WU          x                                        x  x
                created sent            success
result 1        x       x---------------x
                created sent    lost                            giveup
result 2        x       x--------                               x
                created     sent                    success
result 3        x           x-----------------------x
</pre>
<hr>
In the next example,
results 2 returns an error at time 5.
This reduces the number of outstanding results to 2;
because target_nresults is 3, BOINC creates another result (result 4).
A consensus is reached at time 9, before result 4 is returned.
<pre>
time        0   1   2   3   4   5   6   7   8   9   10  11  12  13  14

            created                              validate; assimilate
WU          x                                    x  x
                created sent            success
result 1        x       x---------------x
                created sent    error
result 2        x       x-------x
                created     sent                success
result 3        x           x-------------------x
                                 created     sent           success
result 4                         x   x----------------------x
</pre>
";
page_tail();
?>
