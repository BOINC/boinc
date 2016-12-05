<?php

require_once("../inc/util.inc");

function search_form($url) {
    page_head("Site search");
    echo '
        <form class="form-inline" method="get" action="https://google.com/search">
        <input type=hidden name=domains value="'.$url.'">
        <input type=hidden name=sitesearch value="'.$url.'">
        <div class="form-group">
        <input class="btn btn-success form-control" type="submit" value='.tra("Search").'>
        <input type="text" class="form-control" name="q" size="20" placeholder="keywords">
        </div>
        </form>
    ';
    page_tail();
}

search_form($master_url);

?>
