<?php
// $Id: devel_generate.drush.inc,v 1.1.2.14 2010/07/07 16:46:37 weitzman Exp $

/**
 * @file
 *  Generate content, taxonomy, and users via drush framework.
 */

/**
 * Implementation of hook_drush_help().
 */
function devel_generate_drush_help($section) {
  switch ($section) {
    case 'drush:generate-users':
      return dt('Generate user accounts.');
    case 'drush:generate-taxonomy':
      return dt('Generate taxonomy terms for a given vocabulary.');
    case 'drush:generate-content':
      return dt('Generate nodes and comments.');
  }
}

/**
 * Implementation of hook_drush_command().
 */
function devel_generate_drush_command() {
  $items['generate-users'] = array(
    'description' => 'Creates users.',
    'arguments' => array(
      'num_users' => 'Number of users to generate.',
    ),
    'options' => array(
      'kill' => 'Delete all users before generating new ones.',
    ),
    'aliases' => array('genu'),
  );
  $items['generate-taxonomy'] = array(
    'description' => 'Creates taxonomy.',
    'arguments' => array(
      'vid' => 'Vocabulary ID into which new terms will be inserted.',
      'num_terms' => 'Number of terms to generate',
    ),
    'options' => array(
      'kill' => 'Delete all terms in specified vocab before generating.'
    ),
    'aliases' => array('gent'),
  );
  $items['generate-content'] = array(
    'description' => 'Creates content.',
    'drupal dependencies' => array('devel_generate'),
    'arguments' => array(
      'num_nodes' => 'Number of nodes to generate.',
      'max_comments' => 'Maximum number of comments to generate.',
    ),
    'options' => array(
      'kill' => 'Delete all content before generating new content.',
      'add-terms' => 'Add terms to generated nodes.',
      'add-upload' => 'Add an attached file to generated nodes.',
      'types' => 'Specify which types of nodes to generate. Should be a comma-separated list.',
      'languages' => 'A comma-separated list of language codes',
    ),
    'aliases' => array('genc'),
  );
  return $items;
}

/**
 * Command callback. Generate a number of users.
 */
function drush_devel_generate_generate_users($num_users = NULL) {
  if (drush_generate_is_number($num_users) == FALSE) {
    drush_set_error('DEVEL_GENERATE_INVALID_NUMBER', dt('Invalid number of users.'));
  }
  drush_generate_include_devel();
  drush_op('devel_create_users', $num_users, drush_get_option('kill'));
  drush_log(dt('Generated @number users.', array('@number' => $num_users)), 'ok');
}

/**
 * Command callback. Generate a number of taxonomy.
 */
function drush_devel_generate_generate_taxonomy($num_vocab = NULL, $num_terms = NULL) {
  if (drush_generate_is_number($num_vocab) == FALSE) {
    drush_set_error('DEVEL_GENERATE_INVALID_VID', dt('Invalid vocabulary ID.'));
  }
  if (drush_generate_is_number($num_terms) == FALSE) {
    drush_set_error('DEVEL_GENERATE_INVALID_NUM_TERMS', dt('Invalid number of terms.'));
  }
  drush_generate_include_devel();
  drush_op('devel_generate_taxonomy_data', $num_vocab, $num_terms, '12', drush_get_option('kill'));
  drush_log(dt('Generated @num_vocab vocabulary, @num_terms terms', array('@num_vocab' => $num_vocab, '@num_terms' => $num_terms)), 'ok');
}

/**
 * Command callback. Generate a number of content.
 */
function drush_devel_generate_generate_content($num_nodes = NULL, $max_comments = NULL, $add_language = NULL) {
  if (drush_generate_is_number($num_nodes) == FALSE) {
    drush_set_error('DEVEL_GENERATE_INVALID_NUM_NODES', dt('Invalid number of nodes'));
  }
  if (!empty($max_comments) && drush_generate_is_number($max_comments) == FALSE) {
    drush_set_error('DEVEL_GENERATE_INVALID_NUM_COMMENTS', dt('Invalid number of comments.'));
  }

  $add_language = drush_get_option('languages');
  if (!empty($add_language)) {
    $add_language = explode(',', str_replace(' ', '', $add_language));
    // Intersect with the enabled languages to make sure the language args 
    // passed are actually enabled.
    $values['values']['add_language'] = array_flip(array_intersect($add_language, array_keys(locale_language_list())));
  }

  // Let's load user 1, seems to be need for creating comments.
  global $user;
  $user_one = user_load(array('uid' => '1'));
  $user = $user_one;

  session_save_session(FALSE);
  drush_generate_include_devel();
  
  if (drush_get_option('kill')) $values['values']['kill_content'] = 1;
  $values['values']['title_length'] = '8';
  $values['values']['num_nodes'] = $num_nodes;
  $values['values']['max_comments'] = $max_comments;
  $values['values']['add_terms'] = drush_get_option('add-terms', FALSE);
  $values['values']['add_upload'] = drush_get_option('add-upload', FALSE);
  $all_types = array_keys(devel_generate_content_types());
  if (drush_get_option('types')) {
    $types = explode(',', drush_get_option('types'));
    $types = array_intersect($types, $all_types);
  }
  else {
    $types = $all_types;
  }
  $values['values']['node_types'] = drupal_map_assoc($types);

  drush_op('devel_generate_content', $values);
  drush_log(dt('Generated @num_nodes nodes, @max_comments comments per node', array('@num_nodes' => $num_nodes, '@max_comments' => $max_comments)), 'ok');
}

//////////////////////////////////////////////////////////////////////////////
// Helper functions

// verify if param is a number
function drush_generate_is_number($number) {
  if ($number == NULL) return FALSE;
  if (!is_numeric($number)) return FALSE;
  if ($number < 0) return FALSE;
  return TRUE;
}

// include devel_generate.inc
function drush_generate_include_devel() {
  $path = drupal_get_path('module', 'devel_generate');
  require_once($path .'/devel_generate.inc');
}