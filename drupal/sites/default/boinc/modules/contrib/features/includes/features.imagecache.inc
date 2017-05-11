<?php

/**
 * Implementation of hook_features_api().
 */
function imagecache_features_api() {
  return array(
    'imagecache' => array(
      'name' => t('Imagecache presets'),
      'default_hook' => 'imagecache_default_presets',
    )
  );
}

/**
 * Implementation of hook_features_export_options().
 */
function imagecache_features_export_options() {
  $options = array();
  foreach (imagecache_presets() as $preset) {
    $options[$preset['presetname']] = $preset['presetname'];
  }
  return $options;
}

/**
 * Implementation of hook_features_export().
 */
function imagecache_features_export($data, &$export, $module_name = '') {
  // Collect a module to preset map
  $map = features_get_default_map('imagecache', 'presetname');
  foreach ($data as $preset) {
    // If another module provides this preset, add it as a dependency
    if (isset($map[$preset]) && $map[$preset] != $module_name) {
      $module = $map[$preset];
      $export['dependencies'][$module] = $module;
    }
    // Otherwise, export the preset
    else {
      $export['features']['imagecache'][$preset] = $preset;
    }
  }
}

/**
 * Implementation of hook_features_export_render().
 */
function imagecache_features_export_render($module_name, $data) {
  $items = array();
  foreach ($data as $key) {
    // This second argument to clear the static cache relies on patch in
    // http://drupal.org/node/665284.
    $preset = imagecache_preset_by_name($key, TRUE);
    _imagecache_features_preset_sanitize($preset);
    $items[$key] = $preset;
  }
  $code = "  \$items = ". features_var_export($items, '  ') .";\n";
  $code .= '  return $items;';
  return array('imagecache_default_presets' => $code);
}

/**
 * Implementation of hook_features_revert().
 */
function imagecache_features_revert($module) {
  if ($default_presets = features_get_default('imagecache', $module)) {
    foreach (array_keys($default_presets) as $default_preset) {
      $preset = imagecache_preset_by_name($default_preset);
      if ($preset) {
        imagecache_preset_delete($preset);
      }
    }
  }
}

/**
 * Remove unnecessary keys for export.
 */
function _imagecache_features_preset_sanitize(&$preset) {
  $omit = array('presetid', 'storage', 'actionid');
  if (is_array($preset)) {
    foreach ($preset as $k => $v) {
      if (in_array($k, $omit, TRUE)) {
        unset($preset[$k]);
      }
      else if (is_array($v)) {
        _imagecache_features_preset_sanitize($preset[$k]);
      }
    }
  }
}
