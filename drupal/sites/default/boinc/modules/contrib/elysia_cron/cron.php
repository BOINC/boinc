<?php

/**
 * @file
 * Handles incoming requests to fire off regularly-scheduled tasks (cron jobs).
 */

if (!file_exists('includes/bootstrap.inc')) {
  if (!empty($_SERVER['DOCUMENT_ROOT']) && file_exists($_SERVER['DOCUMENT_ROOT'] . '/includes/bootstrap.inc')) {
    chdir($_SERVER['DOCUMENT_ROOT']);
  } elseif (preg_match('@^(.*)[\\\\/]sites[\\\\/][^\\\\/]+[\\\\/]modules[\\\\/]([^\\\\/]+[\\\\/])?elysia(_cron)?$@', getcwd(), $r) && file_exists($r[1] . '/includes/bootstrap.inc')) {
    chdir($r[1]);
  } else {
    die("Cron Fatal Error: Can't locate bootstrap.inc. Check cron.php position.");
  }
}

include_once './includes/bootstrap.inc';
$_SERVER['SCRIPT_NAME'] = '/cron.php';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

if (function_exists('elysia_cron_run')) {
  elysia_cron_run();
}
else {
  drupal_cron_run();
}
