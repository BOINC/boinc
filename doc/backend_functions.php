<?
require_once("docutil.php");
page_head("Back end functions");
echo "

<p>
The general functions of a project back end include:
<ul>
<li> <b>Generate work</b>.
<p>
<li> <b>Reissue results</b>.
Hosts may fail to return results for various reasons;
such results are 'lost'.
Lost and erroneous results may prevent
finding a canonical result for a workunit.
The 'result reissue' mechanism generates additional
results as needed to find a canonical result.

<p>
<li> <b>Select canonical results</b>.
Communication from the core client can easily be altered or forged.
Output files may be wrong.
This can result from tampering or hardware failures.
This problem can be addressed
by <b>redundant computing</b>
In this approach, each workunit is processed at least twice.
The project back end waits until a minimum number of results have been returned,
then compares the results and decides which are considered correct.
The notion of equality of results,
and the policy for deciding which are correct, are project-specific.
<p>

<li> <b>Grant credit</b>.
Some users will attempt to get undeserved credit by falsifying their CPU
metrics or CPU times.  Each project and application can have its own
credit-granting algorithm, for example granting the minimum or the mean of
the median of all claimed credits (during validation time).  The granted
credit is assigned to all correct results. This ensures that as long as a
reasonable majority of participants don't falsify credit, almost all credit
accounting will be correct.
<p>
<li> <b>Assimilate results</b>.
<p>
<li> <b>Delete files</b>.
<p>
<li> <b>Detect 'problem' workunits</b>.
</ul>
";
page_tail();
?>
