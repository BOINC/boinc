<?php

/**
 * Implementation of hook_features_export().
 */
function views_api_features_export($data, &$export, $module_name = '') {
  // Add views dependency
  $export['dependencies']['views'] = 'views';

  // Add the actual views API hook entries to be accounted for in
  // hook_views_api(). The components are actually identified by a delimited
  // list of values: `key:value`. Currently only the 'api' key is supported
  // as Features does not yet know how to write to subdirectories.
  foreach ($data as $component) {
    if (is_numeric($component)) {
      $version = "api:{$component}";
      $export['features']['views_api'][$version] = $version;
    }
  }

  return array();
}

/**
 * Implementation of hook_features_export_render().
 * Adds the views mothership hook, views_api().
 */
function views_api_features_export_render($module, $data) {
  $values = array();
  foreach ($data as $component) {
    $split = explode(':', $component);
    if (count($split) === 2) {
      list($key, $value) = $split;
      $values[$key] = $value;
    }
  }
  $code = '  return ' . features_var_export($values, '  ') . ';';
  return array('views_api' => $code);
}

/**
 * Implementation of hook_features_api().
 */
function views_features_api() {
  return array(
    'views' => array(
      'name' => t('Views'),
      'feature_source' => TRUE,
      'default_hook' => 'views_default_views',
      'default_file' => FEATURES_DEFAULTS_CUSTOM,
      'default_filename' => 'views_default',
    ),
    'views_api' => array(
      'name' => t('Views API'),
      'feature_source' => FALSE,
      'duplicates' => FEATURES_DUPLICATES_ALLOWED,
      // Views API integration does not include a default hook declaration as
      // it is not a proper default hook.
      // 'default_hook' => 'views_api',
    )
  );
}

/**
 * Implementation of hook_features_export_options().
 */
function views_features_export_options() {
  $enabled_views = array();
  $views = views_get_all_views();
  foreach ($views as $view) {
    if (!isset($views[$view->name]->disabled) || !$views[$view->name]->disabled) {
      $enabled_views[$view->name] = $view->name;
    }
  }
  ksort($enabled_views);
  return $enabled_views;
}

/**
 * Implementation of hook_features_export_render().
 */
function views_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $views = array();';
  $code[] = '';

  // Build views & add to export array
  foreach ($data as $view_name) {
    // Build the view
    $view = views_get_view($view_name, TRUE);
    if ($view) {
      $code[] = '  // Exported view: '. $view_name;
      $code[] = $view->export('  ');
      $code[] = '  $views[$view->name] = $view;';
      $code[] = '';
    }
  }
  $code[] = '  return $views;';
  $code = implode("\n", $code);
  return array('views_default_views' => $code);
}

/**
 * Implementation of hook_features_export().
 */
function views_features_export($data, &$export, $module_name = '') {
  // Build views & add to export array
  $map = features_get_default_map('views', 'name');
  $views = array();
  $conflicts = array();
  foreach ($data as $view_name) {
    if ($view = views_get_view($view_name, TRUE)) {
      // This view is provided by another module. Add it as a dependency or
      // display a conflict message if the View is overridden.
      if (isset($map[$view_name]) && $map[$view_name] !== $module_name) {
        if ($v = views_get_view($view_name)) {
          if ($v->type === 'Overridden') {
            $conflicts[$map[$view_name]] = $view_name;
          }
          elseif ($v->type === 'Default') {
            $export['dependencies'][$map[$view_name]] = $map[$view_name];
          }
        }
      }
      // Otherwise just add to exports
      else {
        $export['features']['views'][$view_name] = $view_name;
        $views[$view_name] = $view;
      }
    }
  }
  if (!empty($conflicts)) {
    $message = 'The following overridden view(s) are provided by other modules: !views. To resolve this problem either revert each view or clone each view so that modified versions can be exported with your feature.';
    $tokens = array('!views' => implode(', ', $conflicts));
    features_log(t($message, $tokens), 'warning');
  }

  // Only add Views API hook if there are actually views to export.
  if (!empty($export['features']['views'])) {
    $export['features']['views_api']['api:'. views_api_version()] = 'api:'. views_api_version();
    $export['dependencies']['views'] = 'views';
  }

  // Discover module dependencies
  // We need to find dependencies based on:
  // 1. handlers
  // 2. plugins (style plugins, display plugins)
  // 3. other... (e.g. imagecache display option for CCK imagefields) : (

  // Handlers
  $handlers = array('fields', 'filters', 'arguments', 'sort', 'relationships');
  $handler_dependencies = views_handler_dependencies();

  // Plugins
  // For now we only support dependency detection for a subset of all views plugins
  $plugins = array('display', 'style', 'row', 'access');
  $plugin_dependencies = views_plugin_dependencies();

  foreach ($views as $view) {
    foreach ($view->display as $display) {
      // Handlers
      foreach ($handlers as $handler) {
        if (isset($display->display_options[$handler])) {
          foreach ($display->display_options[$handler] as $item) {
            if ($item['table'] && isset($handler_dependencies[$item['table']])) {
              $module = $handler_dependencies[$item['table']];
              $export['dependencies'][$module] = $module;
            }
          }
        }
      }

      // Plugins
      foreach ($plugins as $plugin_type) {
        $plugin_name = '';
        switch ($plugin_type) {
          case 'display':
            if (isset($display->display_plugin)) {
              $plugin_name = $display->display_plugin;
            }
            break;
          case 'access':
            if (isset($display->display_options['access'], $display->display_options['access']['type']) && $display->display_options['access']['type'] != 'none') {
              $plugin_name = $display->display_options['access']['type'];
            }
            break;
          default:
            if (isset($display->display_options["{$plugin_type}_plugin"])) {
              $plugin_name = $display->display_options["{$plugin_type}_plugin"];
            }
            break;
        }
        if (!empty($plugin_name) && isset($plugin_dependencies[$plugin_type][$plugin_name])) {
          $module = $plugin_dependencies[$plugin_type][$plugin_name];
          $export['dependencies'][$module] = $module;
        }
      }
    }
  }
}

/**
 * Provides an array that maps hook_views_data() tables to modules.
 */
function views_handler_dependencies() {
  views_include_handlers();

  static $map;
  if (!isset($map)) {
    $map = array();
    foreach (module_implements('views_data') as $module) {
      if ($tables = module_invoke($module, 'views_data')) {
        foreach ($tables as $table => $info) {
          if (isset($info['table']) && (!isset($map[$table]) || $info['table']['group'])) {
            $map[$table] = $module;
          }
          else if (!isset($map[$table])) {
            $map[$table] = $module;
          }
        }
      }
    }
  }
  return $map;
}

/**
 * Provides an array that maps hook_views_plugins() to modules.
 */
function views_plugin_dependencies() {
  views_include_handlers();

  static $map;
  if (!isset($map)) {
    $map = array();
    foreach (module_implements('views_plugins') as $module) {
      $plugins = module_invoke($module, 'views_plugins');
      if (!empty($plugins)) {
        foreach ($plugins as $type => $items) {
          if (is_array($items)) {
            foreach (array_keys($items) as $plugin_name) {
              $map[$type][$plugin_name] = $module;
            }
          }
        }
      }
    }
  }
  return $map;
}

/**
 * Implementation of hook_features_revert().
 */
function views_features_revert($module) {
  if ($default_views = features_get_default('views', $module)) {
    foreach ($default_views as $default_view) {
      if ($current_view = views_get_view($default_view->name)) {
        $current_view->delete(FALSE);
      }
    }
    // Flush caches.
    cache_clear_all();
    menu_rebuild();
  }
}
