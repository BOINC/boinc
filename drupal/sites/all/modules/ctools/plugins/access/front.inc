<?php

/**
 * @file
 * Plugin to provide access control based on drupal_is_front_page.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t('Front page'),
  'description' => t('Is this the front page.'),
  'callback' => 'ctools_front_ctools_access_check',
  'default' => array('negate' => 0),
  'settings form' => 'ctools_front_ctools_access_settings',
  'summary' => 'ctools_front_ctools_access_summary',
);

/**
 * Settings form for the 'by parent term' access plugin
 */
function ctools_front_ctools_access_settings(&$form, &$form_state, $conf) {
  // No additional configuration necessary.
}

/**
 * Check for access.
 */
function ctools_front_ctools_access_check($conf, $context) {
  if (drupal_is_front_page()) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Provide a summary description based upon the checked terms.
 */
function ctools_front_ctools_access_summary($conf, $context) {
  return t('The front page');
}
