<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

function news_item($date, $text, $title="") {
    echo "
        <span class=news_title>$title</span>
        <br><span class=news_date>$date</span>
        <br><span class=news_content>$text</span>
        <br>
    ";
}

function show_news($items, $n) {
    if ($n > count($items)) {
        $n = count($items);
    }
    for ($i=0; $i<$n; $i++) {
        $title = null;
        if (isset($items[$i][2])) $title = $items[$i][2];
        news_item($items[$i][0], $items[$i][1], $title);
    }
}

function show_old_news($items, $n) {
    $tot = count($items);
    for ($i=$n; $i<count($items); $i++) {
        $j = $tot-$i;
        echo "<a name=$j></a>\n";
        news_item($items[$i][0], $items[$i][1]);
    }
}

$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit
?>
