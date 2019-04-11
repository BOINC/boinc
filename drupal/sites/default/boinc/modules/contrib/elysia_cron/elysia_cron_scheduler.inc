<?php

function elysia_cron_should_run($conf, $now = -1) {  $prev_rule_run = 0; // A che ora SAREBBE dovuto essere lanciato l'ultima volta
  if ($conf['disabled']) {
    return false;
  }
  if ($now < 0) {
    $now = time();
  }
  if ((!$conf['last_run']) || ($now - $conf['last_run'] > 365 * 86400)) {
    return true;
  }

  $next_run = _elysia_cron_next_run($conf);
  return $now >= $next_run;
}

function _elysia_cron_next_run($conf) {
  $ranges = array(
    array(0, 59),
    array(0, 23),
    array(1, 31), // TODO
    array(1, 12),
    array(0, 3000),
    array(0, 6),
  );

  if (!preg_match('/^([0-9*,\/-]+)[ ]+([0-9*,\/-]+)[ ]+([0-9*,\/-]+)[ ]+([0-9*,\/-]+)[ ]+([0-9*,\/-]+)$/', $conf['rule'], $rules)) {
    _dco_watchdog('cron', 'Invalid rule found: %rule', array('%rule' => $conf['rule']));
    return false;
  }

  $rule = array($rules[1], $rules[2], array($rules[3], $rules[5]), $rules[4]);
  $ruledec = array();
  $date = _cronDecodeDate($conf['last_run'], 1);
  $expected_date = _cronDecodeDate(!empty($conf['last_run_expected']) ? $conf['last_run_expected'] : 0);
  
  for ($i = 0; $i < 4; $i++) {
    if ($i != 2) {
      // Standard scheme for mins, hours, month
      $ruledec[$i] = _cronDecodeRule($rule[$i], $ranges[$i][0], $ranges[$i][1]);
    } else {
      // For mday+week we follow another scheme
      $ruledec[$i] = _cronDecodeRuleMday($rule[2], $date[3], $date[4]);
    }
    $r = $ruledec[$i];
    $new = $date[$i];
    if ($r['d']) {
      if ($expected_date[$i] > $date[$i]) {
        $expected_date[$i] -= $ranges[$i][1] + 1;
      }
      $new = $expected_date[$i] + ceil(($date[$i] - $expected_date[$i]) / $r['d']) * $r['d'];
    }
    elseif ($r['n']) {
      $new = _cronNextOrEqual($date[$i], $r['n'], $ranges[$i][0], $ranges[$i][1]);
    }
    if ($new != $date[$i]) {
      $date[$i] = $new;
      if ($date[$i] > $ranges[$i][1]) {
        $date[$i + 1]++;
        $date[$i] = $ranges[$i][0] + $date[$i] - $ranges[$i][1] - 1;
      }
      for ($j = 0; $j < $i; $j++) {
        if ($j == 2) {
          // For mday+week decoded rule could be changed (by month+year)
          $ruledec[$j] = _cronDecodeRuleMday($rule[2], $date[3], $date[4]);
        }
        $date[$j] = $ruledec[$j]['d'] ? ($ranges[$j][0] == 0 ? 0 : $ruledec[$j]['d']) : ($ruledec[$j]['n'] ? reset($ruledec[$j]['n']) : $ranges[$j][0]);
        $expected_date[$j] = 0;
      }
    }
  }

  return _cronEncodeDate($date);
}

function _cronDecodeDate($timestamp, $min_diff = 0) {
  $time = floor($timestamp / 60);
  $time += $min_diff;
  
  $date = $time ? getdate($time * 60) : 0;
  $date = array(
    $time ? $date['minutes'] : 0,
    $time ? $date['hours'] : 0,
    $time ? $date['mday'] : 0,
    $time ? $date['mon'] : 0,
    $time ? $date['year'] : 0,
  );
  return $date;
}
function _cronEncodeDate($date) {
  return mktime($date[1], $date[0], 0, $date[3], $date[2], $date[4]);
}

function _cronNextOrEqual($el, $arr, $range_start, $range_end) {
  if (empty($arr)) {
    return $el;
  }
  foreach ($arr as $x) {
    if ($x >= $el) {
      return $x;
    }
  }
  return $range_end + reset($arr) + 1 - $range_start;
}

function _cronDecodeRule($rule, $min, $max) {
  if ($rule == '*') {
    return array('n' => array(), 'd' => 0);
  }

  $result = array('n' => array(), 'd' => 0);
  foreach (explode(',', $rule) as $token) {
    if (preg_match('/^([0-9]+)-([0-9]+)$/', $token, $r)) {
      $result['n'] = array_merge($result['n'], range($r[1], $r[2]));
    }
    elseif (preg_match('/^\*\/([0-9]+)$/', $token, $r)) {
      $result['d'] = $r[1];
    }
    elseif (is_numeric($token)) {
      $result['n'][] = $token;
    }
  }
  sort($result['n']);
  return $result;
}

function _cronDecodeRuleMday($rule, $month, $year) {
  $range_from = 1;
  $range_to = $month != 2 ? (in_array($month, array(4,6,9,11)) ? 30 : 31) : ($year % 4 == 0 ? 29 : 28);
  $r1 = _cronDecodeRule($rule[0], $range_from, $range_to);
  $r2 = _cronDecodeRule($rule[1], $range_from, $range_to);
  if ($r2['d']) {
    for ($i = 0; $i < 7; $i++) {
      if ($i % $r2['d'] == 0) {
        $r2['n'][] = $i;
      }
    }
  }
  if ($r2['n']) {
    $r2['n'] = array_unique($r2['n']);
    $r1['n'] = array_merge($r1['n'], _cronMonDaysFromWeekDays($year, $month, $r2['n']), _cronMonDaysFromWeekDays($year, $month + 1, $r2['n'], $range_to));
  }
  return $r1;
}

// Used by elysia_cron_should_run
function _cronMonDaysFromWeekDays($year, $mon, $weekdays, $offset = 0) {
  if ($mon > 12) {
    $year ++;
    $mon = $mon - 12;
  }
  
  $result = array();
  for ($i = 1; checkdate($mon, $i, $year); $i++) {
    $w = date('w', mktime(12, 00, 00, $mon, $i, $year));
    if (in_array($w, $weekdays)) {
      $result[] = $i + $offset;
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
