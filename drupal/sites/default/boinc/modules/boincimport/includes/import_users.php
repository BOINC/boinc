<?php
// $Id$

/**
 * @file
 * A special, detachable routine for importing user accounts.
 *
 * The process of importing users relies on Drupal API calls that may not
 * be intended for large batch processing. This script is intended to be called
 * via exec() by a higher level import routine to handle a small subset of users
 * at a time and avoid exhausting memory.
 */


  require_once('./includes/bootstrap.inc');
  drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  require_boinc('db');

  // Parse arguments
  $import_lurkers = isset($argv[1]) ? $argv[1] : false;
  $record_offset = isset($argv[2]) ? $argv[2] : 0;
  $chunk_size = isset($argv[3]) ? $argv[3] : 100;

  // Construct sql conditions
  $limit = sprintf('LIMIT %d,%d', $record_offset, $chunk_size);

  $count = 0;

  db_set_active('boinc_rw');
  if ($import_lurkers) {
    $boinc_accounts = db_query('SELECT id FROM user ORDER BY id %s', $limit);
  }
  //else $boinc_accounts = db_query('SELECT DISTINCT user AS boinc_id FROM post ORDER BY boinc_id %s', $limit);
  else {
    // Need to import any user who is currently ignored in order to keep them
    // ignored... not particularly clean (ignored users are stored in a string)
    $ignored_user_list = array();
    $ignoring_users = db_query("
      SELECT ignorelist
      FROM forum_preferences
      WHERE ignorelist <> ''
      ORDER BY userid ASC"
    );
    while ($ignoring_user = db_fetch_object($ignoring_users)) {
      $ignored_user_list = $ignored_user_list + array_fill_keys(explode('|', trim($ignoring_user->ignorelist, '|')), 1);
    }
    $ignored_user_list = array_keys($ignored_user_list);
    $boinc_accounts = db_query("
      SELECT id FROM
      (
        (SELECT id FROM {user} WHERE teamid > 0 OR id IN(%s)) UNION
        (SELECT DISTINCT user FROM {post}) UNION
        (SELECT DISTINCT userid FROM {subscriptions}) UNION
        (SELECT DISTINCT user_src FROM {friend} WHERE reciprocated = 1) UNION
        (SELECT DISTINCT user_dest FROM {friend} WHERE reciprocated = 1) UNION
        (SELECT DISTINCT userid FROM {forum_preferences} WHERE ignorelist <> '') UNION
        (SELECT DISTINCT userid FROM {private_messages}) UNION
        (SELECT DISTINCT senderid FROM {private_messages})
      ) AS usersToImport ORDER BY id ASC %s", implode(',', $ignored_user_list), $limit
    );
  }
  db_set_active('default');

  while ($boinc_account = db_fetch_object($boinc_accounts)) {
    // Grab the BOINC user object and create a Drupal user from it
    if (boincuser_register_make_drupal_user($boinc_account->id)) {
      $count++;
    }
  }

  echo $count;
