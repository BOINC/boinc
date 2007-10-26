<?php

require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/user.inc");
require_once("../inc/host.inc");
require_once("../inc/db.inc");
require_once("../inc/translation.inc");

$config = get_config();
$hosts_per_page = parse_config($config, "<hosts_per_page>");
if (!$hosts_per_page) {
        $hosts_per_page = 20;
}
define (ITEM_LIMIT,10000);

//Possibly move this to db.inc at some point...
function get_top_hosts($offset,$sort_by){
    global $hosts_per_page;
    if ($sort_by == "total_credit") {
	$sort_order = "total_credit desc";
    } else {
        $sort_order = "expavg_credit desc";
    }
    $res=mysql_query("select * from host order by $sort_order limit $offset,".$hosts_per_page);
    echo mysql_error();
    while ($arr[]=mysql_fetch_object($res)){};
    return $arr;
}

function hosts_to_store($participants){ //These converter functions are here in case we later decide to use something 
    return serialize($participants);	       //else than serializing to save temp data
}
function store_to_hosts($data){
    return unserialize($data);
}

if (isset($_GET["sort_by"])) {
    $sort_by = $_GET["sort_by"];
} else {
    $sort_by = "expavg_credit";
}

$offset = get_int("offset", true);
if (!$offset) $offset=0;
if ($offset % $hosts_per_page) $offset = 0;

if ($offset < ITEM_LIMIT) {
    $cache_args = "sort_by=$sort_by&offset=$offset";
    db_init(true);
    $cacheddata=get_cached_data(TOP_PAGES_TTL,$cache_args);
    if ($cacheddata){ //If we have got the data in cache
	$data = store_to_hosts($cacheddata); // use the cached data
    } else { //if not do queries etc to generate new data
	$data = get_top_hosts($offset,$sort_by);
	set_cache_data(hosts_to_store($data),$cache_args); //save data in cache
    };
} else {
    error_page("Limit exceeded - Sorry, first ".ITEM_LIMIT." items only");
}


//Now display what we've got (either gotten from cache or from DB)
page_head(tr(TOP_HOST_TITLE));
top_host_table_start($sort_by);
$i = 1 + $offset;
$o = 0;
while ($host = $data[$o]) {
    show_host_row($host, $i, false, true);
    $i++;
    $o++;
}
echo "</table>\n<p>";
if ($offset > 0) {
    $new_offset = $offset - $hosts_per_page;
    echo "<a href=top_hosts.php?sort_by=$sort_by&offset=$new_offset>Previous ".$hosts_per_page."</a> | ";

}
if ($o==$hosts_per_page){ //If we aren't on the last page
    $new_offset = $offset + $hosts_per_page;
    echo "<a href=top_hosts.php?sort_by=$sort_by&offset=$new_offset>Next ".$hosts_per_page."</a>";
}

page_tail();


?>
