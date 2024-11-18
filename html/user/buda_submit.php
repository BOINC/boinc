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

// web interface for submitting BUDA jobs

require_once('../inc/util.inc');
require_once('../inc/sandbox.inc');

display_errors();

function submit_form($user) {
    $sbitems_zip = sandbox_select_items($user, '/.zip$/');
    if (!$sbitems_zip) {
        error_page("No .zip files in your sandbox.");
    }
    $app = get_str('app');
    $variant = get_str('variant');

    page_head("Submit jobs to $app ($variant)");
    form_start('buda_submit.php');
    form_input_hidden('action', 'submit');
    form_input_hidden('app', $app);
    form_input_hidden('variant', $variant);
    form_input_text('Batch name', 'batch_name');
    form_select('Job file', 'job_file', $sbitems_zip);
    form_submit('OK');
    form_end();
    page_tail();
}

function handle_submit() {
    // stage app files if not already staged
    //

    // create batch
    //

    // unzip batch file
    //

    // stage top-level input files
    //

    // scan jobs; stage per-job input files and make create_work input
    //

    // create jobs
}

$user = get_logged_in_user();
$action = get_str('action', true);
if ($action == 'submit') {
    handle_submit();
} else {
    submit_form($user);
}

?>
