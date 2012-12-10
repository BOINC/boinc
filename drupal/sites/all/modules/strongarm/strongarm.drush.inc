<?php

/**
 * Implementation of hook_drush_command().
 */
function strongarm_drush_command() {
  $items = array();

  $items['strongarm-revert'] = array(
    'description' => 'Revert all strongarmed variables from code to the database.',
    'options' => array(
      'force' => 'Reset all variables, including those that are marked as already being set to the database.',
    ),
    'bootstrap' => 'DRUSH_BOOTSTRAP_DRUPAL_FULL',
  );

  return $items;
}

/**
 * Command callback for strongarm_revert.
 */
function drush_strongarm_revert() {
  _drush_strongarm_revert(drush_get_option('force', FALSE));
  drush_log('Pushed variables from code to the database.', 'success');
}

/**
 * Handle the revert of variables into the database.
 */
function _drush_strongarm_revert($force) {
  global $conf;

  $vars = strongarm_vars_load(TRUE, TRUE);
  foreach ($vars as $name => $var) {
    if ($force || !empty($var->in_code_only)) {
      if (!isset($conf[$name]) || $var->value != $conf[$name]) {
        variable_set($name, $var->value);
      }
    }
  }
}
