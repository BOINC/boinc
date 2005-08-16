<?php

require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/db.inc");
require_once("../inc/translation.inc");

define (ITEMS_PER_PAGE, 20);
define (ITEM_LIMIT,10000);



function get_top_teams($offset,$sort_by,$type=""){ //Possibly move this to db.inc at some point...
    if ($type){
	$type_sql = "where type=".(int)$type;
    }
    if ($sort_by == "total_credit") {
	$sort_order = "total_credit desc";
    } else {
        $sort_order = "expavg_credit desc";
    }
    $res=mysql_query("select * from team $type_sql order by $sort_order limit $offset,".ITEMS_PER_PAGE);
    while ($arr[]=mysql_fetch_object($res)){};
    return $arr;
}

function teams_to_store($participants){ //These converter functions are here in case we later decide to use something 
    return serialize($participants);	       //else than serializing to save temp data
}
function store_to_teams($data){
    return unserialize($data);
}

if (isset($_GET["sort_by"])) {
    $sort_by = $_GET["sort_by"];
} else {
    $sort_by = "expavg_credit";
}

$type = get_int("type", true);
if ($type < 1 || $type > 7) {
    $type = null;
}
$type_url="";$type_sql="";$type_name="";
if ($type) {
    $type_url = "&type=$type";
    $type_name = team_type_name($type);
}


$offset = get_int("offset", true);
if (!$offset) $offset=0;
if ($offset % $n) $offset = 0;

if ($offset < ITEM_LIMIT) {
    $cache_args = "sort_by=$sort_by&offset=$offset&type=$type";
    $cacheddata=get_cached_data(TOP_PAGES_TTL,$cache_args);
    if ($cacheddata){ //If we have got the data in cache
	$data = store_to_teams($cacheddata); // use the cached data
    } else { //if not do queries etc to generate new data
	db_init();
	$data = get_top_teams($offset,$sort_by,$type);
	set_cache_data(teams_to_store($data),$cache_args); //save data in cache
    };
} else {
    error_page("Limit exceeded - Sorry, first ".ITEM_LIMIT." items only");
}


//Now display what we've got (either gotten from cache or from DB)
page_head(sprintf(tr(TOP_TEAMS_TITLE),$type_name));
start_table();
team_table_start($sort_by,$type_url);
$i = 1 + $offset;
$o = 0;
while ($team = $data[$o]) {
    show_team_row($team, $i);
    $i++;
    $o++;
}
echo "</table>\n<p>";
if ($offset > 0) {
    $new_offset = $offset - ITEMS_PER_PAGE;
    echo "<a href=top_teams.php?sort_by=$sort_by&offset=$new_offset".$type_url.">Previous ".ITEMS_PER_PAGE."</a> | ";

}
$new_offset = $offset + ITEMS_PER_PAGE;
echo "<a href=top_teams.php?sort_by=$sort_by&offset=$new_offset".$type_url.">Next ".ITEMS_PER_PAGE."</a>";

page_tail();


?>
