<?php

/**
 * Implementation of hook_default_fe_block_settings().
 */
function boinc_standard_default_fe_block_settings() {
  $export = array();

  // boinc
  $theme = array();

  $theme['menu_block-1'] = array(
    'module' => 'menu_block',
    'delta' => '1',
    'theme' => 'boinc',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '<none>',
    'cache' => '-1',
  );

  $export['boinc'] = $theme;

  // einstein
  $theme = array();

  $theme['menu_block-1'] = array(
    'module' => 'menu_block',
    'delta' => '1',
    'theme' => 'einstein',
    'status' => 0,
    'weight' => '-23',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '<none>',
    'cache' => '-1',
  );

  $export['einstein'] = $theme;

  // garland
  $theme = array();

  $theme['menu_block-1'] = array(
    'module' => 'menu_block',
    'delta' => '1',
    'theme' => 'garland',
    'status' => 0,
    'weight' => '0',
    'region' => '',
    'custom' => '0',
    'throttle' => '0',
    'visibility' => '0',
    'pages' => '',
    'title' => '<none>',
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
