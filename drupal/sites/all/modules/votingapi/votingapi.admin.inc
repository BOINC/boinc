<?php
// $Id: votingapi.admin.inc,v 1.1.2.3 2009/05/26 03:50:00 eaton Exp $

/**
 * @file
 * Configuration forms and helper functions for VotingAPI module.
 */

/**
 * Administrative settings for VotingAPI.
 */
function votingapi_settings_form($form_state) {
  $period = array(0 => t('Immediately')) + drupal_map_assoc(array(300, 900, 1800, 3600, 10800, 21600, 32400, 43200, 86400, 172800, 345600, 604800), 'format_interval') + array(-1 => t('Never'));

  $form['votingapi_anonymous_window'] = array(
    '#type' => 'select',
    '#title' => t('Anonymous vote rollover'),
    '#description' => t('The amount of time that must pass before two anonymous votes from the same computer are considered unique. Setting this to \'never\' will eliminate most double-voting, but will make it impossible for multiple anonymous on the same computer (like internet cafe customers) from casting votes.'),
    '#default_value' => variable_get('votingapi_anonymous_window', 86400),
    '#options' => $period
  );

  $form['votingapi_calculation_schedule'] = array(
    '#type' => 'radios',
    '#title' => t('Vote tallying'),
    '#description' => t('On high-traffic sites, administrators can use this setting to postpone the calculation of vote results.'),
    '#default_value' => variable_get('votingapi_calculation_schedule', 'immediate'),
    '#options' => array(
      'immediate' => t('Tally results whenever a vote is cast'),
      'cron' => t('Tally results at cron-time'),
      'manual' => t('Do not tally results automatically: I am using a module that manages its own vote results.')
    ),
  );

  return system_settings_form($form);
}

/**
 * Developer tool to generate dummy votes.
 */
function votingapi_generate_votes_form() {
  $types = node_get_types();
  foreach ($types as $type) {
    $options[$type->type] = t($type->name);
  }

  $form['node_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Which node types should receive votes?'),
    '#options' => $options,
    '#default_value' => array_keys($options),
  );

  $form['vote_type'] = array(
    '#type' => 'select',
    '#title' => t('What type of votes should be generated?'),
    '#options' => array(
      'five' => t('Fivestar style'),
      'flag' => t('Digg style'),
      'updown' => t('Reddit style'),
    ),
    '#default_value' => 'percent',
  );

  $form['kill_votes'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete existing votes before generating new ones.'),
    '#default_value' => FALSE,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Do it!'),
  );
  return $form;
}

/**
 * Submit handler for votingapi_generate_votes_form.
 */
function votingapi_generate_votes_form_submit($form, &$form_state) {
  votingapi_generate_votes($form_state['values']['node_types'], $form_state['values']['vote_type'], $form_state['values']['kill_votes']);
}

/**
 * Utility function to generate votes.
 */
function votingapi_generate_votes($node_types, $vote_type, $kill_votes = FALSE) {
  module_load_include('inc', 'devel_generate');

  if ($kill_votes) {
    db_query("TRUNCATE TABLE {votingapi_vote}");
    db_query("TRUNCATE TABLE {votingapi_cache}");
  }

  $uids = devel_get_users();
  $nids = array();
  
  $sql  = "SELECT n.nid, n.created FROM {node} n ";
  $sql .= "WHERE n.type IN (" . db_placeholders($node_types, 'varchar') . ") ";
  $sql .= "ORDER BY n.created DESC";
  
  $results = db_query($sql, $node_types);
  while ($node = db_fetch_array($results)) {
    $nids[$node['nid']] = $node['created'];
  }
  
  foreach ($nids as $nid => $timestamp) {
    _votingapi_cast_votes($nid, $timestamp, $uids, $vote_type);
  }
}

/**
 * Utility function to generate votes on a node by a set of users.
 */
function _votingapi_cast_votes($nid, $timestamp, $uids, $style) {
  $votes = array();
  foreach ($uids as $uid) {
    switch ($style) {
      case 'five':
        if (rand(0, 2)) {
          $votes[] = array(
            'uid' => $uid,
            'content_id' => $nid,
            'value_type' => 'percent',
            'timestamp' => time() - rand(0, time() - $timestamp),
            'value' => rand(0, 5) * 20,
          );
        }
        break;
      case 'flag':
        if (rand(0, 1)) {
          $votes[] = array(
            'uid' => $uid,
            'content_id' => $nid,
            'value_type' => 'points',
            'timestamp' => time() - rand(0, time() - $timestamp),
            'value' => 1,
          );
        }
        break;
      case 'updown':
        if (rand(0, 3)) {
          $votes[] = array(
            'uid' => $uid,
            'content_id' => $nid,
            'value_type' => 'points',
            'timestamp' => time() - rand(0, time() - $timestamp),
            'value' => rand(0, 1) ? 1 : -1,
          );
        }
        break;
    }
  }
  votingapi_set_votes($votes, array());
}
