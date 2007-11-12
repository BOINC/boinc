<?php

// show summary of a workunit

require_once("../inc/boinc_db.inc");
require_once("../inc/result.inc");

$wuid = get_int("wuid");
$wu = BoincWorkunit::lookup_id($wuid);
if (!$wu) {
    error_page("can't find workunit");
}

page_head("Workunit details");
$app = BoincApp::lookup_id($wu->appid);

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
row2("max # of error/total/success tasks",
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
$results = BoincResult::enum("workunitid=$wuid");
foreach ($results as $result) {
    show_result_row($result, false, true, true);
}
echo "</table>\n";
page_tail();

?>
