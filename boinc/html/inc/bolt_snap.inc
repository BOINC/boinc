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



////// stuff related to snapshots
//
// There are 2 kinds of snapshots: "compare" and "map".
//
// A "compare snapshot" is a condensed representation of the results
// for a particular select/xset pair.
// Namely, it's an array whose elements contain
//  bolt_user: the user
//  xset_result: the user's first completion of the xset
//  select_finished: the user's last completion of the select before this
//
// A "map snapshot" is:
// - an assoc array "views" mapping unit name to a list of views
// - an assoc array "results" mapping unit name to a list of results
// - an assoc array "xset_results" mapping unit name to a list of xset results
// - an assoc array "questions" mapping unit name to a list of questions
// - an assoc array "users" mapping user ID to user record
//


function compare_snapshot_filename($course_id, $select_name, $xset_name) {
    @mkdir("../bolt_snap");
    $x = urlencode($course_id."_".$select_name."_".$xset_name);
    return "../bolt_snap/compare_snapshot_$x";
}

function write_compare_snapshot($course_id, $select_name, $xset_name, $dur) {
    $now = time();
    $start = $now - $dur*86400;
    $xrs = BoltXsetResult::enum(
        "course_id=$course_id and name='$xset_name' and create_time >= $start"
    );
    $sfs = BoltSelectFinished::enum(
        "course_id=$course_id and name='$select_name' and end_time >= $start"
    );

    // make an array $a, keyed by user ID, of earliest xset result
    //
    $a = array();
    foreach ($xrs as $xr) {
        $uid = $xr->user_id;
        if (!array_key_exists($uid, $a) || $xr->create_time < $a[$uid]->xr->create_time) {
            $x = null;
            $x->xr = $xr;
            $a[$uid] = $x;
        }
    }

    // now scan select finishes, and for each user find last one before xset
    //
    foreach ($sfs as $sf) {
        $uid = $sf->user_id;
        if (!array_key_exists($uid, $a)) {
            echo "no xset result";
            continue;
        }
        $x = $a[$uid];
        $xr = $x->xr;
        if ($sf->end_time > $xr->create_time) {
            //echo "select finish is too late";
            continue;
        }
        if (!isset($x->sf) || $sf->end_time > $x->sf->end_time) {
            $x->sf = $sf;
            $a[$uid] = $x;
        }
    }

    // cull array elements for which we didn't find a select finish.
    // Look up user records for other elements.
    //
    foreach ($a as $uid=>$x) {
        if (!isset($x->sf)) {
            unset($a[$uid]);
        } else {
            $user = BoincUser::lookup_id($uid);
            BoltUser::lookup($user);
            $x->user = $user;
            $a[$uid] = $x;
        }
    }

    $filename = compare_snapshot_filename($course_id, $select_name, $xset_name);
    $f = fopen($filename, "w");

    $s = null;
    $s->recs = $a;
    $s->dur = $dur;
    $s->time = $now;
    fwrite($f, serialize($s));
    fclose($f);
    return $s;
}

function read_compare_snapshot($course_id, $select_name, $xset_name) {
    $filename = compare_snapshot_filename($course_id, $select_name, $xset_name);
    $f = @fopen($filename, "r");
    if (!$f) return null;
    $x = fread($f, filesize($filename));
    fclose($f);
    return unserialize($x);
}


function map_snapshot_filename($course_id) {
    return "../bolt_snap/map_snapshot_".$course_id;
}

function write_map_snapshot($course_id, $dur) {
    $now = time();
    $start = $now - $dur*86400;

    $views = array();
    $results = array();
    $xset_results = array();
    $users = array();
    $questions = array();

    $vs = BoltView::enum("course_id=$course_id and start_time>$start");
    foreach ($vs as $v) {
        if (array_key_exists($v->item_name, $views)) {
            $x = $views[$v->item_name];
            $x[] = $v;
            $views[$v->item_name] = $x;
        } else {
            $views[$v->item_name] = array($v);
        }
        if (!array_key_exists($v->user_id, $users)) {
            $user = BoincUser::lookup_id($v->user_id);
            BoltUser::lookup($user);
            $users[$v->user_id] = $user;
        }
    }

    $rs = BoltResult::enum("course_id=$course_id and create_time>$start");
    foreach ($rs as $r) {
        if (array_key_exists($r->item_name, $results)) {
            $x = $results[$r->item_name];
            $x[] = $r;
            $results[$r->item_name] = $x;
        } else {
            $results[$r->item_name] = array($r);
        }
        if (!array_key_exists($r->user_id, $users)) {
            $user = BoincUser::lookup_id($r->user_id);
            BoltUser::lookup($user);
            $users[$r->user_id] = $user;
        }
    }

    $xrs = BoltXsetResult::enum("course_id=$course_id and create_time>$start");
    foreach ($xrs as $xr) {
        if (array_key_exists($xr->name, $xset_results)) {
            $x = $xset_results[$xr->name];
            $x[] = $xr;
            $xset_results[$xr->name] = $x;
        } else {
            $xset_results[$xr->name] = array($xr);
        }
        if (!array_key_exists($xr->user_id, $users)) {
            $user = BoincUser::lookup_id($xr->user_id);
            BoltUser::lookup($user);
            $users[$xr->user_id] = $user;
        }
    }
    $qs = BoltQuestion::enum("course_id=$course_id and create_time>$start");
    foreach ($qs as $q) {
        if (array_key_exists($q->name, $questions)) {
            $x = $questions[$q->name];
            $x[] = $q;
            $questions[$q->name] = $x;
        } else {
            $questions[$q->name] = array($q);
        }
        if (!array_key_exists($q->user_id, $users)) {
            $user = BoincUser::lookup_id($q->user_id);
            BoltUser::lookup($user);
            $users[$q->user_id] = $user;
        }
    }

    $y = null;
    $y->views = $views;
    $y->results = $results;
    $y->xset_results = $xset_results;
    $y->users = $users;
    $y->questions = $questions;
    $y->dur = $dur;
    $y->time = $now;

    $filename = map_snapshot_filename($course_id);
    $f = fopen($filename, "w");
    fwrite($f, serialize($y));
    fclose($f);

    return $y;
}

function read_map_snapshot($course_id) {
    $filename = map_snapshot_filename($course_id);
    $f = @fopen($filename, "r");
    if (!$f) return null;
    $x = fread($f, filesize($filename));
    fclose($f);
    return unserialize($x);
}

?>
