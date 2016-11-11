<?php

require_once("../inc/util.inc");

function search_form($url) {
    page_head("Site search");
    echo '
        <form method=get action="https://google.com/search">
        <input type=hidden name=domains value="'.$url.'">
        <input type=hidden name=sitesearch value="'.$url.'">
        <span class="nobar">
        <input class="small" name="q" size="20">
        <input class="small" type="submit" value='.tra("Search").'>
        </span>
        </form>
    ';
    page_tail();
}

$master_url = parse_config(get_config(), "<master_url>");

search_form($master_url);

?>
