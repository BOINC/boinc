<?php
// show summary of a workunit

require_once("../inc/db.inc");
require_once("../inc/result.inc");

db_init();
$wuid = get_int("wuid");
$wu = lookup_wu($wuid);
if (!$wu) {
    error_page("can't find workunit");
}

page_head("Workunit");
$app = lookup_app($wu->appid);

start_table();
row2("application", $app->user_friendly_name);
row2("created", time_str($wu->create_time));
row2("name", $wu->name);
if ($wu->canonical_resultid) {
    row2("canonical result", "<a href=result.php?resultid=$wu->canonical_resultid>$wu->canonical_resultid</a>");
    row2("granted credit", format_credit($wu->canonical_credit));
}
row2("minimum quorum", $wu->min_quorum);
row2("initial replication", $wu->target_nresults);
row2("max # of error/total/success results",
    "$wu->max_error_results, $wu->max_total_results, $wu->max_success_results"
);
if ($wu->error_mask) {
    row2("errors", wu_error_mask_str($wu->error_mask));
}
if ($wu->need_validate) {
    row2("validation", "Pending");
}
end_table();
project_workunit($wu);

result_table_start(false, true, true);
$result = mysql_query("select * from result where workunitid=$wuid");
while ($res = mysql_fetch_object($result)) {
    show_result_row($res, false, true, true);
}
mysql_free_result($result);
echo "</table>\n";
page_tail();

?>
