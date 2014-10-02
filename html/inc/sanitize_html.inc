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

require_once("../inc/htmLawed.php");

$allowed = array(
    'b' => array(),
    'i' => array(),
    'a' => array('href' => array('maxlen' => 150), 'title' => 1),
    'p' => array('align' => 1),
    'font' => array('face' => 1),
    'br' => array(),
    'strong' => array(),
    'ul' => array(),
    'ol' => array(),
    'li' => array(),
    'pre' => array(),
    'img' => array('src' => 1, 'border' => 1, 'height' => array('maxval' => 450)),
    'blockquote' => array(),
    'u' => array()
);


function sanitize_html($str) {
    global $allowed;
    return kses($str, $allowed);
}

?>
