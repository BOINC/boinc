<?php

require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/db.inc");
require_once("../inc/translation.inc");

$config = get_config();
$users_per_page = parse_config($config, "<users_per_page>");
if (!$users_per_page) {
    $users_per_page = 20;
}
define (ITEM_LIMIT, 10000);

// move this to db.inc at some point...
function get_top_participants($offset,$sort_by) {
    global $users_per_page;
    if ($sort_by == "total_credit") {
        $sort_order = "total_credit desc";
    } else {
        $sort_order = "expavg_credit desc";
    }
    $res = mysql_query("select * from user order by $sort_order limit $offset,".$users_per_page);
    while ($arr[]=mysql_fetch_object($res)){};
    return $arr;
}

// These converter functions are here in case we later decide to use something 
// else than serializing to save temp data
//
function participants_to_store($participants){
    return serialize($participants);
}
function store_to_participants($data){
    return unserialize($data);
}

if (isset($_GET["sort_by"])) {
    $sort_by = $_GET["sort_by"];
} else {
    $sort_by = "expavg_credit";
}

$offset = get_int("offset", true);
if (!$offset) $offset=0;
if ($offset % $users_per_page) $offset = 0;

if ($offset < ITEM_LIMIT) {
    $cache_args = "sort_by=$sort_by&offset=$offset";
    $cacheddata=get_cached_data(TOP_PAGES_TTL,$cache_args);

    // Do we have the data in cache?
    //
    if ($cacheddata){
        $data = store_to_participants($cacheddata); // use the cached data
    } else {
        //if not do queries etc to generate new data
        db_init(true);
        $data = get_top_participants($offset,$sort_by);

        //save data in cache
        //
        set_cache_data(participants_to_store($data),$cache_args);
    }
} else {
    error_page("Limit exceeded - Sorry, first ".ITEM_LIMIT." items only");
}

// Now display what we've got (either gotten from cache or from DB)
page_head(tr(TOP_PARTICIPANT_TITLE));
user_table_start($sort_by);
$i = 1 + $offset;
$o = 0;
while ($user = $data[$o]) {
    show_user_row($user, $i);
    $i++;
    $o++;
}
echo "</table>\n<p>";
if ($offset > 0) {
    $new_offset = $offset - $users_per_page;
    echo "<a href=top_users.php?sort_by=$sort_by&offset=$new_offset>Previous ".$users_per_page."</a> | ";

}
if ($o==$users_per_page){ //If we aren't on the last page
    $new_offset = $offset + $users_per_page;
    echo "<a href=top_users.php?sort_by=$sort_by&offset=$new_offset>Next ".$users_per_page."</a>";
}

page_tail();

?>
