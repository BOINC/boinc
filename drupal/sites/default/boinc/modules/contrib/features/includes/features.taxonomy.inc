<?php

/**
 * Implementation of hook_features_api().
 */
function taxonomy_features_api() {
  return array(
    'taxonomy' => array(
      'name' => t('Taxonomy'),
      'feature_source' => TRUE,
      'default_hook' => 'taxonomy_default_vocabularies',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
    ),
  );
}

/**
 * Implementation of hook_features_export_options().
 */
function taxonomy_features_export_options() {
  $vocabularies = array();
  foreach (_taxonomy_features_get_vocabularies() as $machine_name => $vocabulary) {
    $vocabularies[$machine_name] = $vocabulary->name;
  }
  return $vocabularies;
}

/**
 * Implementation of hook_features_export().
 *
 * @todo Test adding existing dependencies.
 */
function taxonomy_features_export($data, &$export, $module_name = '') {
  // taxonomy_default_vocabularies integration is provided by Features.
  $export['dependencies']['features'] = 'features';
  $export['dependencies']['taxonomy'] = 'taxonomy';

  // Add dependencies for each vocabulary.
  $map = features_get_default_map('taxonomy');
  foreach ($data as $machine_name) {
    if (isset($map[$machine_name]) && $map[$machine_name] != $module_name) {
      $export['dependencies'][$map[$machine_name]] = $map[$machine_name];
    }
    else {
      $export['features']['taxonomy'][$machine_name] = $machine_name;
    }
  }
  return array();
}

/**
 * Implementation of hook_features_export_render().
 */
function taxonomy_features_export_render($module, $data) {
  $vocabularies = _taxonomy_features_get_vocabularies();
  $code = array();
  foreach ($data as $machine_name) {
    foreach ($vocabularies as $k => $v) {
      if ($k == $machine_name) {
        unset($v->vid);
        $code[$machine_name] = $v;
      }
    }
  }
  $code = "  return ". features_var_export($code, '  ') .";";
  return array('taxonomy_default_vocabularies' => $code);
}

/**
 * Implementation of hook_features_revert().
 */
function taxonomy_features_revert($module) {
  taxonomy_features_rebuild($module);
}

/**
 * Implementation of hook_features_rebuild().
 *
 * Rebuilds Taxonomy vocabularies from code defaults.
 */
function taxonomy_features_rebuild($module) {
  if ($vocabularies = features_get_default('taxonomy', $module)) {
    foreach ($vocabularies as $machine_name => $vocabulary) {
      if ($vid = db_result(db_query("SELECT vid FROM {vocabulary} WHERE module = '%s'", 'features_' . $machine_name))) {
        $vocabulary['vid'] = $vid;
      }
      taxonomy_save_vocabulary($vocabulary);
    }
  }
}

/**
 * Retrieves all vocabularies viable for export.
 *
 * This function omits the vocabularies whose 'module' property doesn't begin
 * with 'features_' so that only vocabularies with non-serial identifiers are returned.
 */
function _taxonomy_features_get_vocabularies() {
  $vocabularies = array();
  foreach (taxonomy_get_vocabularies() as $k => $vocabulary) {
    if (strpos($vocabulary->module, 'features_') === 0) {
      // Don't bother site builder with 'features_'.
      $vocabularies[substr($vocabulary->module, 9)] = $vocabulary;
    }
  }
  return $vocabularies;
}

/**
 * @see features_form_taxonomy_form_vocabulary_alter().
 */
function _features_form_taxonomy_form_vocabulary_alter(&$form, &$form_state) {
  $elem = array(
    '#type' => 'textfield',
    '#title' => t('Machine name'),
    '#description' => t('A system name for this vocabulary, may not be changed once it has been set.'),
  );

  if (!isset($form['module']) || $form['module']['#value'] == 'taxonomy') {
    // This vocabulary is new or wasn't assigned a machine name.
    $elem['#element_validate'] = array('features_taxonomy_machine_name_validate');
    unset($form['module']);
  }
  elseif (strpos($form['module']['#value'], 'features_') === 0) {
    // Simply display the existing machine name if we have one.
    $elem['#disabled'] = true;
    $elem['#value'] = substr($form['module']['#value'], 9);
  }
  else {
    // Some other module is responsible here, do nothing.
    return;
  }

  // Position the machine name right after the vocab name without interfering
  // with any explicit weights.
  $new = array();
  foreach ($form['identification'] as $k => $v) {
    $new[$k] = $v;
    if ($k == 'name') {
      $new['module'] = $elem;
    }
  } 
  $form['identification'] = $new;
}

// Validation handler for the machine name field.
function features_taxonomy_machine_name_validate($element, &$form_state) {
  // Validate format - lowecase + underscore only...
  if (!empty($element['#value']) && !preg_match('!^[a-z0-9_]+$!', $element['#value'])) {
    form_error($element, t('The machine name can only consist of lowercase letters, underscores, and numbers.'));
  }

  // If provided check machine name for conflicts and prepend with 'features'.
  if (!empty($element['#value'])) {
    $name = 'features_' . $element['#value'];
    $vid = db_result(db_query("SELECT vid FROM {vocabulary} WHERE module = '%s'", $name));
    if ($vid) {
      form_error($element, t('This machine name is already taken. Please enter a unique machine name or this taxonomy.'));
    }
    else {
      $form_state['values']['module'] = $name;
    }
  }
  // Otherwise, default to the default module name 'taxonomy'.
  else {
    $form_state['values']['module'] = 'taxonomy';
  }
}
