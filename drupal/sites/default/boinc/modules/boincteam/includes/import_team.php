<?php
// $Id$

/**
 * @file
 * A special, detachable routine for importing teams.
 *
 * The process of importing teams relies on Drupal API calls that may not
 * be intended for large batch processing. This script is intended to be called
 * via exec() by a higher level import routine to handle a small subset of teams
 * at a time and avoid exhausting memory.
 */

  require_once('./includes/bootstrap.inc');
  drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  require_boinc('db');
  
  // Parse arguments
  $team_id = isset($argv[1]) ? $argv[1] : null;
  $team_type_map_id = isset($argv[2]) ? $argv[2] : null;
  $input_format = isset($argv[3]) ? $argv[3] : null;
  //$record_offset = isset($argv[1]) ? $argv[1] : 0;
  //$chunk_size = isset($argv[2]) ? $argv[2] : 100;
  
  // Construct sql conditions
  //$limit = sprintf('LIMIT %d,%d', $record_offset, $chunk_size);
  
  $count = 0;
  
  // Get teams from BOINC
  db_set_active('boinc');
  $boincteam = db_fetch_object(db_query('SELECT * FROM team WHERE id=%d', array($team_id)));
  $boincteam_members = db_query('SELECT id FROM user WHERE teamid=%d', array($team_id));
  $boincteam_admin = (int) db_result(db_query('SELECT userid FROM team_admin WHERE teamid=%d', array($team_id)));
  db_set_active('default');
  
  $boincteam->description = _boincforum_text_sanitise($boincteam->description);
  $teaser = node_teaser($boincteam->description);
  
  // Construct the team as an organic group node
  $node = array(
    'type' => 'team',
    'title' => $boincteam->name,
    'body' => $boincteam->description,
    'teaser' => $teaser,
    'uid' => get_drupal_id($boincteam->userid),
    'path' => null,
    'status' => 1,  // published or not - always publish
    'promote' => 0,
    'created' => $boincteam->create_time,
    'comment' => 0,  // comments disabled
    'moderate' => 0,
    'sticky' => 0,
    'format' => $input_format
  );
  
  // Use pathauto function, if available, to clean up the path
  if (module_exists('pathauto')) {
    module_load_include('inc', 'pathauto', 'pathauto');
    $node['path'] = pathauto_cleanstring($boincteam->name);
  }
  else {
    echo 'fail'; exit;
    //$node['path'] = check_plain($boincteam->name);
  }
  
  // Add special organic group properties
  //$node->og_initial_groups[0][$node->nid];
  //$node->og_groups[0] = $node->nid;
  $node['og_public'] = 0;
  //$node->og_groups_names[0] = $node->title;
  
  // Save the team node
  $node = (object) $node; // node_save requires an object form
  node_save($node);
  
  // Save the team IDs to a BOINC <--> Drupal reference table.
  db_query('INSERT INTO {boincteam} (team_id, nid) VALUES (%d, %d)', $boincteam->id, $node->nid);
  
  // Determine team membership
  $boincteam_member_ids = array();
  while ($boincuser = db_fetch_object($boincteam_members)) $boincteam_member_ids[] = $boincuser->id;
  $team_members = db_query('SELECT uid FROM {boincuser} WHERE boinc_id IN(%s)', implode(',', $boincteam_member_ids));
  $team_admin = (int) db_result(db_query('SELECT uid FROM {boincuser} WHERE boinc_id=%d', $boincteam_admin));
  
  // Assign team membership "subscriptions"
  while ($drupal_user = db_fetch_object($team_members)) {
    og_save_subscription($node->nid, $drupal_user->uid, array('is_active' => 1, 'is_admin' => (($drupal_user->uid == $team_admin) ? 1 : 0)));
    $count++;
  }
  
  echo $count;