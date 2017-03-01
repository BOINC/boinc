<?php

/**
 * Implementation of hook_default_fe_block_settings().
 */
function mobilemenu_default_fe_block_settings() {
  $export = array();

  // boinc
  $theme = array();

  $theme['mobile_menu_toggle-0'] = array(
    'module' => 'mobile_menu_toggle',
    'delta' => '0',
    'theme' => 'boinc',
    'status' => '1',
    'weight' => '0',
    'region' => 'header',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '<none>',
    'cache' => '1',
  );

  $export['boinc'] = $theme;

  // einstein
  $theme = array();

  $theme['mobile_menu_toggle-0'] = array(
    'module' => 'mobile_menu_toggle',
    'delta' => '0',
    'theme' => 'einstein',
    'status' => '1',
    'weight' => '-25',
    'region' => 'header',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '<none>',
    'cache' => '1',
  );

  $export['einstein'] = $theme;

  // garland
  $theme = array();

  $theme['mobile_menu_toggle-0'] = array(
    'module' => 'mobile_menu_toggle',
    'delta' => '0',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '<none>',
    'cache' => '1',
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
