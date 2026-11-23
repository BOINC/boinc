<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

// show job submitters their sticky files, and let them 'release' them
// if they're not being used by active batches

require_once('../inc/util.inc');
require_once('../inc/submit_db.inc');
require_once('../inc/submit_util.inc');
require_once('../inc/dir_hier.inc');

function main($msg=null) {
    $user = get_logged_in_user();

    $sfs = BoincStickyFile::enum("user_id=$user->id");
    page_head('Sticky files');
    if ($msg) {
        echo "<p>$msg<hr><p>\n";
    }
    if (!$sfs) {
        echo 'No sticky files';
    } else {
        start_table('table-striped');
        table_header(
            'ID<br><small>click for details</small>',
            'Sandbox name', 'Logical name', 'Created', 'Batch', 'MD5'
        );
        foreach ($sfs as $sf) {
            $sfrs = BoincStickyFileRef::enum(
                "sticky_file_id=$sf->id order by id limit 1"
            );
            $sfr = $sfrs[0];
            table_row(
                sprintf(
                    '<a href=sticky.php?action=detail&sf_id=%d>%d</a>',
                    $sf->id, $sf->id
                ),
                $sfr->sandbox_name, $sfr->logical_name,
                date_str($sf->create_time),
                sprintf(
                    '<a href=submit.php?action=query_batch&batch_id=%d>%d</a>',
                    $sfr->batch_id,
                    $sfr->batch_id
                ),
                $sf->md5
            );
        }
        end_table();
    }
    page_tail();
}

function sf_detail($sf_id) {
    $sf = BoincStickyFile::lookup_id($sf_id);
    if (!$sf) error_page('no sticky file');
    if ($sf->user_id != get_logged_in_user()->id) error_page('no access');
    page_head("Sticky file $sf_id");
    start_table('table-striped');
    row2('Size', size_string($sf->size));
    row2('MD5', $sf->md5);
    if ($sf->remote_url) {
        row2('Remote URL', $sf->remote_url);
    }
    end_table();

    $sfrs = BoincStickyFileRef::enum(
        "sticky_file_id=$sf_id order by id"
    );
    echo "<h2>References</h2>\n";
    start_table('table-striped');
    table_header('Created', 'Batch', 'Sandbox name', 'Logical name');
    $can_release = true;
    foreach ($sfrs as $sfr) {
        $batch = BoincBatch::lookup_id($sfr->batch_id);
        table_row(
            date_str($sfr->create_time),
            sprintf(
                '<a href=submit.php?action=query_batch&batch_id=%d>%d</a> (%s)',
                $sfr->batch_id,
                $sfr->batch_id,
                batch_state_string($batch->state)
            ),
            $sfr->sandbox_name,
            $sfr->logical_name
        );
        if ($batch->state != BATCH_STATE_RETIRED) {
            $can_release = false;
        }
    }
    end_table();
    if ($can_release) {
        show_button(
            sprintf(
                'sticky.php?action=release&sf_id=%d%s',
                $sf_id, url_tokens()
            ),
            'Release sticky file'
        );
    }
    page_tail();
}

function sf_release($sf_id) {
    $sf = BoincStickyFile::lookup_id($sf_id);
    if (!$sf) error_page('no sticky file');
    if ($sf->user_id != get_logged_in_user()->id) error_page('no access');
    $fname = $sf->physical_name;
    BoincStickyFileRef::delete_aux("sticky_file_id=$sf_id");
    $sf->delete();

    // delete file and .md5 from download dir
    //
    $path = download_hier_path($fname);
    unlink($path);
    unlink($path+'.md5');

    main("Sticky file $sf_id released");
}

$action = get_str('action', true);
switch ($action) {
case 'detail':
    $sf_id = get_int('sf_id');
    sf_detail($sf_id);
    break;
case 'release':
    $sf_id = get_int('sf_id');
    check_tokens();
    sf_release($sf_id);
    break;
default:
    main();
    break;
}

?>
