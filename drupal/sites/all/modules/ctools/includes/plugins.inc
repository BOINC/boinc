<?php
// $Id: plugins.inc,v 1.18.2.30 2010/08/25 20:22:22 merlinofchaos Exp $

/**
 * @file
 *
 * Contains routines to organize and load plugins. It allows a special
 * variation of the hook system so that plugins can be kept in separate
 * .inc files, and can be either loaded all at once or loaded only when
 * necessary.
 */

/**
 * Get an array of information about modules that support an API.
 *
 * This will ask each module if they support the given API, and if they do
 * it will return an array of information about the modules that do.
 *
 * This function invokes hook_ctools_api. This invocation is statically
 * cached, so feel free to call it as often per page run as you like, it
 * will cost very little.
 *
 * This function can be used as an alternative to module_implements and can
 * thus be used to find a precise list of modules that not only support
 * a given hook (aka 'api') but also restrict to only modules that use
 * the given version. This will allow multiple modules moving at different
 * paces to still be able to work together and, in the event of a mismatch,
 * either fall back to older behaviors or simply cease loading, which is
 * still better than a crash.
 *
 * @param $owner
 *   The name of the module that controls the API.
 * @param $api
 *   The name of the api. The api name forms the file name:
 *   $module.$api.inc
 * @param $minimum_version
 *   The lowest version API that is compatible with this one. If a module
 *   reports its API as older than this, its files will not be loaded. This
 *   should never change during operation.
 * @param $current_version
 *   The current version of the api. If a module reports its minimum API as
 *   higher than this, its files will not be loaded. This should never change
 *   during operation.
 *
 * @return
 *   An array of API information, keyed by module. Each module's information will
 *   contain:
 *   - 'version': The version of the API required by the module. The module
 *     should use the lowest number it can support so that the widest range
 *     of supported versions can be used.
 *   - 'path': If not provided, this will be the module's path. This is
 *     where the module will store any subsidiary files. This differs from
 *     plugin paths which are figured separately.
 *
 *   APIs can request any other information to be placed here that they might
 *   need. This should be in the documentation for that particular API.
 */
function ctools_plugin_api_info($owner, $api, $minimum_version, $current_version) {
  $cache = &ctools_static(__FUNCTION__, array());
  if (!isset($cache[$owner][$api])) {
    $cache[$owner][$api] = array();
    foreach (module_implements('ctools_plugin_api') as $module) {
      $function = $module . '_ctools_plugin_api';
      $info = $function($owner, $api);
      if (!isset($info['version'])) {
        continue;
      }

      // Only process if version is between minimum and current, inclusive.
      if ($info['version'] >= $minimum_version && $info['version'] <= $current_version) {
        if (!isset($info['path'])) {
          $info['path'] = drupal_get_path('module', $module);
        }
        $cache[$owner][$api][$module] = $info;
      }
    }

    // And allow themes to implement these as well.
    $themes = _ctools_list_themes();
    foreach ($themes as $name => $theme) {
      if (!empty($theme->info['api'][$owner][$api])) {
        $info = $theme->info['api'][$owner][$api];
        if (!isset($info['version'])) {
          continue;
        }

        // Only process if version is between minimum and current, inclusive.
        if ($info['version'] >= $minimum_version && $info['version'] <= $current_version) {
          if (!isset($info['path'])) {
            $info['path'] = '';
          }
          // Because themes can't easily specify full path, we add it here
          // even though we do not for modules:
          $info['path'] = drupal_get_path('theme', $name) . '/' . $info['path'];
          $cache[$owner][$api][$name] = $info;
        }
      }
    }
  }

  return $cache[$owner][$api];
}

/**
 * Load a group of API files.
 *
 * This will ask each module if they support the given API, and if they do
 * it will load the specified file name. The API and the file name
 * coincide by design.
 *
 * @param $owner
 *   The name of the module that controls the API.
 * @param $api
 *   The name of the api. The api name forms the file name:
 *   $module.$api.inc, though this can be overridden by the module's response.
 * @param $minimum_version
 *   The lowest version API that is compatible with this one. If a module
 *   reports its API as older than this, its files will not be loaded. This
 *   should never change during operation.
 * @param $current_version
 *   The current version of the api. If a module reports its minimum API as
 *   higher than this, its files will not be loaded. This should never change
 *   during operation.
 *
 * @return
 *   The API information, in case you need it.
 */
function ctools_plugin_api_include($owner, $api, $minimum_version, $current_version) {
  static $already_done = array();

  $info = ctools_plugin_api_info($owner, $api, $minimum_version, $current_version);
  if (!isset($already_done[$owner][$api])) {
    foreach ($info as $module => $plugin_info) {
      if (!isset($plugin_info['file'])) {
        $plugin_info['file'] = "$module.$api.inc";
      }
      if (file_exists("./$plugin_info[path]/$plugin_info[file]")) {
        $plugin_info[$module]['included'] = TRUE;
        require_once "./$plugin_info[path]/$plugin_info[file]";
      }
      $info[$module] = $plugin_info;
    }
    $already_done[$owner][$api] = TRUE;
  }

  return $info;
}

/**
 * Fetch a group of plugins by name.
 *
 * @param $module
 *   The name of the module that utilizes this plugin system. It will be
 *   used to call hook_ctools_plugin_$plugin() to get more data about the plugin.
 * @param $type
 *   The type identifier of the plugin.
 * @param $id
 *   If specified, return only information about plugin with this identifier.
 *   The system will do its utmost to load only plugins with this id.
 *
 * @return
 *   An array of information arrays about the plugins received. The contents
 *   of the array are specific to the plugin.
 */
function ctools_get_plugins($module, $type, $id = NULL) {
  // Store local caches of plugins and plugin info so we don't have to do full
  // lookups everytime.
  $info = &ctools_static('ctools_plugin_info', array());
  $plugins = &ctools_static('ctools_plugins', array());

  // Attempt to shortcut this whole piece of code if we already have
  // the requested plugin:
  if ($id && isset($plugins[$module][$type]) && array_key_exists($id, $plugins[$module][$type])) {
    return $plugins[$module][$type][$id];
  }

  // Store the status of plugin loading. If a module plugin type pair is true,
  // then it is fully loaded and no searching or setup needs to be done.
  $setup = &ctools_static('ctools_plugin_setup', array());

  // Request metadata/defaults for this plugin from the declaring module. This
  // is done once per page request, upon a request being made for that plugin.
  if (!isset($info[$module][$type])) {
    $info[$module][$type] = ctools_plugin_get_info($module, $type);
    // Also, initialize the local plugin cache.
    $plugins[$module][$type] = array();
  }

  // We assume we don't need to build a cache.
  $build_cache = FALSE;

  // If the plugin info says this can be cached, check cache first.
  if ($info[$module][$type]['cache'] && empty($setup[$module][$type])) {
    $cache = cache_get("plugins:$module:$type", $info[$module][$type]['cache table']);

    if (!empty($cache->data)) {
      // Cache load succeeded so use the cached plugin list.
      $plugins[$module][$type]   = $cache->data;
      // Set $setup to true so we know things where loaded.
      $setup[$module][$type]     = TRUE;
    }
    else {
      // Cache load failed so store that we need to build and write the cache.
      $build_cache = TRUE;
    }
  }

  // Always load all hooks if we need them. Note we only need them now if the
  // plugin asks for them. We can assume that if we have plugins we've already
  // called the global hook.
  if (!empty($info[$module][$type]['use hooks']) && empty($plugins[$module][$type])) {
    $plugins[$module][$type] = ctools_plugin_load_hooks($info[$module][$type]);
  }

  // Then see if we should load all files. We only do this if we
  // want a list of all plugins or there was a cache miss.
  if (empty($setup[$module][$type]) && ($build_cache || !$id)) {
    $setup[$module][$type] = TRUE;
    $plugins[$module][$type] = array_merge($plugins[$module][$type], ctools_plugin_load_includes($info[$module][$type]));
    // If the plugin can have child plugins, and we're loading all plugins,
    // go through the list of plugins we have and find child plugins.
    if (!$id && !empty($info[$module][$type]['child plugins'])) {
      // If a plugin supports children, go through each plugin and ask.
      $temp = array();
      foreach ($plugins[$module][$type] as $name => $plugin) {
        if (!empty($plugin['get children']) && function_exists($plugin['get children'])) {
          $temp = array_merge($plugin['get children']($plugin, $name), $temp);
        }
        else {
          $temp[$name] = $plugin;
        }
      }
      $plugins[$module][$type] = $temp;
    }
  }


  // If we were told earlier that this is cacheable and the cache was
  // empty, give something back.
  if ($build_cache) {
    cache_set("plugins:$module:$type", $plugins[$module][$type], $info[$module][$type]['cache table']);
  }

  // If no id was requested, we are finished here:
  if (!$id) {
    // Use array_filter because looking for unknown plugins could cause NULL
    // entries to appear in the list later.
    return array_filter($plugins[$module][$type]);
  }

  // Check to see if we need to look for the file
  if (!array_key_exists($id, $plugins[$module][$type])) {
    // If we can have child plugins, check to see if the plugin name is in the
    // format of parent:child and break it up if it is.
    if (!empty($info[$module][$type]['child plugins']) && strpos($id, ':') !== FALSE) {
      list($parent, $child) = explode(':', $id, 2);
    }
    else {
      $parent = $id;
    }

    if (!array_key_exists($parent, $plugins[$module][$type])) {
      $result = ctools_plugin_load_includes($info[$module][$type], $parent);
      // Set to either what was returned or NULL.
      $plugins[$module][$type][$parent] = isset($result[$parent]) ? $result[$parent] : NULL;
    }

    // If we are looking for a child, and have the parent, ask the parent for the child.
    if (!empty($child) && !empty($plugins[$module][$type][$parent]) && function_exists($plugins[$module][$type][$parent]['get child'])) {
      $plugins[$module][$type][$id] = $plugins[$module][$type][$parent]['get child']($plugins[$module][$type][$parent], $parent, $child);
    }
  }

  // At this point we should either have the plugin, or a NULL.
  return $plugins[$module][$type][$id];
}

/**
 * Load plugins from a directory.
 *
 * @param $info
 *   The plugin info as returned by ctools_plugin_get_info()
 * @param $file
 *   The file to load if we're looking for just one particular plugin.
 *
 * @return
 *   An array of information created for this plugin.
 */
function ctools_plugin_load_includes($info, $filename = NULL) {
  // Keep a static array so we don't hit drupal_system_listing more than necessary.
  $all_files = &ctools_static(__FUNCTION__, array());

  // If we're being asked for all plugins of a type, skip any caching
  // we may have done because this is an admin task and it's ok to
  // spend the extra time.
  if (!isset($filename)) {
    $all_files[$info['module']][$info['type']] = NULL;
  }

  if (!isset($all_files[$info['module']][$info['type']])) {
    // If a filename was set, we will try to load our list of files from
    // cache. This is considered normal operation and we try to reduce
    // the time spent finding files.
    if (isset($filename)) {
      $cache = cache_get("ctools_plugin_files:$info[module]:$info[type]");
      if ($cache) {
        $all_files[$info['module']][$info['type']] = $cache->data;
      }
    }

    if (!isset($all_files[$info['module']][$info['type']])) {
      $all_files[$info['module']][$info['type']] = array();
      // Load all our plugins.
      $directories = ctools_plugin_get_directories($info);
      $extension = empty($info['info file']) ? $info['extension'] : 'info';

      foreach ($directories as $module => $path) {
        $all_files[$info['module']][$info['type']][$module] = drupal_system_listing('\.' . $extension . '$', $path, 'name', 0);
      }

      cache_set("ctools_plugin_files:$info[module]:$info[type]", $all_files[$info['module']][$info['type']]);
    }
  }
  $file_list = $all_files[$info['module']][$info['type']];
  $plugins = array();

  // Iterate through all the plugin .inc files, load them and process the hook
  // that should now be available.
  foreach (array_filter($file_list) as $module => $files) {
    if ($filename) {
      $files = isset($files[$filename]) ? array($filename => $files[$filename]) : array();
    }
    foreach ($files as $file) {
      if (!empty($info['info file'])) {
        // Parse a .info file
        $result = ctools_plugin_process_info($info, $module, $file);
      }
      else {
        // Parse a hook.
        $plugin = NULL; // ensure that we don't have something leftover from earlier.
        include_once './' . $file->filename;
        // .inc files have a special format for the hook identifier.
        // For example, 'foo.inc' in the module 'mogul' using the plugin
        // whose hook is named 'borg_type' should have a function named (deep breath)
        // mogul_foo_borg_type()

        // If, however, the .inc file set the quasi-global $plugin array, we
        // can use that and not even call a function. Set the $identifier
        // appropriately and ctools_plugin_process() will handle it.
        $identifier = isset($plugin) ? $plugin : $module . '_' . $file->name;
        $result = ctools_plugin_process($info, $module, $identifier, dirname($file->filename), basename($file->filename), $file->name);
      }
      if (is_array($result)) {
        $plugins = array_merge($plugins, $result);
      }
    }
  }
  return $plugins;
}

/**
 * Get a list of directories to search for plugins of the given type.
 *
 * This utilizes hook_ctools_plugin_directory() to determine a complete list of
 * directories. Only modules that implement this hook and return a string
 * value will have their directories included.
 *
 * @param $info
 *   The $info array for the plugin as returned by ctools_plugin_get_info().
 *
 * @return array $directories
 *   An array of directories to search.
 */
function ctools_plugin_get_directories($info) {
  $directories = array();

  foreach (module_implements('ctools_plugin_directory') as $module) {
    $function = $module . '_ctools_plugin_directory';
    $result = $function($info['module'], $info['type']);
    if ($result && is_string($result)) {
      $directories[$module] = drupal_get_path('module', $module) . '/' . $result;
    }
  }

  if (!empty($info['load themes'])) {
    $themes = _ctools_list_themes();
    foreach ($themes as $name => $theme) {
      if (!empty($theme->info['plugins'][$info['module']][$info['type']])) {
        $directories[$name] = drupal_get_path('theme', $name) . '/' . $theme->info['plugins'][$info['module']][$info['type']];
      }
    }
  }
  return $directories;
}

/**
 * Helper function to build a ctools-friendly list of themes capable of
 * providing plugins.
 *
 * @return array $themes
 *   A list of themes that can act as plugin providers, sorted parent-first with
 *   the active theme placed last.
 */
function _ctools_list_themes() {
  static $themes;
  if (is_null($themes)) {
    $current = variable_get('theme_default', FALSE);
    $themes = $active = array();
    $all_themes = list_themes();
    foreach ($all_themes as $name => $theme) {
      // Only search from active themes
      if (empty($theme->status) && $theme->name != $current) {
        continue;
      }
      $active[$name] = $theme;
      // Prior to drupal 6.14, $theme->base_themes does not exist. Build it.
      if (!isset($theme->base_themes) && !empty($theme->base_theme)) {
        $active[$name]->base_themes = ctools_find_base_themes($all_themes, $name);
      }
    }

    // Construct a parent-first list of all themes
    foreach ($active as $name => $theme) {
      $base_themes = isset($theme->base_themes) ? $theme->base_themes : array();
      $themes = array_merge($themes, $base_themes, array($name => $theme->info['name']));
    }
    // Put the actual theme info objects into the array
    foreach (array_keys($themes) as $name) {
      $themes[$name] = $all_themes[$name];
    }

    // Make sure the current default theme always gets the last word
    if ($current_key = array_search($current, array_keys($themes))) {
      $themes += array_splice($themes, $current_key, 1);
    }
  }
  return $themes;
}


/**
 * Find all the base themes for the specified theme.
 *
 * Themes can inherit templates and function implementations from earlier themes.
 *
 * NOTE: this is a verbatim copy of system_find_base_themes(), which was not
 * implemented until 6.14. It is included here only as a fallback for outdated
 * versions of drupal core.
 *
 * @param $themes
 *   An array of available themes.
 * @param $key
 *   The name of the theme whose base we are looking for.
 * @param $used_keys
 *   A recursion parameter preventing endless loops.
 * @return
 *   Returns an array of all of the theme's ancestors; the first element's value
 *   will be NULL if an error occurred.
 */
function ctools_find_base_themes($themes, $key, $used_keys = array()) {
  $base_key = $themes[$key]->info['base theme'];
  // Does the base theme exist?
  if (!isset($themes[$base_key])) {
    return array($base_key => NULL);
  }

  $current_base_theme = array($base_key => $themes[$base_key]->info['name']);

  // Is the base theme itself a child of another theme?
  if (isset($themes[$base_key]->info['base theme'])) {
    // Do we already know the base themes of this theme?
    if (isset($themes[$base_key]->base_themes)) {
      return $themes[$base_key]->base_themes + $current_base_theme;
    }
    // Prevent loops.
    if (!empty($used_keys[$base_key])) {
      return array($base_key => NULL);
    }
    $used_keys[$base_key] = TRUE;
    return ctools_find_base_themes($themes, $base_key, $used_keys) + $current_base_theme;
  }
  // If we get here, then this is our parent theme.
  return $current_base_theme;
}


/**
 * Load plugin info for the provided hook; this is handled separately from
 * plugins from files.
 *
 * @param $info
 *   The info array about the plugin as created by ctools_plugin_get_info()
 *
 * @return
 *   An array of info supplied by any hook implementations.
 */
function ctools_plugin_load_hooks($info) {
  $hooks = array();
  foreach (module_implements($info['hook']) as $module) {
    $result = ctools_plugin_process($info, $module, $module, drupal_get_path('module', $module));
    if (is_array($result)) {
      $hooks = array_merge($hooks, $result);
    }
  }
  return $hooks;
}

/**
 * Process a single hook implementation of a ctools plugin.
 *
 * @param $info
 *   The $info array about the plugin as returned by ctools_plugin_get_info()
 * @param $module
 *   The module that implements the plugin being processed.
 * @param $identifier
 *   The plugin identifier, which is used to create the name of the hook
 *   function being called.
 * @param $path
 *   The path where files utilized by this plugin will be found.
 * @param $file
 *   The file that was loaded for this plugin, if it exists.
 * @param $base
 *   The base plugin name to use. If a file was loaded for the plugin, this
 *   is the plugin to assume must be present. This is used to automatically
 *   translate the array to make the syntax more friendly to plugin
 *   implementors.
 */
function ctools_plugin_process($info, $module, $identifier, $path, $file = NULL, $base = NULL) {
  if (is_array($identifier)) {
    $result = $identifier;
  }
  else {
    $function = $identifier . '_' . $info['hook'];
    if (!function_exists($function)) {
      return NULL;
    }
    $result = $function();
    if (!isset($result) || !is_array($result)) {
      return NULL;
    }
  }

  // Automatically convert to the proper format that lets plugin implementations
  // not nest arrays as deeply as they used to, but still support the older
  // format where they do:
  if ($base && (!isset($result[$base]) || !is_array($result[$base]))) {
    $result = array($base => $result);
  }

  return _ctools_process_data($result, $info, $module, $path, $file);
}

/**
 * Fill in default values and run hooks for data loaded for one or
 * more plugins.
 */
function _ctools_process_data($result, $info, $module, $path, $file) {
  // Fill in global defaults.
  foreach ($result as $name => $plugin) {
    $result[$name] += array(
      'module' => $module,
      'name' => $name,
      'path' => $path,
      'file' => $file,
      'plugin module' => $info['module'],
      'plugin type' => $info['type'],
    );

    // Fill in plugin-specific defaults, if they exist.
    if (!empty($info['defaults'])) {
      if (is_array($info['defaults'])) {
        $result[$name] += $info['defaults'];
      }
      // FIXME This callback-based approach for the 'defaults' key is entirely
      // redundant with the 'process' callback. Consequently, that approach is
      // DEPRECATED in favor using a 'process' callback. In the next major
      // version, 'defaults' callbacks will be removed entirely; only the array
      // addition 'defaults' approach will be allowed.
      else if (function_exists($info['defaults'])) {
        $info['defaults']($info, $result[$name]);
      }
    }

    // Allow the plugin owner to do additional processing.
    if (!empty($info['process']) && function_exists($info['process'])) {
      $info['process']($result[$name], $info);
    }
  }
  return $result;
}


/**
 * Process an info file for plugin information, rather than a hook.
 */
function ctools_plugin_process_info($info, $module, $file) {
  $result = drupal_parse_info_file($file->filename);
  if ($result) {
    $result = array($file->name => $result);
    return _ctools_process_data($result, $info, $module, dirname($file->filename), basename($file->filename));
  }
}

/**
 * Ask a module for info about a particular plugin type.
 */
function ctools_plugin_get_info($module, $type) {
  $info = array();
  $function = $module . '_ctools_plugin_' . $type;
  if (function_exists($function)) {
    $info = $function();
  }

  // Apply defaults. Array addition will not overwrite pre-existing keys.
  $info += array(
    'module' => $module,
    'type' => $type,
    'cache' => FALSE,
    'cache table' => 'cache',
    'use hooks' => TRUE, // TODO will default to FALSE in next major version
    'defaults' => array(),
    'process' => '',
    'extension' => 'inc',
    'info file' => FALSE,
    'hook' => $module . '_' . $type,
    'load themes' => FALSE,
  );

  return $info;
}

/**
 * Get a function from a plugin, if it exists. If the plugin is not already
 * loaded, try ctools_plugin_load_function() instead.
 *
 * @param $plugin_definition
 *   The loaded plugin type.
 * @param $function_name
 *   The identifier of the function. For example, 'settings form'.
 *
 * @return
 *   The actual name of the function to call, or NULL if the function
 *   does not exist.
 */
function ctools_plugin_get_function($plugin_definition, $function_name) {
  // If cached the .inc file may not have been loaded. require_once is quite safe
  // and fast so it's okay to keep calling it.
  if (isset($plugin_definition['file'])) {
    // Plugins that are loaded from info files have the info file as
    // $plugin['file'].  Don't try to run those.
    $info = ctools_plugin_get_info($plugin_definition['plugin module'], $plugin_definition['plugin type']);
    if (empty($info['info file'])) {
      require_once './' . $plugin_definition['path'] . '/' . $plugin_definition['file'];
    }
  }

  if (!isset($plugin_definition[$function_name])) {
    return;
  }

  if (is_array($plugin_definition[$function_name]) && isset($plugin_definition[$function_name]['function'])) {
    $function = $plugin_definition[$function_name]['function'];
    if (isset($plugin_definition[$function_name]['file'])) {
      $file = $plugin_definition[$function_name]['file'];
      if (isset($plugin_definition[$function_name]['path'])) {
        $file = $plugin_definition[$function_name]['path'] . '/' . $file;
      }
      require_once './' . $file;
    }
  }
  else {
    $function = $plugin_definition[$function_name];
  }

  if (function_exists($function)) {
    return $function;
  }
}

/**
 * Load a plugin and get a function name from it, returning success only
 * if the function exists.
 *
 * @param $module
 *   The module that owns the plugin type.
 * @param $type
 *   The type of plugin.
 * @param $id
 *   The id of the specific plugin to load.
 * @param $function_name
 *   The identifier of the function. For example, 'settings form'.
 *
 * @return
 *   The actual name of the function to call, or NULL if the function
 *   does not exist.
 */
function ctools_plugin_load_function($module, $type, $id, $function_name) {
  $plugin = ctools_get_plugins($module, $type, $id);
  return ctools_plugin_get_function($plugin, $function_name);
}

/**
 * Get a class from a plugin, if it exists. If the plugin is not already
 * loaded, try ctools_plugin_load_class() instead.
 *
 * @param $plugin_definition
 *   The loaded plugin type.
 * @param $class_name
 *   The identifier of the class. For example, 'handler'.
 * @param $abstract
 *   If true, will return abstract classes. Otherwise, parents will be included but nothing will be returned.
 *
 * @return
 *   The actual name of the class to call, or NULL if the class does not exist.
 */
function ctools_plugin_get_class($plugin_definition, $class_name, $abstract = FALSE) {
  // If cached the .inc file may not have been loaded. require_once is quite safe
  // and fast so it's okay to keep calling it.
  if (isset($plugin_definition['file'])) {
    // Plugins that are loaded from info files have the info file as
    // $plugin['file'].  Don't try to run those.
    $info = ctools_plugin_get_info($plugin_definition['plugin module'], $plugin_definition['plugin type']);
    if (empty($info['info file'])) {
      require_once './' . $plugin_definition['path'] . '/' . $plugin_definition['file'];
    }
  }

  if (!isset($plugin_definition[$class_name])) {
    return;
  }

  if (is_array($plugin_definition[$class_name]) && isset($plugin_definition[$class_name]['class'])) {
    if (isset($plugin_definition[$class_name]['parent'])) {
      // Make sure parents are included.
      // TODO parent-loading needs to be better documented; the 'parent' designated
      // on the plugin actually corresponds not to the name of the parent CLASS,
      // but the name of the parent PLUGIN (and it then loads loads whatever
      // class is in the same $class_name slot). Initially unintuitive.
      ctools_plugin_load_class($plugin_definition['plugin module'], $plugin_definition['plugin type'], $plugin_definition[$class_name]['parent'], $class_name);
    }
    $class = $plugin_definition[$class_name]['class'];
    if (isset($plugin_definition[$class_name]['file'])) {
      $file = $plugin_definition[$class_name]['file'];
      if (isset($plugin_definition[$class_name]['path'])) {
        $file = $plugin_definition[$class_name]['path'] . '/' . $file;
      }
      require_once './' . $file;
    }
  }
  else {
    $class = $plugin_definition[$class_name];
  }

  // If we didn't explicitly include a file above, try autoloading a file
  // based on the class' name.
  if (!isset($file) && file_exists($plugin_definition['path'] . "/$class.class.php")) {
    require_once $plugin_definition['path'] . "/$class.class.php";
  }

  if (class_exists($class) &&
    (empty($plugin_definition[$class_name]['abstract']) || $abstract)) {
    return $class;
  }
}

/**
 * Load a plugin and get a class name from it, returning success only if the
 * class exists.
 *
 * @param $module
 *   The module that owns the plugin type.
 * @param $type
 *   The type of plugin.
 * @param $id
 *   The id of the specific plugin to load.
 * @param $class_name
 *   The identifier of the class. For example, 'handler'.
 * @param $abstract
 *   If true, will tell ctools_plugin_get_class to allow the return of abstract classes.
 *
 * @return
 *   The actual name of the class to call, or NULL if the class does not exist.
 */
function ctools_plugin_load_class($module, $type, $id, $class_name, $abstract = FALSE) {
  $plugin = ctools_get_plugins($module, $type, $id);
  return ctools_plugin_get_class($plugin, $class_name, $abstract);
}

/**
 * Sort callback for sorting plugins naturally.
 *
 * Sort first by weight, then by title.
 */
function ctools_plugin_sort($a, $b) {
  if (is_object($a)) {
    $a = (array) $a;
  }
  if (is_object($b)) {
    $b = (array) $b;
  }

  if (empty($a['weight'])) {
    $a['weight'] = 0;
  }

  if (empty($b['weight'])) {
    $b['weight'] = 0;
  }

  if ($a['weight'] == $b['weight']) {
    return strnatcmp(strtolower($a['title']), strtolower($b['title']));
  }
  return ($a['weight'] < $b['weight']) ? -1 : 1;
}
