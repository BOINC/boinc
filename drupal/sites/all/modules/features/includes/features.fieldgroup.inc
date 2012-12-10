<?php

/**
 * Implementation of hook_features_api().
 */
function fieldgroup_features_api() {
  return array(
    'fieldgroup' => array(
      'name' => t('Fieldgroup'),
      'default_hook' => 'fieldgroup_default_groups',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
    ),
  );
}

/**
 * Implementation of hook_features_export().
 */
function fieldgroup_features_export($data, &$export, $module_name = '') {
  features_include_defaults('fieldgroup');
  $pipe = array();

  // The hook_fieldgroup_default_groups() hook integration is provided by the
  // features module so we need to add it as a dependency.
  $export['dependencies']['features'] = 'features';

  // Collect a group to module map
  $map = features_get_default_map('fieldgroup', NULL, 'fieldgroup_features_identifier');
  foreach ($data as $instance) {
    // If this group is already provided by another module, remove the group
    // and add the other module as a dependency.
    if (isset($map[$instance]) && $map[$instance] != $module_name) {
      if (isset($export['features']['fieldgroup'][$instance])) {
        unset($export['features']['fieldgroup'][$instance]);
      }
      $module = $map[$instance];
      $export['dependencies'][$module] = $module;
    }
    // If the group has not yet been exported, add it.
    else {
      $split = explode('-', $instance);
      $type_name = $split[0];
      $group_name = $split[1];
      $groups = fieldgroup_groups($type_name, FALSE, TRUE);

      if (isset($groups[$group_name]) && $group = $groups[$group_name]) {
        $export['features']['fieldgroup'][$instance] = $instance;
        $export['dependencies']['fieldgroup'] = 'fieldgroup';
      }
    }
  }

  return $pipe;
}

/**
 * Implementation of hook_features_export_render().
 */
function fieldgroup_features_export_render($module, $data) {
  $translatables = $code = array();

  $code[] = '  $groups = array();';
  $code[] = '';
  foreach ($data as $instance) {
    $instance = explode('-', $instance);
    $type_name = $instance[0];
    $group_name = $instance[1];
    $groups = fieldgroup_groups($type_name, FALSE, TRUE);
    if (isset($groups[$group_name]) && $group = $groups[$group_name]) {
      // Clean up the fields to only list the names.
      $group['fields'] = array_keys((array) $group['fields']);

      $group_identifier = features_var_export(fieldgroup_features_identifier($group));
      $group_export = features_var_export($group, '  ');
      $code[] = "  // Exported group: {$group_name}";
      $code[] = "  \$groups[{$group_identifier}] = {$group_export};";
      $code[] = "";

      // Add any labels to translatables array.
      if (!empty($group['label'])) {
        $translatables[] = $group['label'];
      }
    }
  }
  if (!empty($translatables)) {
    $code[] = features_translatables_export($translatables, '  ');
  }
  $code[] = '  return $groups;';
  $code = implode("\n", $code);
  return array('fieldgroup_default_groups' => $code);
}

/**
 * Implementation of hook_features_revert().
 */
function fieldgroup_features_revert($module) {
  fieldgroup_features_rebuild($module);
}

/**
 * Implementation of hook_features_rebuild().
 * Rebuilds CCK fieldgroup definitions from code defaults.
 */
function fieldgroup_features_rebuild($module) {
  if ($groups = features_get_default('fieldgroup', $module)) {
    content_clear_type_cache(TRUE);

    foreach ($groups as $group) {
      $type_name = $group['type_name'];
      $group_name = $group['group_name'];
      $groups = fieldgroup_groups($type_name, FALSE, TRUE);

      if (isset($groups[$group_name])) {
        $existing_group = $groups[$group_name];
        // Only field names are exported in fieldgroup_features_export_render(), so we 
        // update the existing group to match.
        $existing_group['fields'] = array_keys($existing_group['fields']);
      }

      // No need to rebuild if the group already exists and is identical.
      if ($existing_group != $group) {
        // Update each field from this group.
        foreach ($group['fields'] as $field_name) {
          if ($field = content_fields($field_name, $type_name)) {
            $field['group'] = $group_name;
            fieldgroup_update_fields($field);
          }
        }

        // Look in the old group for any fields that have been removed.
        if ($existing_group && !empty($existing_group['fields'])) {
          foreach($existing_group['fields'] as $field_name) {
            // We only want to update the field if the field no longer exists in the group 
            // and the field's existing group name matches the group currently being rebuilt.
            if (
              !in_array($field_name, $group['fields']) &&
              _fieldgroup_field_get_group($type_name, $field_name) == $group_name &&
              $field = content_fields($field_name, $type_name)
            ) {
              $field['group'] = '';
              fieldgroup_update_fields($field);
            }
          }
        }

        fieldgroup_save_group($type_name, $group);
        variable_set('menu_rebuild_needed', TRUE);
      }
    }
  }
}

/**
 * Callback for generating an identifier for a fieldgroup.
 */
function fieldgroup_features_identifier($object) {
  return isset($object['type_name'], $object['group_name']) ? "{$object['type_name']}-{$object['group_name']}" : FALSE;
}
