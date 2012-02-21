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

// script to repair the "has_picture" field of profiles

ini_set("memory_limit", "1023M");

$cli_only = true;
require_once("../inc/util_ops.inc");

BoincDb::get();

$profiles = BoincProfile::enum("");

foreach ($profiles as $p) {
    $id = $p->userid;
    $path = "../user_profile/images/$id.jpg";
    $smpath = "../user_profile/images/".$id."_sm.jpg";
    $has_pic = file_exists($path);
    $has_pic_sm = file_exists($smpath);
    if ($p->has_picture) {
        if (!$has_pic || !$has_pic_sm) {
            echo "$id $p->has_picture $has_pic $has_pic_sm\n";
            BoincProfile::update_aux("has_picture=0 where userid=$id");
        }
    } else {
        if ($has_pic && $has_pic_sm) {
            echo "$id $p->has_picture $has_pic $has_pic_sm\n";
            BoincProfile::update_aux("has_picture=1 where userid=$id");
        }
    }
}

?>
