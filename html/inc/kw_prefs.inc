<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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
// read user prefs; return lists of yes and no keywords
//

// utility functions for user keyword prefs

define('KW_YES', 1);
define('KW_MAYBE', 0);
define('KW_NO', -1);

// read user prefs; return lists of yes and no keywords
//
function read_kw_prefs($user) {
    $path = "../../kw_prefs/$user->id";
    if (file_exists($path)) {
        $x = file($path);
        $yes = explode(' ', trim($x[0]));
        if ($yes[0]=='') $yes=[];
        $yes = array_map('intval', $yes);
        $no = explode(' ', trim($x[1]));
        if ($no[0]=='') $no=[];
        $no = array_map('intval', $no);
        return [$yes, $no];
    } else {
        return [[],[]];
    }
}

function write_kw_prefs($user, $yes, $no) {
    $y = implode(' ', array_map('strval', $yes));
    $n = implode(' ', array_map('strval', $no));
    $path = "../../kw_prefs/$user->id";
    file_put_contents($path, "$y\n$n\n");
}

?>
