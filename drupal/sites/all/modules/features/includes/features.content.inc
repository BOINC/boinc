<?php

/**
 * Implementation of hook_features_api().
 */
function content_features_api() {
  return array(
    'content' => array(
      'name' => t('CCK'),
      'default_hook' => 'content_default_fields',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
      'feature_source' => TRUE,
    ),
  );
}

/**
 * Implementation of hook_features_export_options().
 */
function content_features_export_options() {
  $options = array();
  foreach (node_get_types('names') as $type => $type_name) {
    $content_info = content_types($type);
    if (!empty($content_info['fields'])) {
      foreach ($content_info['fields'] as $field) {
        $field_name = isset($field['widget']['label']) ? $field['widget']['label'] : $field['field_name'];
        $options[content_features_identifier($field)] = "{$type_name}: {$field_name}";
      }
    }
  }
  return $options;
}

/**
 * Implementation of hook_features_export().
 */
function content_features_export($data, &$export, $module_name = '') {
  features_include_defaults('content');
  $pipe = array();

  // The content_default_fields() hook integration is provided by the
  // features module so we need to add it as a dependency.
  $export['dependencies']['features'] = 'features';

  // Collect a field to module map
  $map = features_get_default_map('content', NULL, 'content_features_identifier');
  foreach ($data as $instance) {
    // If this field is already provided by another module remove the field.
    // We do not add the other module as a dependency as the field-providing
    // module may be extending the content type in question.
    if (isset($map[$instance]) && $map[$instance] != $module_name) {
      if (isset($export['features']['content'][$instance])) {
        unset($export['features']['content'][$instance]);
      }
    }
    // If the field has not yet been exported, add it
    else {
      $split = explode('-', $instance);
      $type_name = $split[0];
      $field_name = $split[1];
      $field = content_fields($field_name, $type_name);

      if (!empty($field)) {
        // Add field item instance for later export.
        $export['features']['content'][$instance] = $instance;

        // Add module which provides field.
        $export['dependencies'][$field['module']] = $field['module'];

        // Add module which provides field widget.
        $export['dependencies'][$field['widget']['module']] = $field['widget']['module'];

        // Add modules which provide display.
        foreach (array('teaser', 'full') as $display) {
          $formatter = _content_get_formatter($field['display_settings'][$display]['format'], $field['type']);
          $export['dependencies'][$formatter['module']] = $formatter['module'];

          // TODO make this logic more generic, for now though we just handle
          // the imagecache presets.
          if ($formatter['module'] == 'imagecache') {
            $format = $field['display_settings'][$display]['format'];

            $parts = explode('_', $format);
            $style = array_pop($parts);
            $presetname = implode('_', $parts);

            $pipe[$formatter['module']][] = $presetname;
          }
        }
      }
    }
  }

  return $pipe;
}

/**
 * Implementation of hook_features_export_render().
 */
function content_features_export_render($module, $data) {
  $translatables = $code = array();

  $code[] = '  $fields = array();';
  $code[] = '';
  foreach ($data as $instance) {
    $instance = explode('-', $instance);
    $type_name = $instance[0];
    $field_name = $instance[1];
    if ($field = content_fields($field_name, $type_name)) {
      unset($field['columns']);
      unset($field['locked']);
      unset($field['db_storage']);
      $field_identifier = features_var_export(content_features_identifier($field));
      $field_export = features_var_export($field, '  ');

      $code[] = "  // Exported field: {$field_name}";
      $code[] = "  \$fields[{$field_identifier}] = {$field_export};";
      $code[] = "";

      // Add any labels to translatables array.
      if (!empty($field['widget']['label'])) {
        $translatables[] = $field['widget']['label'];
      }
    }
  }
  if (!empty($translatables)) {
    $code[] = features_translatables_export($translatables, '  ');
  }
  $code[] = '  return $fields;';
  $code = implode("\n", $code);
  return array('content_default_fields' => $code);
}

/**
 * Implementation of hook_features_revert().
 */
function content_features_revert($module) {
  content_features_rebuild($module);
}

/**
 * Implementation of hook_features_rebuild().
 * Rebuilds CCK field definitions from code defaults.
 */
function content_features_rebuild($module) {
  if ($fields = features_get_default('content', $module)) {
    module_load_include('inc', 'content', 'includes/content.crud');
    content_clear_type_cache(TRUE);

    foreach ($fields as $field) {
      // We use content_field_instance_read() here so that we can get inactive 
      // fields too. We can't just pass $field to it as the fnc will add every
      // item of the array to the WHERE clause.
      $param = array('field_name' => $field['field_name'], 'type_name' => $field['type_name']);
      $existing_instance = array_shift(content_field_instance_read($param, TRUE));

      if (!empty($existing_instance)) {
        // If this existing instance is inactive, we need to activate it before
        // running content_field_instance_update().
        if (empty($existing_instance['widget_active'])) {
          db_query("UPDATE {". content_instance_tablename() ."} SET widget_active = 1 WHERE field_name = '%s' AND type_name = '%s'", $field['field_name'], $field['type_name']);
          // We need to clear the type cache again, unfortunately.
          content_clear_type_cache(TRUE);
        }

        content_field_instance_update($field, FALSE);
      }
      else {
        // Summary: content_field_instance_create() will fall back to prior
        // instance values for weight, label, desc if not set. 
        // See http://drupal.org/node/686240#comment-2786162
        $field['weight'] = $field['widget']['weight'];
        $field['label'] = $field['widget']['label'];
        $field['description'] = $field['widget']['description'];
        content_field_instance_create($field, FALSE);
      }
      variable_set('menu_rebuild_needed', TRUE);
    }
  }
}

/**
 * Callback for generating the field exportable identifier for a field.
 */
function content_features_identifier($field) {
  return isset($field['type_name'], $field['field_name']) ? "{$field['type_name']}-{$field['field_name']}" : FALSE;
}

/**
 * Helper function: retrieve default fields by node type.
 */
function content_features_fields_default($node_type, $reset = FALSE) {
  static $content_default_fields;
  if (!isset($content_default_fields) || $reset) {
    foreach (features_get_default('content') as $field) {
      $content_default_fields[$field['type_name']][] = $field['field_name'];
    }
  }
  return isset($content_default_fields[$node_type]) ? $content_default_fields[$node_type] : array();
}

/**
 * Helper function: retrieve normal fields by node type.
 */
function content_features_fields_normal($node_type) {
  $normal_fields = array();
  $content_info = content_types($node_type);
  if (!empty($content_info['fields'])) {
    $normal_fields = array_keys($content_info['fields']);
  }
  return $normal_fields;
}
