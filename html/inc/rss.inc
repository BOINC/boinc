<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2021 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// functions for fetching and displaying RSS feeds

require_once("../inc/cache.inc");

// parse an RSS feed into objects
//
function get_rss_feed($url) {
    $x = @simplexml_load_file($url);
    if (!$x) return null;
    $x = $x->channel;
    $items = array();
    foreach ($x->item as $item) {
        $y = new StdClass;
        $y->title = (string)$item->title;
        $y->link = (string)$item->link;
        $y->description = (string)$item->description;
        $y->pubDate = (string)$item->pubDate;
        $items[] = $y;
    }
    return $items;
}

// same, but cache the results; $dur is the refresh period
//
function get_rss_feed_cached($url, $dur) {
    $d = get_cached_data($dur, $url);
    if ($d) {
        return unserialize($d);
    }
    $items = get_rss_feed($url);
    if (!$items) return null;
    set_cached_data($dur, serialize($items), $url);
    return $items;
}

// Show up to $n items.
// You can pass a function $filter;
// if $filter(item) returns true, don't show it.
//
function show_rss_items($items, $n=0, $filter=null) {
    $i = 0;
    foreach ($items as $item) {
        if ($filter && $filter($item)) {
            continue;
        }
        echo sprintf(
            '<h4>%s</h4>
            <p>
            %s
            <p>
            <a href=%s>View article</a> &middot; %s
            <hr>
            ',
            $item->title,
            $item->description,
            $item->link,
            $item->pubDate
        );
        $i++;
        if ($i == $n) {
            break;
        }
    }
}

?>
