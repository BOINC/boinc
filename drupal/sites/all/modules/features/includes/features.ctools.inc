<?php

/**
 * This is a wild hack, but effective.
 * Dynamically declare functions under a ctools component's namespace if they are not already declared.
 */
foreach (_ctools_features_get_info() as $component => $info) {
  $code = '';
  if (!function_exists("{$info['module']}_features_api")) {
    $code .= 'function '. $info['module'] .'_features_api() { return ctools_component_features_api("'. $info['module'] .'"); }';
  }
  if (!function_exists("{$component}_features_export")) {
    $code .= 'function '. $component .'_features_export($data, &$export, $module_name = "") { return ctools_component_features_export("'. $component .'", $data, $export, $module_name); }';
  }
  if (!function_exists("{$component}_features_export_options")) {
    $code .= 'function '. $component .'_features_export_options() { return ctools_component_features_export_options("'. $component .'"); }';
  }
  if (!function_exists("{$component}_features_export_render")) {
    $code .= 'function '. $component .'_features_export_render($module, $data) { return ctools_component_features_export_render("'. $component .'", $module, $data); }';
  }
  if (!function_exists("{$component}_features_revert")) {
    $code .= 'function '. $component .'_features_revert($module) { return ctools_component_features_revert("'. $component .'", $module); }';
  }
  eval($code);
}

/**
 * Implementation of hook_features_api().
 */
function ctools_features_api() {
  return array(
    'ctools' => array(
      'name' => 'CTools export API',
      'feature_source' => TRUE,
      'duplicates' => FEATURES_DUPLICATES_ALLOWED,
      // CTools API integration does not include a default hook declaration as
      // it is not a proper default hook.
      // 'default_hook' => 'ctools_plugin_api',
    ),
  );
}

/**
 * Implementation of hook_features_export().
 * Adds references to the ctools mothership hook, ctools_plugin_api().
 */
function ctools_features_export($data, &$export, $module_name = '') {
  // Add ctools dependency
  $export['dependencies']['ctools'] = 'ctools';

  // Add the actual ctools components which will need to be accounted for in
  // hook_ctools_plugin_api(). The components are actually identified by a
  // delimited list of values: `module_name:api:current_version`
  foreach ($data as $component) {
    if ($info = _ctools_features_get_info($component)) {
      $identifier = "{$info['module']}:{$info['api']}:{$info['current_version']}";
      $export['features']['ctools'][$identifier] = $identifier;
    }
  }

  return array();
}

/**
 * Implementation of hook_features_export_render().
 * Adds the ctools mothership hook, ctools_plugin_api().
 */
function ctools_features_export_render($module, $data) {
  $code = array();
  $code[] = '  list($module, $api) = func_get_args();';

  $first = TRUE;
  foreach ($data as $component) {
    if ($info = _ctools_features_get_info($component)) {
      $if = $first ? 'if' : 'elseif';
      $code[] = '  '. $if .' ($module == "'. $info['module'] .'" && $api == "'. $info['api'] .'") {';
      $code[] = '    return array("version" => '. $info['current_version'] .');';
      $code[] = '  }';

      $first = FALSE;
    }
  }
  return array('ctools_plugin_api' => implode("\n", $code));
}

/**
 * Master implementation of hook_features_api() for all ctools components.
 *
 * Note that this master hook does not use $component like the others, but uses the
 * component module's namespace instead.
 */
function ctools_component_features_api($module_name) {
  $api = array();
  foreach (_ctools_features_get_info() as $component => $info) {
    if ($info['module'] === $module_name) {
      $api[$component] = $info;
    }
  }
  return $api;
}

/**
 * Master implementation of hook_features_export_options() for all ctools components.
 */
function ctools_component_features_export_options($component) {
  $options = array();

  ctools_include('export');
  $schema = ctools_export_get_schema($component);
  if ($schema && $schema['export']['bulk export']) {
    if (!empty($schema['export']['list callback']) && function_exists($schema['export']['list callback'])) {
      $options = $schema['export']['list callback']();
    }
    else {
      $options = _ctools_features_export_default_list($component, $schema);
    }
  }
  asort($options);
  return $options;
}

/**
 * Master implementation of hook_features_export() for all ctools components.
 */
function ctools_component_features_export($component, $data, &$export, $module_name = '') {
  // Add the actual implementing module as a dependency
  $info = _ctools_features_get_info();
  if ($module_name !== $info[$component]['module']) {
    $export['dependencies'][$info[$component]['module']] = $info[$component]['module'];
  }

  // Add the components
  foreach ($data as $object_name) {
    if ($object = _ctools_features_export_crud_load($component, $object_name)) {
      // If this object is provided as a default by a different module, don't
      // export and add that module as a dependency instead.
      if (!empty($object->export_module) && $object->export_module !== $module_name) {
        $export['dependencies'][$object->export_module] = $object->export_module;
        if (isset($export['features'][$component][$object_name])) {
          unset($export['features'][$component][$object_name]);
        }
      }
      // Otherwise, add the component.
      else {
        $export['features'][$component][$object_name] = $object_name;
      }
    }
  }

  // Let CTools handle API integration for this component.
  return array('ctools' => array($component));
}

/**
 * Master implementation of hook_features_export_render() for all ctools components.
 */
function ctools_component_features_export_render($component, $module, $data) {
  ctools_include('export');
  $schema = ctools_export_get_schema($component);

  if (function_exists($schema['export']['to hook code callback'])) {
    $export = $schema['export']['to hook code callback']($data, $module);
    $code = explode("{\n", $export);
    array_shift($code);
    $code = explode('}', implode($code, "{\n"));
    array_pop($code);
    $code = implode('}', $code);
  }
  else {
    $code = '  $export = array();'."\n\n";
    foreach ($data as $object_name) {
      if ($object = _ctools_features_export_crud_load($component, $object_name)) {
        $identifier = $schema['export']['identifier'];
        $code .= _ctools_features_export_crud_export($component, $object, '  ');
        $code .= "  \$export[" . ctools_var_export($object_name) . "] = \${$identifier};\n\n";
      }
    }
    $code .= '  return $export;';
  }

  return array($schema['export']['default hook'] => $code);
}

/**
 * Master implementation of hook_features_revert() for all ctools components.
 */
function ctools_component_features_revert($component, $module) {
  if ($objects = features_get_default($component, $module)) {
    foreach ($objects as $object) {
      _ctools_features_export_crud_delete($component, $object);
    }
  }
}

/**
 * Helper function to return various ctools information for components.
 */
function _ctools_features_get_info($identifier = NULL, $reset = FALSE) {
  static $components;
  if (!isset($components) || $reset) {
    $components = array();
    $modules = features_get_info();
    ctools_include('export');
    foreach (ctools_export_get_schemas_by_module() as $module => $schemas) {
      foreach ($schemas as $table => $schema) {
        if ($schema['export']['bulk export']) {
          // Let the API owner take precedence as the owning module.
          $api_module = isset($schema['export']['api']['owner']) ? $schema['export']['api']['owner'] : $module;
          $components[$table] = array(
            'name' => isset($modules[$api_module]->info['name']) ? $modules[$api_module]->info['name'] : $api_module,
            'default_hook' => $schema['export']['default hook'],
            'default_file' => FEATURES_DEFAULTS_CUSTOM,
            'module' => $api_module,
            'feature_source' => TRUE,
          );
          if (isset($schema['export']['api'])) {
            $components[$table] += array(
              'api' => $schema['export']['api']['api'],
              'default_filename' => $schema['export']['api']['api'],
              'current_version' => $schema['export']['api']['current_version'],
            );
          }
        }
      }
    }
  }

  // Return information specific to a particular component.
  if (isset($identifier)) {
    // Identified by the table name.
    if (isset($components[$identifier])) {
      return $components[$identifier];
    }
    // New API identifier. Allows non-exportables related CTools APIs to be
    // supported by an explicit `module:api:current_version` key. 
    else if (substr_count($identifier, ':') === 2) {
      list($module, $api, $current_version) = explode(':', $identifier);
      // If a schema component matches the provided identifier, provide that
      // information. This also ensures that the version number is up to date.
      foreach ($components as $table => $info) {
        if ($info['module'] == $module && $info['api'] == $api && $info['current_version'] >= $current_version) {
          return $info;
        }
      }
      // Fallback to just giving back what was provided to us.
      return array('module' => $module, 'api' => $api, 'current_version' => $current_version);
    }
    return FALSE;
  }

  return $components;
}

/**
 * Wrapper around ctools_export_crud_export() for < 1.7 compatibility.
 */
function _ctools_features_export_crud_export($table, $object, $indent = '') {
  return ctools_api_version('1.7') ? ctools_export_crud_export($table, $object, $indent) : ctools_export_object($table, $object, $indent);
}

/**
 * Wrapper around ctools_export_crud_load() for < 1.7 compatibility.
 */
function _ctools_features_export_crud_load($table, $name) {
  if (ctools_api_version('1.7')) {
    return ctools_export_crud_load($table, $name);
  }
  elseif ($objects = ctools_export_load_object($table, 'names', array($name))) {
    return array_shift($objects);
  }
  return FALSE;
}

/**
 * Wrapper around ctools_export_default_list() for < 1.7 compatibility.
 */
function _ctools_features_export_default_list($table, $schema) {
  if (ctools_api_version('1.7')) {
    return ctools_export_default_list($table, $schema);
  }
  elseif ($objects = ctools_export_load_object($table, 'all')) {
    return drupal_map_assoc(array_keys($objects));
  }
  return array();
}

/**
 * Wrapper around ctools_export_crud_delete() for < 1.7 compatibility.
 */
function _ctools_features_export_crud_delete($table, $object) {
  if (ctools_api_version('1.7')) {
    ctools_export_crud_delete($table, $object);
  }
  else {
    $schema = ctools_export_get_schema($table);
    $export = $schema['export'];
    db_query("DELETE FROM {{$table}} WHERE {$export['key']} = '%s'", $object->{$export['key']});
  }
}

/**
 * Implementation of hook_features_export_render() for page_manager.
 */
function page_manager_pages_features_export_render($module, $data) {
  // Ensure that handlers have their code included before exporting.
  page_manager_get_tasks();
  return ctools_component_features_export_render('page_manager_pages', $module, $data);
}

/**
 * Implementation of hook_features_revert() for page_manager.
 */
function page_manager_pages_features_revert($module) {
  if ($pages = features_get_default('page_manager_pages', $module)) {
    require_once drupal_get_path('module','ctools').'/page_manager/plugins/tasks/page.inc';
    foreach ($pages as $page) {
      page_manager_page_delete($page);
    }
  }
}
