<?php

function elysia_cron_should_run($conf, $now = -1) {
  if ($conf['disabled']) {
    return false;
  }

  if ($now < 0) {
    $now = time();
  }

  if ((!$conf['last_run']) || ($now - $conf['last_run'] > 365 * 86400)) {
    return true;
  }

  if (!preg_match('/^([0-9*,\/-]+)[ ]+([0-9*,\/-]+)[ ]+([0-9*,\/-]+)[ ]+([0-9*,\/-]+)[ ]+([0-9*,\/-]+)$/', $conf['rule'], $rules)) {
    _dco_watchdog('cron', 'Invalid rule found: %rule', array('%rule' => $conf['rule']));
    return false;
  }

  $weekdayspec = ($rules[5] != '*');
  $mondayspec = ($rules[3] != '*');

  $rules[5] = _cronDecodeRule($rules[5], 0, 6);
  $rules[4] = _cronDecodeRule($rules[4], 1, 12);
  $rules[3] = _cronDecodeRule($rules[3], 1, 31);
  $rules[2] = _cronDecodeRule($rules[2], 0, 23);
  $rules[1] = _cronDecodeRule($rules[1], 0, 59);

  $lastT = _cronT($conf['last_run'] + 30);
  $nowT = _cronT($now);
  $nowTDelta = $nowT - $lastT + ($lastT > $nowT ? 12 * 31 * 24 * 60 : 0);
  $year = date('Y', $conf['last_run']);

  if ($mondayspec || (!$mondayspec && !$weekdayspec)) {
    $first = -1;
    foreach ($rules[4] as $mon) {
      foreach ($rules[3] as $d) {
        if (checkdate($mon, $d, $year)) {
          foreach ($rules[2] as $h) {
            foreach ($rules[1] as $m) {
              $t = _cronT($mon, $d, $h, $m);
              //dprint("* ".$t." L:".$lastT);
              if ($first < 0) {
                $first = $t;
              }
              if ($t > $lastT) {
                $nextT = $t;
                break 4;
              }
            }
          }
        }
      }
    }
    //dprint("R: ".$nextT);
    if (!$nextT) {
      $nextT = $first;
    }

    $nextTDelta = $nextT - $lastT + ($lastT > $nextT ? 12 * 31 * 24 * 60 : 0);

    //dprint($nowT.' '.$nowTDelta.' '.$nextT.' '.$nextTDelta);
    if ($nowTDelta >= $nextTDelta) {
      return true;
    }
  }
  if ($weekdayspec) {
    foreach ($rules[4] as $mon) {
      foreach (_cronMonDaysFromWeekDays($year, $mon, $rules[5]) as $d) {
        foreach ($rules[2] as $h) {
          foreach ($rules[1] as $m) {
            $t = _cronT($mon, $d, $h, $m);
            //dprint("* ".$t." L:".$lastT);
            if ($t > $lastT) {
              $nextT = $t;
              break 4;
            }
          }
        }
      }
    }
    //dprint("R: ".$nextT);
    if (!$nextT) {
      //Must get the first of following year (if day_of_week is specified it's not the same of previous one)
      foreach ($rules[4] as $mon) {
        foreach (_cronMonDaysFromWeekDays($year + 1, $mon, $rules[5]) as $d) {
          foreach ($rules[2] as $h) {
            foreach ($rules[1] as $m) {
              $nextT = _cronT($mon, $d, $h, $m);
              break 4;
            }
          }
        }
      }
    }

    $nextTDelta = $nextT - $lastT + ($lastT > $nextT ? 12 * 31 * 24 * 60 : 0);

    //dprint($nowT.' '.$nowTDelta.' '.$nextT.' '.$nextTDelta);
    if ($nowTDelta >= $nextTDelta) {
      return true;
    }
  }

  return false;
}

// Used by elysia_cron_should_run
function _cronT($time, $d = -1, $h = -1, $m = -1) {
  if ($d < 0) {
    return date('n', $time) * 31 * 24 * 60 + date('j', $time) * 24 * 60 + date('H', $time) * 60 + date('i', $time);
  }
  else {
    return $time * 31 * 24 * 60 + $d * 24 * 60 + $h * 60 + $m;
  }
}

// Used by elysia_cron_should_run
function _cronMonDaysFromWeekDays($year, $mon, $weekdays) {
  $result = array();
  for ($i = 1; checkdate($mon, $i, $year); $i++) {
    $w = date('w', mktime(12, 00, 00, $mon, $i, $year));
    if (in_array($w, $weekdays)) {
      $result[] = $i;
    }
  }
  return $result;
}

// Used by elysia_cron_should_run
function _cronDecodeRule($rule, $min, $max) {
  if ($rule == '*') {
    return range($min, $max);
  }

  $result = array();
  foreach (explode(',', $rule) as $token) {
    if (preg_match('/^([0-9]+)-([0-9]+)$/', $token, $r)) {
      $result = array_merge($result, range($r[1], $r[2]));
    }
    elseif (preg_match('/^\*\/([0-9]+)$/', $token, $r)) {
      for ($i = $min; $i <= $max; $i++) {
        if ($i % $r[1] == 0) {
          $result[] = $i;
        }
      }
    }
    elseif (is_numeric($token)) {
      $result[] = $token;
    }
  }
  return $result;
}

/*******************************************************************************
 * TESTS
 ******************************************************************************/

function test_elysia_cron_should_run() {
  dprint("Start test");
  $start = microtime(true);
  
  //mktime: hr min sec mon day yr
  dprint(" 1." . (false == elysia_cron_should_run(array('rule' => '0 12 * * *', 'last_run' => mktime(12, 0, 0, 1, 2, 2008)), mktime(12, 01, 0, 1, 2, 2008))));
  dprint(" 2." . (false == elysia_cron_should_run(array('rule' => '0 12 * * *', 'last_run' => mktime(12, 0, 0, 1, 2, 2008)), mktime(15, 00, 0, 1, 2, 2008))));
  dprint(" 3." . (false == elysia_cron_should_run(array('rule' => '0 12 * * *', 'last_run' => mktime(12, 0, 0, 1, 2, 2008)), mktime(11, 59, 0, 1, 3, 2008))));
  dprint(" 4." . (true  == elysia_cron_should_run(array('rule' => '0 12 * * *', 'last_run' => mktime(12, 0, 0, 1, 2, 2008)), mktime(12, 00, 0, 1, 3, 2008))));
  dprint(" 5." . (false == elysia_cron_should_run(array('rule' => '59 23 * * *', 'last_run' => mktime(23, 59, 0, 1, 2, 2008)), mktime(0, 00, 0, 1, 3, 2008))));
  dprint(" 6." . (true  == elysia_cron_should_run(array('rule' => '59 23 * * *', 'last_run' => mktime(23, 59, 0, 1, 2, 2008)), mktime(23, 59, 0, 1, 3, 2008))));
  dprint(" 7." . (true  == elysia_cron_should_run(array('rule' => '59 23 * * *', 'last_run' => mktime(23, 59, 0, 1, 2, 2008)), mktime(0, 00, 0, 1, 4, 2008))));
  dprint(" 8." . (true  == elysia_cron_should_run(array('rule' => '59 23 * * *', 'last_run' => mktime(23, 58, 0, 1, 2, 2008)), mktime(23, 59, 0, 1, 2, 2008))));
  dprint(" 9." . (true  == elysia_cron_should_run(array('rule' => '59 23 * * *', 'last_run' => mktime(23, 58, 0, 1, 2, 2008)), mktime(0, 0, 0, 1, 3, 2008))));
  dprint("10." . (false == elysia_cron_should_run(array('rule' => '59 23 * * 0', 'last_run' => mktime(23, 58, 0, 1, 5, 2008)), mktime(23, 59, 0, 1, 5, 2008))));
  dprint("11." . (false == elysia_cron_should_run(array('rule' => '59 23 * * 0', 'last_run' => mktime(23, 58, 0, 1, 5, 2008)), mktime(0, 0, 0, 1, 6, 2008))));
  dprint("12." . (true  == elysia_cron_should_run(array('rule' => '59 23 * * 0', 'last_run' => mktime(23, 58, 0, 1, 5, 2008)), mktime(23, 59, 0, 1, 6, 2008))));
  dprint("13." . (true  == elysia_cron_should_run(array('rule' => '59 23 * * 0', 'last_run' => mktime(23, 58, 0, 1, 5, 2008)), mktime(00, 00, 0, 1, 7, 2008))));
  dprint("14." . (true  == elysia_cron_should_run(array('rule' => '29,59 23 * * 0', 'last_run' => mktime(23, 58, 0, 1, 5, 2008)), mktime(23, 29, 0, 1, 6, 2008))));
  dprint("15." . (true  == elysia_cron_should_run(array('rule' => '29,59 23 * * 0', 'last_run' => mktime(23, 58, 0, 1, 5, 2008)), mktime(23, 59, 0, 1, 6, 2008))));
  dprint("16." . (false == elysia_cron_should_run(array('rule' => '29,59 23 * * 0', 'last_run' => mktime(23, 58, 0, 1, 5, 2008)), mktime(23, 59, 0, 1, 5, 2008))));
  dprint("17." . (true  == elysia_cron_should_run(array('rule' => '29,59 23 * * 0', 'last_run' => mktime(23, 58, 0, 1, 5, 2008)), mktime(23, 58, 0, 1, 6, 2008))));
  dprint("18." . (false == elysia_cron_should_run(array('rule' => '29,59 23 * * 0', 'last_run' => mktime(23, 58, 0, 1, 5, 2008)), mktime(23, 28, 0, 1, 6, 2008))));
  dprint("19." . (false == elysia_cron_should_run(array('rule' => '29,59 23 * * 0', 'last_run' => mktime(23, 28, 0, 1, 5, 2008)), mktime(23, 29, 0, 1, 5, 2008))));
  dprint("20." . (false == elysia_cron_should_run(array('rule' => '29,59 23 * * 0', 'last_run' => mktime(23, 28, 0, 1, 5, 2008)), mktime(23, 30, 0, 1, 5, 2008))));
  dprint("21." . (false == elysia_cron_should_run(array('rule' => '29,59 23 * * 0', 'last_run' => mktime(23, 28, 0, 1, 5, 2008)), mktime(23, 59, 0, 1, 5, 2008))));
  dprint("22." . (true  == elysia_cron_should_run(array('rule' => '29,59 23 * * 0', 'last_run' => mktime(23, 28, 0, 1, 5, 2008)), mktime(23, 29, 0, 1, 6, 2008))));

  dprint("23." . (false == elysia_cron_should_run(array('rule' => '29,59 23 * * 5', 'last_run' => mktime(23, 59, 0, 2, 22, 2008)), mktime(23, 59, 0, 2, 28, 2008))));
  dprint("24." . (true  == elysia_cron_should_run(array('rule' => '29,59 23 * * 5', 'last_run' => mktime(23, 59, 0, 2, 22, 2008)), mktime(23, 59, 0, 2, 29, 2008))));
  dprint("25." . (true  == elysia_cron_should_run(array('rule' => '29,59 23 * * 5', 'last_run' => mktime(23, 59, 0, 2, 22, 2008)), mktime(0, 0, 0, 3, 1, 2008))));

  dprint("26." . (false == elysia_cron_should_run(array('rule' => '59 23 * * 3', 'last_run' => mktime(23, 59, 0, 12, 31, 2008)), mktime(0, 0, 0, 1, 1, 2009))));
  dprint("27." . (false == elysia_cron_should_run(array('rule' => '59 23 * * 3', 'last_run' => mktime(23, 59, 0, 12, 31, 2008)), mktime(0, 0, 0, 1, 7, 2009))));
  dprint("28." . (true  == elysia_cron_should_run(array('rule' => '59 23 * * 3', 'last_run' => mktime(23, 59, 0, 12, 31, 2008)), mktime(23, 59, 0, 1, 7, 2009))));

  dprint("29." . (true  == elysia_cron_should_run(array('rule' => '59 23 * 2 5', 'last_run' => mktime(23, 59, 0, 2, 22, 2008)), mktime(23, 59, 0, 2, 29, 2008))));
  dprint("30." . (true  == elysia_cron_should_run(array('rule' => '59 23 * 2 5', 'last_run' => mktime(23, 59, 0, 2, 22, 2008)), mktime(0, 0, 0, 3, 1, 2008))));
  dprint("31." . (false == elysia_cron_should_run(array('rule' => '59 23 * 2 5', 'last_run' => mktime(23, 59, 0, 2, 29, 2008)), mktime(23, 59, 0, 3, 7, 2008))));
  dprint("32." . (false == elysia_cron_should_run(array('rule' => '59 23 * 2 5', 'last_run' => mktime(23, 59, 0, 2, 29, 2008)), mktime(23, 58, 0, 2, 6, 2009))));
  dprint("33." . (true  == elysia_cron_should_run(array('rule' => '59 23 * 2 5', 'last_run' => mktime(23, 59, 0, 2, 29, 2008)), mktime(23, 59, 0, 2, 6, 2009))));
  dprint("34." . (true  == elysia_cron_should_run(array('rule' => '59 23 *' . '/10 * *', 'last_run' => mktime(23, 58, 0, 1, 10, 2008)), mktime(23, 59, 0, 1, 10, 2008))));
  dprint("35." . (false == elysia_cron_should_run(array('rule' => '59 23 *' . '/10 * *', 'last_run' => mktime(23, 59, 0, 1, 10, 2008)), mktime(23, 59, 0, 1, 11, 2008))));
  dprint("36." . (true  == elysia_cron_should_run(array('rule' => '59 23 *' . '/10 * *', 'last_run' => mktime(23, 59, 0, 1, 10, 2008)), mktime(23, 59, 0, 1, 20, 2008))));
  dprint("37." . (true  == elysia_cron_should_run(array('rule' => '59 23 1-5,10-15 * *', 'last_run' => mktime(23, 59, 0, 1, 4, 2008)), mktime(23, 59, 0, 1, 5, 2008))));
  dprint("38." . (true  == elysia_cron_should_run(array('rule' => '59 23 1-5,10-15 * *', 'last_run' => mktime(23, 59, 0, 1, 4, 2008)), mktime(23, 59, 0, 1, 6, 2008))));
  dprint("39." . (false == elysia_cron_should_run(array('rule' => '59 23 1-5,10-15 * *', 'last_run' => mktime(23, 59, 0, 1, 5, 2008)), mktime(23, 59, 0, 1, 6, 2008))));
  dprint("40." . (false == elysia_cron_should_run(array('rule' => '59 23 1-5,10-15 * *', 'last_run' => mktime(23, 59, 0, 1, 5, 2008)), mktime(23, 58, 0, 1, 10, 2008))));
  dprint("41." . (true  == elysia_cron_should_run(array('rule' => '59 23 1-5,10-15 * *', 'last_run' => mktime(23, 59, 0, 1, 5, 2008)), mktime(23, 59, 0, 1, 10, 2008))));
  dprint("42." . (true  == elysia_cron_should_run(array('rule' => '59 23 1-5,10-15 * *', 'last_run' => mktime(23, 59, 0, 1, 5, 2008)), mktime(23, 59, 0, 1, 16, 2008))));

  dprint("43." . (true  == elysia_cron_should_run(array('rule' => '59 23 1-5 1 0', 'last_run' => mktime(23, 59, 0, 1, 4, 2008)), mktime(23, 59, 0, 1, 5, 2008))));
  dprint("44." . (true  == elysia_cron_should_run(array('rule' => '59 23 1-5 1 0', 'last_run' => mktime(23, 59, 0, 1, 5, 2008)), mktime(23, 59, 0, 1, 6, 2008))));
  dprint("45." . (false == elysia_cron_should_run(array('rule' => '59 23 1-5 1 0', 'last_run' => mktime(23, 59, 0, 1, 6, 2008)), mktime(23, 59, 0, 1, 7, 2008))));
  dprint("46." . (true  == elysia_cron_should_run(array('rule' => '59 23 1-5 1 0', 'last_run' => mktime(23, 59, 0, 1, 6, 2008)), mktime(23, 59, 0, 1, 13, 2008))));
  dprint("47." . (false == elysia_cron_should_run(array('rule' => '59 23 1-5 1 0', 'last_run' => mktime(23, 59, 0, 2, 4, 2008)), mktime(23, 59, 0, 2, 5, 2008))));
  dprint("48." . (false == elysia_cron_should_run(array('rule' => '59 23 1-5 1 0', 'last_run' => mktime(23, 59, 0, 2, 5, 2008)), mktime(23, 59, 0, 2, 10, 2008))));
  dprint("49." . (false == elysia_cron_should_run(array('rule' => '59 23 1-5 1 0', 'last_run' => mktime(23, 59, 0, 2, 10, 2008)), mktime(23, 59, 0, 2, 17, 2008))));

  dprint("49." . (true  == elysia_cron_should_run(array('rule' => '* 0,1,2,3,4,5,6,7,8,18,19,20,21,22,23 * * *', 'last_run' => mktime(8, 58, 0, 2, 10, 2008)), mktime(8, 59, 0, 2, 10, 2008))));
  dprint("50." . (false == elysia_cron_should_run(array('rule' => '* 0,1,2,3,4,5,6,7,8,18,19,20,21,22,23 * * *', 'last_run' => mktime(8, 59, 0, 2, 10, 2008)), mktime(9, 00, 0, 2, 10, 2008))));
  dprint("51." . (false == elysia_cron_should_run(array('rule' => '* 0,1,2,3,4,5,6,7,8,18,19,20,21,22,23 * * *', 'last_run' => mktime(8, 59, 0, 2, 10, 2008)), mktime(17, 59, 0, 2, 10, 2008))));
  dprint("52." . (true  == elysia_cron_should_run(array('rule' => '* 0,1,2,3,4,5,6,7,8,18,19,20,21,22,23 * * *', 'last_run' => mktime(8, 59, 0, 2, 10, 2008)), mktime(18, 00, 0, 2, 10, 2008))));
  dprint("53." . (true  == elysia_cron_should_run(array('rule' => '* 0,1,2,3,4,5,6,7,8,18,19,20,21,22,23 * * *', 'last_run' => mktime(18, 00, 0, 2, 10, 2008)), mktime(18, 01, 0, 2, 10, 2008))));
  dprint("54." . (true  == elysia_cron_should_run(array('rule' => '* 0,1,2,3,4,5,6,7,8,18,19,20,21,22,23 * * *', 'last_run' => mktime(18, 00, 0, 2, 10, 2008)), mktime(19, 0, 0, 2, 10, 2008))));
  dprint("55." . (true  == elysia_cron_should_run(array('rule' => '* 0,1,2,3,4,5,6,7,8,18,19,20,21,22,23 * * *', 'last_run' => mktime(18, 00, 0, 2, 10, 2008)), mktime(9, 0, 0, 3, 10, 2008))));

  dprint("End test (" . (microtime(true) - $start) . ")");
}
// Remove comment to run tests
//test_elysia_cron_should_run();die();
