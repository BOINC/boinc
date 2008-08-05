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

require_once("../inc/util.inc");
require_once("../inc/bossa.inc");
require_once("../inc/bossa_example4.inc");

function handle_add($job, $inst) {
    $f = null;
    $f->x = get_int('pic_x');
    $f->y = get_int('pic_y');
    $f->type = get_str('type');
    $c = get_str('comment', true);
    if (strstr($c, "(optional)")) $c = "";
    $f->comment = $c;
    $output = $inst->get_opaque_data();
    $output->features[] = $f;
    $inst->set_opaque_data($output);
    header("location: bossa_example4.php?bji=$inst->id");
}

function handle_delete($job, $inst, $index) {
    $output = $inst->get_opaque_data();
    $features = $output->features;
    array_splice($features, $index, 1);
    $output->features = $features;
    $inst->set_opaque_data($output);
    header("location: bossa_example4.php?bji=$inst->id");
}

$bji = get_int("bji");
if (!bossa_lookup_job($bji, $job, $inst, $u)) {
    error_page("No such instance");
}
$user = get_logged_in_user();
if ($u->id != $user->id) {
    error_page("Not your job");
}

$action = get_str("action", true);
switch ($action) {
case "add":
    handle_add($job, $inst);
    break;
case "delete":
    $index = get_int("index");
    handle_delete($job, $inst, $index);
    break;
default:
    job_show($job, $inst, $user);
    break;
}

?>
