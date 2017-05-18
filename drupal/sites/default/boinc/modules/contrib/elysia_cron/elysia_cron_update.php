<?php

/*******************************************************************************
 * ELYSIA CRON VERSION UPDATE
 ******************************************************************************/

function elysia_cron_check_version_update() {
  $ver = variable_get('elysia_cron_version', 0);
  if ($ver < 20111012) {
    $ver = _ec_variable_get('elysia_cron_version', 0);
  }

  if (!$ver || $ver < 20090218) {

    $unchanged = array(
      'elysia_cron_last_context',
      'elysia_cron_last_run',
      'elysia_cron_disabled',
      'elysia_cron_semaphore',
      'elysia_cron_key',
      'elysia_cron_allowed_hosts',
      'elysia_cron_default_rule',
      'elysia_cron_script',
      'elysia_cron_runtime_replacement',
      'elysia_cron_version',
    );

    $rs = db_query('select * from {variable} where name like "elysia_cron_%%"');
    while ($v = db_fetch_object($rs)) {
      if (!in_array($v->name, $unchanged)) {
        $vn = false;
        if (preg_match('/^elysia_cron_ctx_(.*)_(running|disabled|last_run|last_aborted|abort_count|execution_count|last_execution_time|avg_execution_time|max_execution_time|last_shutdown_time|last_abort_function)/', $v->name, $r)) {
          switch ($r[2]) {
            case 'running':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_r';
              break;
            case 'disabled':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_d';
              break;
            case 'last_run':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_lr';
              break;
            case 'last_aborted':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_la';
              break;
            case 'abort_count':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_ac';
              break;
            case 'execution_count':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_ec';
              break;
            case 'last_execution_time':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_let';
              break;
            case 'avg_execution_time':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_aet';
              break;
            case 'max_execution_time':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_met';
              break;
            case 'last_shutdown_time':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_lst';
              break;
            case 'last_abort_function':
              $vn = 'ecc_' . _ec_get_name($r[1]) . '_laf';
              break;
          }
        }
        elseif (preg_match('/^elysia_cron_(.*)_(rule|disabled|context|running|last_run|last_execution_time|execution_count|avg_execution_time|max_execution_time)/', $v->name, $r)) {
          switch ($r[2]) {
            case 'rule':
              $vn = 'ec_' . _ec_get_name($r[1]) . '_rul';
              break;
            case 'disabled':
              $vn = 'ec_' . _ec_get_name($r[1]) . '_d';
              break;
            case 'context':
              $vn = 'ec_' . _ec_get_name($r[1]) . '_c';
              break;
            case 'running':
              $vn = 'ec_' . _ec_get_name($r[1]) . '_r';
              break;
            case 'last_run':
              $vn = 'ec_' . _ec_get_name($r[1]) . '_lr';
              break;
            case 'last_execution_time':
              $vn = 'ec_' . _ec_get_name($r[1]) . '_let';
              break;
            case 'execution_count':
              $vn = 'ec_' . _ec_get_name($r[1]) . '_ec';
              break;
            case 'avg_execution_time':
              $vn = 'ec_' . _ec_get_name($r[1]) . '_aet';
              break;
            case 'max_execution_time':
              $vn = 'ec_' . _ec_get_name($r[1]) . '_met';
              break;
          }
        }
        if ($vn) {
          variable_set($vn, unserialize($v->value));
        }
        else {
          _dco_watchdog('cron', 'Error in update, cant convert %name (value: %value)', array('%name' => $v->name, '%value' => $v->value), WATCHDOG_ERROR);
        }

        variable_del($v->name);
      }
    }

    variable_set('elysia_cron_version', 20090218);
  }
  if ($ver < 20090920) {
    variable_set('elysia_cron_version', 20090920);

  }
  if ($ver < 20100507) {
    if (EC_DRUPAL_VERSION >= 6) {
      // D6
      drupal_install_schema('elysia_cron');

      // In ver 20111020 disabled has been renamed to disable, revert it now
      if (EC_DRUPAL_VERSION >= 7) {
        // D7
        // Must use "$v" for PHP5.3 running D6 version (detect the error even if it doesn't pass here)
        db_change_field($v = 'elysia_cron', 'disable', 'disabled', array('type' => 'int', 'size' => 'tiny', 'not null' => FALSE));

      }
      elseif (EC_DRUPAL_VERSION >= 6) {
        // D6
        $ret = array();
        db_change_field($ret, 'elysia_cron', 'disable', 'disabled', array('type' => 'int', 'size' => 'tiny', 'not null' => FALSE));
      }
    }
    else {
      // D5
      switch ($GLOBALS['db_type']) {
        case 'mysqli':
        case 'mysql':
          db_query("create table if not exists {elysia_cron} (
            name varchar(120) not null,
            disabled tinyint(1) not null default '0',
            rule varchar(32),
            weight int(11) not null default '0',
            context varchar(32),
            running int(11) not null default '0',
            last_run int(11) not null default '0',
            last_aborted tinyint(1) not null default '0',
            abort_count int(11) not null default '0',
            last_abort_function varchar(32),
            last_execution_time int(11) not null default '0',
            execution_count int(11) not null default '0',
            avg_execution_time float(5,2) not null default '0',
            max_execution_time int(11) not null default '0',
            last_shutdown_time int(11) not null default '0',
            primary key (name)
          )");
          break;
        case 'pgsql':
          db_query("create table {elysia_cron} (
            name varchar(120) not null,
            disabled smallint not null default '0',
            rule varchar(32),
            weight integer not null default '0',
            context varchar(32),
            running int not null default '0',
            last_run integer not null default '0',
            last_aborted smallint not null default '0',
            abort_count integer not null default '0',
            last_abort_function varchar(32),
            last_execution_time integer not null default '0',
            execution_count integer not null default '0',
            avg_execution_time float not null default '0',
            max_execution_time integer not null default '0',
            last_shutdown_time integer not null default '0',
            primary key (name)
          )");
      }
    }

    $rs = db_query('select * from {variable} where name like "ec_%%" or name like "ecc_%%"');
    $data = array();
    $todelete = array();
    while ($v = db_fetch_object($rs)) {
      $name = false;
      if (preg_match('/^ecc_(.+)_(r|d|lr|la|ac|ec|let|aet|met|lst|laf)/', $v->name, $r)) {
        $name = ':' . $r[1];
      }
      elseif (preg_match('/^ec_(.+)_(rul|d|c|w|r|lr|let|ec|aet|met)/', $v->name, $r)) {
        $name = $r[1];
      }
      if ($name) {
        if (!isset($data[$name])) {
          $data[$name] = array('name' => $name);
        }
        switch ($r[2]) {
          case 'r':
            $f = 'running';
            break;
          case 'd':
            $f = 'disabled';
            break;
          case 'rul':
            $f = 'rule';
            break;
          case 'w':
            $f = 'weight';
            break;
          case 'c':
            $f = 'context';
            break;
          case 'lr':
            $f = 'last_run';
            break;
          case 'la':
            $f = 'last_aborted';
            break;
          case 'ac':
            $f = 'abort_count';
            break;
          case 'laf':
            $f = 'last_abort_function';
            break;
          case 'let':
            $f = 'last_execution_time';
            break;
          case 'ec':
            $f = 'execution_count';
            break;
          case 'aet':
            $f = 'avg_execution_time';
            break;
          case 'met':
            $f = 'max_execution_time';
            break;
          case 'lst':
            $f = 'last_shutdown_time';
            break;
        }
        $data[$name][$f] = unserialize($v->value);
        $todelete[] = $v->name;
      }
    }

    $ifields = array('disabled', 'weight', 'running', 'last_run', 'last_aborted', 'abort_count', 'last_execution_time', 'execution_count', 'avg_execution_time', 'max_execution_time', 'last_shutdown_time');
    foreach ($data as $v) {
      foreach ($ifields as $f) {
        if (empty($v[$f])) {
          $v[$f] = 0;
        }
      }
      db_query("insert into {elysia_cron} (name, disabled, rule, weight, context, running, last_run, last_aborted, abort_count, last_abort_function, last_execution_time, execution_count, avg_execution_time, max_execution_time, last_shutdown_time)
        values ('%s', %d, '%s', %d, '%s', %d, %d, %d, %d, '%s', %d, %d, %f, %d, %d)",
        $v['name'], $v['disabled'], $v['rule'], $v['weight'], $v['context'], $v['running'], $v['last_run'], $v['last_aborted'], $v['abort_count'], $v['last_abort_function'], $v['last_execution_time'], $v['execution_count'], $v['avg_execution_time'], $v['max_execution_time'], $v['last_shutdown_time']
      );
    }

    db_query("update {elysia_cron} set context = null where context = ''");
    db_query("update {elysia_cron} set rule = null where rule = ''");

    foreach ($todelete as $v) {
      variable_del($v);
      db_query("DELETE FROM {variable} WHERE name = '%s'", $v);
    }

    variable_set('elysia_cron_version', 20100507);

    unset($GLOBALS['_ec_variables']);
  }
  // D7 VERSION FROM NOW ON...

  if ($ver < 20110323) {
    if (EC_DRUPAL_VERSION >= 7) {
      // D7
      // Must use "$v" for PHP5.3 running D6 version (detect the error even if it doesn't pass here)
      db_change_field($v = 'elysia_cron', 'weight', 'weight', array('type' => 'int', 'not null' => FALSE));

    }
    elseif (EC_DRUPAL_VERSION >= 6) {
      // D6
      $ret = array();
      db_change_field($ret, 'elysia_cron', 'weight', 'weight', array('type' => 'int', 'not null' => FALSE));
    }
    else {
      // D5
      db_query("alter table {elysia_cron} change weight weight int(11)");
    }

    variable_set('elysia_cron_version', 20110323);
  }
  
  if ($ver < 20111007) {
    $default_rules = variable_get('elysia_cron_default_rules', $GLOBALS['elysia_cron_default_rules']);
    if (!empty($default_rules['*/6 * * * *']) && $default_rules['*/6 * * * *'] == 'Every 6 hours') {
      unset($default_rules['*/6 * * * *']);
      $default_rules['0 */6 * * *'] = 'Every 6 hours';
      variable_set('elysia_cron_default_rules', $default_rules);
    }
    variable_set('elysia_cron_version', 20111007);
  }

  if ($ver < 20111012) {
    // I only need to rebuild variable cache, so i just set the new version
    variable_set('elysia_cron_version', 20111012);
  }

  if ($ver < 20111020) {
    if (EC_DRUPAL_VERSION >= 7) {
      // D7
      // Must use "$v" for PHP5.3 running D6 version (detect the error even if it doesn't pass here)
      db_change_field($v = 'elysia_cron', 'disabled', 'disable', array('type' => 'int', 'size' => 'tiny', 'not null' => FALSE));

    }
    elseif (EC_DRUPAL_VERSION >= 6) {
      // D6
      $ret = array();
      db_change_field($ret, 'elysia_cron', 'disabled', 'disable', array('type' => 'int', 'size' => 'tiny', 'not null' => FALSE));
    }
    else {
      // D5
      db_query("alter table {elysia_cron} change disabled disable tinyint(1)");
    }
    db_query("update {elysia_cron} set disable = NULL where disable = 0");

    variable_set('elysia_cron_version', 20111020);
  }
}
