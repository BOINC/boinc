<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2016 University of California
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

// This page implements the old model where the user downloads BOINC
// from the BOINC web site,
// and creates an account using the Manager.
//
// Link to here (e.g. from front-page Join button
// if you're not a vetted project.
// If you are, link to signup.php instead
//
// This page routes people to the right place depending on whether
// they already have BOINC installed on this device.

// DEPRECATED.  Use signup.php instead

require_once("../inc/util.inc");

// "old" (misnomer) means BOINC is already installed on this device

function show_choose($is_old) {
    panel(null,
        function() use($is_old) {
            echo tra("Is BOINC installed on this device?");
            $y = tra("Yes");
            $n = tra("No");
            if ($is_old) {
                echo sprintf(
                    ' <b>%s</b> &nbsp; |&nbsp; <a href="join.php">%s</a>',
                    $y, $n
                );
            } else {
                echo sprintf(
                    ' <a href="join.php?old=1">%s</a> &nbsp; |&nbsp; <b>%s</b>',
                    $y, $n
                );
            }
        }
    );
}

function show_new() {
    $master_url = master_url();
    panel(null,
        function() use ($master_url) {
            echo '
                <ol>
                <li> '
                .tra('Read our %1 Rules and Policies %2.', '<a href="info.php">', '</a>')
                .'<li> <p>'
                .tra('Download and install BOINC.')
                    .'</p><p>
                    <a href="http://boinc.berkeley.edu/download.php" class="btn btn-success">'.tra('Download').'</a>
                    </p><p>'
                    .tra('For Android devices, download BOINC from the Google Play Store or Amazon App Store.')
                    .'</p>
                <li> '
                .tra('Run BOINC.').'
                <li> '.tra("Choose %1 from the list, or enter %2", "<strong>".PROJECT."</strong>", "<strong>$master_url</strong>").'
                </ol>
            ';
        }
    );
}

function show_old() {
    $master_url = master_url();
    panel(null,
        function() use($master_url) {
            echo '
                <ul>
                <li> '
                .tra('Read our %1 Rules and Policies %2.', '<a href="info.php">', '</a>')
                .'<p><li> '
                .tra('In the BOINC Manager, select Tools / Add Project.')
                .'<p><li> '
                .tra('Choose %1 from the list, or enter %2', "<strong>".PROJECT."</strong>", "<strong>$master_url</strong>")
                .'</ul>
            ';
        }
    );
}

$old = get_int('old', true);

page_head(tra("Join %1", PROJECT));
show_choose($old);
if ($old) {
    show_old();
} else {
    show_new();
}
page_tail();
