<?php

/**
 * @file
 *
 * Administrative items for the panels mini module.
 */
/**
 * Implementation of hook_menu().
 */
function _panels_mini_menu() {
  // Provide some common options to reduce code repetition.
  // By using array addition and making sure these are the rightmost
  // value, they won't override anything already set.
  $base = array(
    'access arguments' => array('create mini panels'),
    'file' => 'panels_mini.admin.inc',
  );

  $items['admin/build/mini-panels/settings'] = array(
    'title' => 'Settings',
    'page callback' => 'panels_mini_settings',
    'type' => MENU_LOCAL_TASK,
  ) + $base;
  // Also provide settings on the main panel UI
  $items['admin/build/panels/settings/panels-mini'] = array(
    'title' => 'Mini panels',
    'page callback' => 'panels_mini_settings',
    'type' => MENU_LOCAL_TASK,
  ) + $base;

  return $items;
}

/**
 * Settings for mini panels.
 */
function panels_mini_settings() {
  ctools_include('common', 'panels');
  return drupal_get_form('panels_common_settings', 'panels_mini');
}
