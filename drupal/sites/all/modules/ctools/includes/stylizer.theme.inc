<?php

/**
 * @file
 * Contains theme registry and theme implementations for the content types.
 */

/**
 * Implementation of hook_theme to load all content plugins and pass thru if
 * necessary.
 */
function ctools_stylizer_theme(&$theme) {
  $theme['ctools_stylizer_color_scheme_form'] = array(
    'arguments' => array('form' => NULL),
    'file' => 'includes/stylizer.inc',
  );

  $theme['ctools_stylizer_preview_form'] = array(
    'arguments' => array('form' => NULL),
    'file' => 'includes/stylizer.inc',
  );

  $theme['ctools_style_icon'] = array(
    'arguments' => array('image' => NULL, 'title' => NULL),
    'file' => 'includes/stylizer.inc',
  );

  ctools_include('stylizer');
  // Register all themes given for basetypes.
  $plugins = ctools_get_style_bases();
  $base_types = ctools_get_style_base_types();
  foreach ($plugins as $plugin) {
    if (!empty($base_types[$plugin['module']][$plugin['type']]) && !empty($plugin['theme'])) {
      $base_type = $base_types[$plugin['module']][$plugin['type']];
      $theme[$plugin['theme']] = array(
        'arguments' => $base_type['theme arguments'],
        'path' => $plugin['path'],
      );

      // if no theme function exists, assume template.
      if (!function_exists("theme_$plugin[theme]")) {
        $theme[$plugin['theme']]['template'] = str_replace('_', '-', $plugin['theme']);
        $theme[$plugin['theme']]['file'] = $plugin['file']; // for preprocess.
      }
    }
  }
}

