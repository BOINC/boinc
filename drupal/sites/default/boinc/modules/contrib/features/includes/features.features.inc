<?php

/**
 * Implementation of hook_features_api().
 */
function features_features_api() {
  return array(
    'dependencies' => array(
      'name' => 'Dependencies',
      'feature_source' => TRUE,
      'duplicates' => FEATURES_DUPLICATES_ALLOWED,
    ),
  );
}

/**
 * Implementation of hook_features_export_options().
 */
function dependencies_features_export_options() {
  // Excluded modules.
  $excluded = drupal_required_modules();
  $options = array();
  foreach (features_get_modules() as $module_name => $info) {
    if (!in_array($module_name, $excluded) && $info->status && !empty($info->info)) {
      $options[$module_name] = $info->info['name'];
    }
  }
  return $options;
}

/**
 * Implementation of hook_features_export().
 */
function dependencies_features_export($data, &$export, $module_name = '') {
  // Don't allow a module to depend upon itself.
  if (!empty($data[$module_name])) {
    unset($data[$module_name]);
  }

  // Clean up existing dependencies and merge.
  $export['dependencies'] = _features_export_minimize_dependencies($export['dependencies'], $module_name);
  $export['dependencies'] = array_merge($data, $export['dependencies']);
  $export['dependencies'] = array_unique($export['dependencies']);
}

/**
 * Implementation of hook_features_revert().
 */
function dependencies_features_revert($module) {
  dependencies_features_rebuild($module);
}

/**
 * Implementation of hook_features_rebuild().
 * Ensure that all of a feature's dependencies are enabled.
 */
function dependencies_features_rebuild($module) {
  $feature = features_get_features($module);
  if (!empty($feature->info['dependencies'])) {
    $install = array();
    foreach ($feature->info['dependencies'] as $dependency) {
      if (!module_exists($dependency)) {
        $install[] = $dependency;
      }
    }
    if (!empty($install)) {
      features_install_modules($install);
    }
  }
}
