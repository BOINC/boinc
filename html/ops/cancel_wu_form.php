<?php
// use this to create accounts while regular account creation is disabled

require_once("../inc/util_ops.inc");

echo "
    <h2>Cancel ".PROJECT." workunit(s)</h2>
    <form action=cancel_wu_action.php>
";
echo "
	This form may be used to cancel unnecessary or unwanted workunits.  We recommend that
	you stop the project before doing this.  Note that the workunits and their corresponding
        results (if any) are NOT removed from the database.  Instead, they are marked as 'no longer
        needed'.  In most cases you should probably only remove workunits whose results are all unsent,
        since otherwise a user will not get credit for a result that they might return.
";
// TODO: David, a query that shows all workunits that do not have all results unsent is:
// select distinct workunit.id,workunit.name from workunit join result where workunit.id=result.workunitid and result.server_state!=2 order by workunit.id
// What is the inverse of this query?  Ie select all workunits all of whose results are unsent.  This would
// be useful to incorporate into this page.

start_table();
row2("First Workunit (ID) to cancel", "<input size=32 name='wuid1'");
row2("Last  Workunit (ID) to cancel", "<input size=32 name='wuid2'");
row2("", "<input type=submit value='CANCEL WORKUNITS'>");
echo "
    </form>
";

?>
