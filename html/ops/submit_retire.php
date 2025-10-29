#!/usr/bin/env php

<?php

// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// - Remind job submitters to retire batches.
//      Email a submitter if we haven't emailed them in last week,
//      and they have batches aborted > 14 days ago
//      or batches completed > 30 days ago.
// - Retire batches after a while.
//      Retire a batch if it was completed > 90 days ago
//      or it was aborted > 30 days ago
//      If we retire any, send the user an email
//
// Run this once a week; the interval determines how often
// users get reminders.

include_once('../inc/boinc_db.inc');
include_once('../inc/submit_util.inc');
include_once('../inc/email.inc');
include_once('../inc/common_defs.inc');

define('COMP_NOTIFY', 30);
define('COMP_LIFE', 90);
define('ABORT_NOTIFY', 14);
define('ABORT_LIFE', 30);

function send_retire_email($user, $ncomp, $nabort) {
    $subject = sprintf('%s job submission reminder', PROJECT);
    $text = sprintf(
'This is a reminder from %s.
',
        PROJECT
    );
    if ($ncomp) {
        $text .= sprintf(
'
You currently have %d job batches that were completed more than %d days ago.
These batches will automatically be retired %d days after completion.',
            $ncomp, COMP_NOTIFY, COMP_LIFE
        );
    }
    if ($nabort) {
        $text .= sprintf(
'
You currently have %d job batches that were aborted more than %d days ago.
These batches will automatically be retired %d days after abortion.',
            $nabort, ABORT_NOTIFY, ABORT_LIFE
        );
    }
    $text .= sprintf(
'

When a batch is retired, its output files are deleted.
Please download any needed output files, and retire batches:
%s%s
',
        master_url(),
        'submit.php?action=show_user_batches'
    );
    echo "reminding $user->name\n";
    send_email($user, $subject, $text);
}

function send_retire_emails() {
    $uss = BoincUserSubmit::enum('');
    foreach ($uss as $us) {
        $user = BoincUser::lookup_id($us->user_id);
        if (!$user) continue;
        $batches = BoincBatch::enum("user_id=$user->id");
        // transition in-progress to completed
        $batches = get_batches_params($batches);

        $ncomp = 0;
        $nabort = 0;
        $min_comp = time() - COMP_NOTIFY*86400;
        $min_abort = time() - ABORT_NOTIFY*86400;
        foreach ($batches as $batch) {
            switch ($batch->state) {
            case BATCH_STATE_COMPLETE:
                if ($batch->completion_time < $min_comp) {
                    $ncomp++;
                }
                break;
            case BATCH_STATE_ABORTED:
                if ($batch->completion_time < $min_abort) {
                    $nabort++;
                }
                break;
            }
        }
        if ($ncomp || $nabort) {
            send_retire_email($user, $ncomp, $nabort);
        }
    }
}

function retire_batches() {
    $batches = BoincBatch::enum('');
    $batches = get_batches_params($batches);
    $users = [];
    $min_comp = time() - COMP_LIFE*86400;
    $min_abort = time() - ABORT_LIFE*86400;
    foreach ($batches as $batch) {
        switch ($batch->state) {
        case BATCH_STATE_COMPLETE:
            if ($batch->completion_time < $min_comp) {
                retire_batch($batch);
                echo "retire complete $batch->id user $batch->user_id\n";
                $users[] = $batch->user_id;
            }
            break;
        case BATCH_STATE_ABORTED:
            if ($batch->completion_time < $min_abort) {
                retire_batch($batch);
                echo "retire aborted $batch->id user $batch->user_id\n";
                $users[] = $batch->user_id;
            }
            break;
        }
    }
    $users = array_unique($users);
    $subject = sprintf('%s batches retired', PROJECT);
    foreach ($users as $id) {
        $user = BoincUser::lookup_id($id);
        if (!$user) continue;
        $text = sprintf(
'Greetings from %s.

We recently retired one or more of your job batches.
Completed batches are automatically retired after %d days.
Aborted batches are retired after %d days.

We suggest that you manually retire batches after downloading needed output files.
',
            PROJECT,
            COMP_LIFE, ABORT_LIFE
        );
        send_email($user, $subject, $text);
    }
}

// Retire batches that are complete and have no jobs.
// This shouldn't happen anymore.
//
function retire_empty_batches() {
    $batches = BoincBatch::enum(
        sprintf('state in (%d, %d)', BATCH_STATE_COMPLETE, BATCH_STATE_ABORTED)
    );
    foreach ($batches as $batch) {
        $n = BoincWorkunit::count("batch=$batch->id");
        if ($n == 0) {
            echo "Batch $batch->id has no jobs\n";
            retire_batch($batch);
        }
    }
}

function main() {
    echo "-------- Starting at ".time_str(time())."-------\n";
    retire_batches();
    send_retire_emails();
    echo "-------- Done at ".time_str(time())."-------\n";
}

//retire_empty_batches();
main();

?>
