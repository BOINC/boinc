<?php

/**
 * Implementation of hook_features_api().
 */
function filter_features_api() {
  return array(
    'filter' => array(
      'name' => t('Filter formats'),
      'default_hook' => 'filter_default_formats',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
    ),
  );
}

/**
 * Implementation of hook_features_export().
 */
function filter_features_export($data, &$export, $module_name = '') {
  // The filter_default_formats() hook integration is provided by the
  // features module so we need to add it as a dependency.
  $export['dependencies']['features'] = 'features';

  $formats = _filter_get_formats();
  foreach ($formats as $format) {
    if (in_array($format['name'], $data)) {
      // Add format to exports
      $export['features']['filter'][$format['name']] = $format['name'];

      // Iterate through filters and ensure each filter's module is included as a dependency
      foreach ($format['filters'] as $filter) {
        $export['dependencies'][$filter['module']] = $filter['module'];
      }
    }
  }

  $pipe = array();
  return $pipe;
}

/**
 * Implementation of hook_features_export_render().
 */
function filter_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $formats = array();';
  $code[] = '';

  $formats = _filter_get_formats();
  foreach ($formats as $format) {
    if (in_array($format['name'], $data)) {
      $format_export = features_var_export($format, '  ');
      $format_identifier = features_var_export($format['name']);
      $code[] = "  // Exported format: {$format['name']}";
      $code[] = "  \$formats[{$format_identifier}] = {$format_export};";
      $code[] = "";
    }
  }

  $code[] = '  return $formats;';
  $code = implode("\n", $code);
  return array('filter_default_formats' => $code);
}

/**
 * Implementation of hook_features_revert().
 */
function filter_features_revert($module) {
  return filter_features_rebuild($module);
}

/**
 * Implementation of hook_features_rebuild().
 */
function filter_features_rebuild($module) {
  if ($defaults = features_get_default('filter', $module)) {
    foreach ($defaults as $format) {
      _filter_features_update($format);
    }
  }
}

/**
 * Retrieve all input formats with their respective filters loaded.
 */
function _filter_get_formats() {
  $formats = array();

  // We cannot use user_roles() here because it has the names translated.
  $roles = _features_get_roles();
  $result = db_query("SELECT * FROM {filter_formats}");
  while ($row = db_fetch_object($result)) {
    $format_roles = array();
    // Prepare a roles array with roles that may access the filter.
    foreach ($roles as $name => $role) {
      if (strstr($row->roles, ','. $role['rid'] .',')) {
        $format_roles[] = $name;
      }
    }
    $formats[$row->format] = array(
      'name' => $row->name,
      'roles' => $format_roles,
      'filters' => array(),
    );
    foreach (_filter_list_format($row->format, TRUE) as $filter) {
      $formats[$row->format]['filters'][] = array(
        'module' => $filter->module,
        'delta' => $filter->delta,
        'weight' => $filter->weight,
      );
    }
  }

  return $formats;
}

/**
 * Direct copy of filter_list_format() but with a way to clear the static.
 */
function _filter_list_format($format, $reset = FALSE) {
  static $filters = array();

  if (!isset($filters[$format]) || $reset) {
    $result = db_query("SELECT * FROM {filters} WHERE format = %d ORDER BY weight, module, delta", $format);
    if (db_affected_rows($result) == 0 && !db_result(db_query("SELECT 1 FROM {filter_formats} WHERE format = %d", $format))) {
      // The format has no filters and does not exist, use the default input
      // format.
      $filters[$format] = filter_list_format(variable_get('filter_default_format', 1));
    }
    else {
      $filters[$format] = array();
      while ($filter = db_fetch_object($result)) {
        $list = module_invoke($filter->module, 'filter', 'list');
        if (isset($list) && is_array($list) && isset($list[$filter->delta])) {
          $filter->name = $list[$filter->delta];
          $filters[$format][$filter->module .'/'. $filter->delta] = $filter;
        }
      }
    }
  }

  return $filters[$format];
}


/**
 * Helper function for updating/inserting formats.
 */
function _filter_features_update($format) {
  $format_id = db_result(db_query("SELECT format FROM {filter_formats} WHERE name = '%s'", $format['name']));

  // Format the role IDs into something that can be inserted into the database.
  // This is so painful. See filter.admin.inc line 218. : (
  // We cannot use user_roles() here because it has the names translated.
  $roles = _features_get_roles();
  $format_rids = array();
  foreach ($format['roles'] as $role_name) {
    // Ensure that each role exists. If it does not, create it and store the rid.
    if (!isset($roles[$role_name])) {
      $record = array('name' => $role_name);
      drupal_write_record('role', $record);
      $roles[$role_name]['rid'] = $record['rid'];
      $roles[$role_name]['perm'] = array();
    }
    // Retrieve the rid corresponding to each role and add it to the format's rids
    if (isset($roles[$role_name])) {
      $format_rids[] = $roles[$role_name]['rid'];
    }
  }
  if ($format_id && $format_id == variable_get('filter_default_format', 1)) {
    $format['roles'] = ','. implode(',', array_keys(user_roles())) .',';
  }
  else {
    $format['roles'] = ','. implode(',', $format_rids) .',';
  }

  // Update or insert the format
  if ($format_id) {
    $format['format'] = $format_id;
    drupal_write_record('filter_formats', $format, 'format');
  }
  else {
    drupal_write_record('filter_formats', $format);
  }

  // Update the filters for the format
  if ($format['format']) {
    db_query("DELETE FROM {filters} WHERE format = %d", $format['format']);
    foreach ($format['filters'] as $filter) {
      $filter['format'] = $format['format'];
      drupal_write_record('filters', $filter);
    }
  }
}
