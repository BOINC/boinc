<?php
// $Id$

/**
 * @file
 * A special, detachable routine for importing subscriptions.
 *
 * The process of importing subscriptions relies on Drupal API calls that may
 * not be intended for large batch processing. This script is intended to be
 * called via exec() by a higher level import routine to handle a small subset
 * of Drupal user objects at a time and avoid exhausting memory.
 */


  require_once('./includes/bootstrap.inc');
  drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  require_boinc('db');

  // Parse arguments
  $record_offset = isset($argv[1]) ? $argv[1] : 0;
  $chunk_size = isset($argv[2]) ? $argv[2] : 100;

  // Construct sql conditions
  $limit = sprintf('LIMIT %d,%d', $record_offset, $chunk_size);

  $total_count = 0;

  // Get the users with subscriptions to import
  db_set_active('boinc_rw');
  $subscribed_boinc_users = db_query("
    SELECT DISTINCT userid FROM {subscriptions}
    ORDER BY userid ASC %s", $limit
  );
  db_set_active('default');

  // Import subscriptions
  while ($boinc_subscription = db_fetch_object($subscribed_boinc_users)) {
    $uid = get_drupal_id($boinc_subscription->userid);
    $count = boincuser_pull_subscriptions($uid);
    $total_count += $count;
    echo "\nuser: {$uid}; boinc_id: {$boinc_subscription->userid}; {$count} subscriptions";
  }
  echo "\n";
  echo $total_count;
