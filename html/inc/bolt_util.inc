<?php

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

// a "snapshot" is a condensed representation of the results
// for a particular select/xset pair.
// Namely, it's an array whose elements contain
//  bolt_user: the user
//  xset_result: the user's first completion of the xset
//  select_finished: the user's last completion of the select before this

function write_compare_snapshot($course_id, $select_name, $xset_name, $start) {
    $xrs = BoltXsetResult::enum(
        "course_id=$course_id and name='$xset_name' and create_time >= $start"
    );
    $sfs = BoltSelectFinished::enum(
        "course_id=$course_id and name='$select_name' and end_time >= $start"
    );

    // make an array, keyed by user ID, of earliest xset result
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
        if (!array_key_exists($uid, $a)) continue;
        $x = $a[$uid];
        $xr = $x->xr;
        if ($sf->end_time > $xr->create_time) continue;
        $s = $x->sf;
        if (!is_set($x->sf) || $sf->create_time > $s.create_time) {
            $x->sf = $sf;
            $a[$uid] = $x;
        }
    }
    $filename = "compare_snapshot_$course_id_$select_name_$xset_name.json";
    $f = fopen($filename, "w");
    fwrite($f, json_encode($a));
    fclose($f);
    return $a;
}

function read_compare_snapshot($course_id, $select_name, $xset_name) {
    $filename = "compare_snapshot_".$course_id."_".$select_name."_".$xset_name.".json";
    $f = @fopen($filename, "r");
    if (!$f) return null;
    $x = fread($f, filesize($filename));
    fclose($f);
    return json_decode($x);
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

?>
