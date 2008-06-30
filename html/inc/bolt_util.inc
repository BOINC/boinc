<?php

////// functions that traverse a unit tree

// get names of units of a given type

function units_of_type($unit, $type) {
    $names = array();
    if (get_class($unit) == $type) {
        $names[] = $unit->name;
    }
    if (is_subclass_of($unit, "BoltSet")) {
        foreach ($unit->units as $u) {
            $n = units_of_type($u, $type);
            $names = array_merge($names, $n);
        }
    }
    return array_unique($names);
}

// show a menu of select units
//
function choose_select($top_unit) {
    echo "<select name=select_name>
        <option selected> ---
    ";
    $names = units_of_type($top_unit, "BoltSelect");
    foreach ($names as $n) {
        echo "<option> $n";
    }
    echo "</select>";
}

// show a menu of exercise sets
//
function choose_xset($top_unit) {
    echo "<select name=xset_name>
        <option selected> ---
    ";
    $names = units_of_type($top_unit, "BoltExerciseSet");
    foreach ($names as $n) {
        echo "<option> $n";
    }
    echo "</select>";
}

// Find a unit of given name
//
function lookup_unit($top_unit, $name) {
    if ($top_unit->name == $name) return $top_unit;
    if (is_subclass_of($top_unit, "BoltSet")) {
        foreach ($top_unit->units as $child) {
            $u = lookup_unit($child, $name);
            if ($u) return $u;
        }
    }
    return null;
}

////// bits of HTML for getting user info

function info_incomplete($user) {
    if (!$user->bolt->birth_year) return true;
    if (!$user->bolt->sex) return true;
    return false;
}

function birth_year_select($user) {
    $this_year = date("Y");
    $x = "<select name=birth_year>\n";
    for ($i=$this_year-100; $i<$this_year; $i++) {
        $s = ($i == $user->bolt->birth_year)?"selected":"";
        $x .= "<option value=$i $s>$i</option>\n";
    }
    $s = (!$user->bolt->birth_year)?"selected":"";
        $x .= "<option value=$0 $s>Unspecified</option>\n";
    $x .= "</select>\n";
    return $x;
}

function sex_select($user) {
    $x = "<select name=sex>\n";
    $s = ($user->bolt->sex == 0)?"selected":"";
    $x .= "<option value=0 $s>Unspecified</option>\n";
    $s = ($user->bolt->sex == 1)?"selected":"";
    $x .= "<option value=1 $s>Male</option>\n";
    $s = ($user->bolt->sex == 2)?"selected":"";
    $x .= "<option value=2 $s>Female</option>\n";
    $x .= "</select>\n";
    return $x;
}

function request_info($user, $course) {
    page_head("About you");
    echo "
        You may optionally tell us some facts about yourself.
        This information will help us improve this course,
        and will be kept private.
        <p>
        <form action=bolt_sched.php>
        <input type=hidden name=action value=update_info>
        <input type=hidden name=course_id value=$course->id>
    ";
    start_table();
    row2("Birth year", birth_year_select($user));
    row2("Sex", sex_select($user));
    row2("", "<input type=submit value=OK>");
    end_table();
    echo "</form>\n";
    page_tail();
}

////// stuff related to snapshots

function compare_snapshot_filename($course_id, $select_name, $xset_name) {
    return "../bolt_snap/compare_snapshot_".$course_id."_".$select_name."_".$xset_name;
}

// a "snapshot" is a condensed representation of the results
// for a particular select/xset pair.
// Namely, it's an array whose elements contain
//  bolt_user: the user
//  xset_result: the user's first completion of the xset
//  select_finished: the user's last completion of the select before this

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
            echo "select finish is too late";
            continue;
        }
        if (!isset($x->sf) || $sf->create_time > $s.create_time) {
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

// a "map snapshot" is:
// - an assoc array "views" mapping unit name to a list of views
// - an assoc array "results" mapping unit name to a list of results
// - an assoc array "xset_results" mapping unit name to a list of xset results
// - an assoc array "users" mapping user ID to user record
//
function write_map_snapshot($course_id, $dur) {
    $now = time();
    $start = $now - $dur*86400;

    $views = array();
    $results = array();
    $xset_results = array();
    $users = array();

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
            $users[$v->user_id] = $user;
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
            $users[$v->user_id] = $user;
        }
    }

    $y = null;
    $y->views = $views;
    $y->results = $results;
    $y->xset_results = $xset_results;
    $y->users = $users;
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

////// Statistics

// compute the mean and stdev of an array
//
function mean_stdev($array, &$mean, &$stdev) {
    $n = 0;
    $m = 0;
    $m2 = 0;

    foreach ($array as $x) {
        $n++;
        $delta = $x - $m;
        $m += $delta/$n;
        $m2 += $delta*($x-$m);
    }
    $mean = $m;
    $stdev = sqrt($m2/($n-1));
}

// approximate the 90% confidence interval for the mean of an array
//
function conf_int_90($array, &$lo, &$hi) {
    $n = count($array);
    mean_stdev($array, $mean, $stdev);

    // I'm too lazy to compute the t distribution
    $t_90 = 1.7;
    $d = $t_90 * $stdev / sqrt($n);
    $lo = $mean - $d;
    $hi = $mean + $d;
}

function test_stats() {
    $a = array(1,1,1,1,0,1,1,1,3, 1, 1, 1, 1);
    mean_stdev($a, $mean, $stdev);
    echo "mean: $mean stdev: $stdev\n";
    conf_int_90($a, $lo, $hi);
    echo "lo $lo hi $hi\n";
}

//////////// graph drawing

function compare_bar($title, $n, $width, $lo, $hi) {
    $x1 = $width*$lo;
    $x2 = $width*($hi-$lo);
    $a1 = number_format($lo*100);
    $a2 = number_format($hi*100);
    return "
        <tr>
        <td>$title<br><span class=note>($n students)</span></td>
        <td>
            <table class=bolt_bar width=$width><tr class=bolt_bar>
            <td class=bolt_bar1 width=$x1 bgcolor=lightgray align=right>$a1</td>
            <td class=bolt_bar2 width=$x2 bgcolor=black></td>
            <td class=bolt_bar1 bgcolor=lightgray>$a2</td>
            </tr></table>
        </td>
        </tr>
    ";
}

function compare_bar_insuff($title, $width) {
    return "
        <tr>
        <td>$title</td>
        <td>
            <table class=bolt_bar width=$width><tr class=bolt_bar>
            <td class=bolt_bar1 bgcolor=lightgray>Insufficient data</td>
            </tr></table>
        </td>
        </tr>
    ";
}

function outcome_graph($x) {
    return "<td>outcome graph</td>";
}

function time_graph($t) {
    return "<td>time graph</td>";
}

function score_graph($t) {
    return "<td>score graph</td>";
}

function empty_cell() {
    return "<td><br></td>";
}

?>
