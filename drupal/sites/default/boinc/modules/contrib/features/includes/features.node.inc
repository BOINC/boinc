<?php

/**
 * Implementation of hook_features_api().
 */
function node_features_api() {
  return array(
    'node' => array(
      'name' => t('Content types'),
      'feature_source' => TRUE,
      'default_hook' => 'node_info',
    ),
  );
}

/**
 * Implementation of hook_features_export_options().
 */
function node_features_export_options() {
  return node_get_types('names');
}

/**
 * Implementation of hook_features_export.
 */
function node_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  $map = features_get_default_map('node');

  foreach ($data as $type) {
    // Poll node module to determine who provides the node type.
    if ($info = node_get_types('module', $type)) {
      if ($module_name != $info && module_exists($info)) {
        $export['dependencies'][$info] = $info;
      }
  
      // If this node type is provided by a different module, add it as a dependency
      if (isset($map[$type]) && $map[$type] != $module_name) {
        $export['dependencies'][$map[$type]] = $map[$type];
      }
      // Otherwise export the node type.
      elseif (in_array($info, array('node', 'features')) || isset($map[$type])) {
        $export['features']['node'][$type] = $type;
      }

      // Create a pipe for CCK fields
      if (module_exists('content')) {
        $content_info = content_types($type);
        if (!empty($content_info['fields'])) {
          foreach ($content_info['fields'] as $key => $field) {
            $pipe['content'][] = "{$type}-{$field['field_name']}";
          }
          // If strongarm is present, create a pipe for the extra field weights
          // variable to be exported.
          if (module_exists('strongarm')) {
            $pipe['variable'][] = "content_extra_weights_{$type}";
          }
        }
        // Create a pipe for Fieldgroups
        if (function_exists('fieldgroup_groups') && $groups = fieldgroup_groups($type)) {
          foreach ($groups as $group) {
            $pipe['fieldgroup'][] = "{$type}-{$group['group_name']}";
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
function node_features_export_render($module, $data, $export = NULL) {
  $elements = array(
    'name' => TRUE,
    'module' => FALSE,
    'description' => TRUE,
    'has_title' => FALSE,
    'title_label' => TRUE,
    'has_body' => FALSE,
    'body_label' => TRUE,
    'min_word_count' => FALSE,
    'help' => TRUE,
  );
  $output = array();
  $output[] = '  $items = array(';
  foreach ($data as $type) {
    if ($info = node_get_types('type', $type)) {
      // Force module name to be 'features' if set to 'node. If we leave as
      // 'node' the content type will be assumed to be database-stored by
      // the node module.
      $info->module = ($info->module === 'node') ? 'features' : $info->module;
      $output[] = "    '{$type}' => array(";
      foreach ($elements as $key => $t) {
        if ($t) {
          $text = str_replace("'", "\'", $info->$key);
          $text = !empty($text) ? "t('{$text}')" : "''";
          $output[] = "      '{$key}' => {$text},";
        }
        else {
          $output[] = "      '{$key}' => '{$info->$key}',";
        }
      }
      // Detect whether there are extra fields on this content type than have
      // been described in defaults. If so, add an additional key to the node
      // type info allowing it to be detected by Features as a component that can
      // be reverted (delete those extra fields) or updated (export those extra
      // fields to code).
      // Note that this key is only added if $export is not set - ie. we never
      // *actually* write this key to code.
      if (!isset($export) && module_exists('content') && module_exists($module)) {
        // If there are deleted fields, mark the content type as such
        $deleted_fields = array_diff(content_features_fields_normal($type), content_features_fields_default($type));
        if (!empty($deleted_fields)) {
          $output[] = "      'content_has_extra_fields' => TRUE,";
        }
      }
      $output[] = "    ),";
    }
  }
  $output[] = '  );';
  $output[] = '  return $items;';
  $output = implode("\n", $output);
  return array('node_info' => $output);
}

/**
 * Implementation of hook_features_revert().
 *
 * @param $module
 * name of module to revert content for
 */
function node_features_revert($module = NULL) {
  if ($default_types = features_get_default('node', $module)) {
    foreach ($default_types as $type_name => $type_info) {
      // We need to de-activate any missing fields. De-activating allows us to
      // preserve data. We de-activate by setting the widget_active flag to 0;
      // widget_active is incorrectly named, and really should be
      // instance_active
      if (module_exists('content')) {
        // Our existing fields ($fields) needs to be the first argument here,
        // so only fields that don't exist in code can be de-activated.
        if ($deleted_fields = array_diff(content_features_fields_normal($type_name), content_features_fields_default($type_name))) {
          foreach($deleted_fields as $field_name) {
            db_query("UPDATE {". content_instance_tablename() ."} SET widget_active = 0 WHERE field_name = '%s' AND type_name = '%s'", $field_name, $type_name);
          }
        }
      }
      // Delete node types
      // We don't use node_type_delete() because we do not actually
      // want to delete the node type (and invoke hook_node_type()).
      // This can lead to bad consequences like CCK deleting field
      // storage in the DB.
      db_query("DELETE FROM {node_type} WHERE type = '%s'", $type_name);
    }
    node_types_rebuild();
    menu_rebuild();
  }
}
