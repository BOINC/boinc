<?php

/**
 * Implementation of hook_default_fe_block_settings().
 */
function user_account_host_list_default_fe_block_settings() {
  $export = array();

  // boinc
  $theme = array();

  $theme['views-boinc_account_computers-block_1'] = array(
    'cache' => -1,
    'module' => 'views',
    'delta' => 'boinc_account_computers-block_1',
    'theme' => 'boinc',
    'pages' => '',
    'status' => 0,
    'weight' => 0,
    'region' => '',
    'custom' => 0,
    'throttle' => 0,
    'visibility' => 0,
    'title' => '',
  );

  $export['boinc'] = $theme;

  // einstein
  $theme = array();

  $theme['views-boinc_account_computers-block_1'] = array(
    'module' => 'views',
    'delta' => 'boinc_account_computers-block_1',
    'theme' => 'einstein',
    'status' => '1',
    'weight' => '11',
    'region' => 'sidebar_second',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '1',
    'pages' => 'account/prefs
account/prefs/computing
account/prefs/computing/edit/*',
    'title' => '',
    'cache' => '-1',
  );

  $export['einstein'] = $theme;

  // garland
  $theme = array();

  $theme['views-boinc_account_computers-block_1'] = array(
    'module' => 'views',
    'delta' => 'boinc_account_computers-block_1',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '',
    'cache' => '-1',
  );

  $export['garland'] = $theme;

  $theme_default = variable_get('theme_default', 'garland');
  $themes = list_themes();
  foreach ($export as $theme_key => $settings) {
    if ($theme_key != $theme_default && empty($themes[$theme_key]->status)) {
      unset($export[$theme_key]);
    }
  }
  return $export;
}
