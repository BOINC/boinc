<?php

/**
 * Implementation of hook_features_api().
 */
function menu_features_api() {
  return array(
    'menu_custom' => array(
      'name' => t('Menus'),
      'default_hook' => 'menu_default_menu_custom',
      'feature_source' => TRUE,
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
    ),
    'menu_links' => array(
      'name' => t('Menu links'),
      'default_hook' => 'menu_default_menu_links',
      'feature_source' => TRUE,
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
    ),
    // DEPRECATED
    'menu' => array(
      'name' => t('Menu items'),
      'default_hook' => 'menu_default_items',
      'default_file' => FEATURES_DEFAULTS_INCLUDED,
      'feature_source' => FALSE,
    ),
  );
}

/**
 * Implementation of hook_features_export().
 * DEPRECATED: This implementation simply migrates deprecated `menu` items
 * to the `menu_links` type.
 */
function menu_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  foreach ($data as $path) {
    $pipe['menu_links'][] = "features:{$path}";
  }
  return $pipe;
}

/**
 * Implementation of hook_features_export_options().
 */
function menu_custom_features_export_options() {
  $options = array();
  $result = db_query("SELECT * FROM {menu_custom}");
  while ($row = db_fetch_array($result)) {
    $options[$row['menu_name']] = $row['title'];
  }
  return $options;
}

/**
 * Implementation of hook_features_export().
 */
function menu_custom_features_export($data, &$export, $module_name = '') {
  // Default hooks are provided by the feature module so we need to add
  // it as a dependency.
  $export['dependencies']['features'] = 'features';
  $export['dependencies']['menu'] = 'menu';

  // Collect a menu to module map
  $pipe = array();
  $map = features_get_default_map('menu_custom', 'menu_name');
  foreach ($data as $menu_name) {
    // If this menu is provided by a different module, add it as a dependency.
    if (isset($map[$menu_name]) && $map[$menu_name] != $module_name) {
      $export['dependencies'][$map[$menu_name]] = $map[$menu_name];
    }
    else {
      $export['features']['menu_custom'][$menu_name] = $menu_name;
    }
  }
  return $pipe;
}

/**
 * Implementation of hook_features_export_render()
 */
function menu_custom_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $menus = array();';
  $code[] = '';

  $translatables = array();
  foreach ($data as $menu_name) {
    $result = db_query("SELECT menu_name, title, description FROM {menu_custom} WHERE menu_name = '%s'", $menu_name);
    while ($row = db_fetch_array($result)) {
      $export = features_var_export($row, '  ');
      $code[] = "  // Exported menu: {$menu_name}";
      $code[] = "  \$menus['{$menu_name}'] = {$export};";
      $translatables[] = $row['title'];
      $translatables[] = $row['description'];
    }
  }
  if (!empty($translatables)) {
    $code[] = features_translatables_export($translatables, '  ');
  }

  $code[] = '';
  $code[] = '  return $menus;';
  $code = implode("\n", $code);
  return array('menu_default_menu_custom' => $code);
}

/**
 * Implementation of hook_features_revert().
 */
function menu_custom_features_revert($module) {
  menu_custom_features_rebuild($module);
}

/**
 * Implementation of hook_features_rebuild().
 */
function menu_custom_features_rebuild($module) {
  if ($defaults = features_get_default('menu_custom', $module)) {
    foreach ($defaults as $menu) {
      $existing = db_result(db_query("SELECT menu_name FROM {menu_custom} WHERE menu_name = '%s'", $menu['menu_name']));
      drupal_write_record('menu_custom', $menu, $existing ? array('menu_name') : array());
    }
  }
}

/**
 * Implementation of hook_features_export_options().
 */
function menu_links_features_export_options() {
  // We can't use the menu_tree_all_data function as it statically caches it's menu items
  // so if something calls the function before now without $menu_admin set to true we 
  // won't see all the links. Instead we reproduce the essential parts of the
  // function here.

  $menu_links = array();
  foreach (array_reverse(menu_get_menus()) as $menu_name => $menu_title) {
    $data = array();
    $cid = 'links:' . $menu_name . ':all-cid:0';
    $cache = cache_get($cid, 'cache_menu');
    if ($cache && isset($cache->data)) {
      // If the cache entry exists, it will just be the cid for the actual data.
      // This avoids duplication of large amounts of data.
      $cache = cache_get($cache->data, 'cache_menu');
      if ($cache && isset($cache->data)) {
        $data = $cache->data;
      }
    } else {
      $data['tree'] = menu_tree_data(db_query("
        SELECT m.load_functions, m.to_arg_functions, m.access_callback, m.access_arguments, m.page_callback, m.page_arguments, m.title, m.title_callback, m.title_arguments, m.type, m.description, ml.*
        FROM {menu_links} ml LEFT JOIN {menu_router} m ON m.path = ml.router_path
        WHERE ml.menu_name = '%s'
        ORDER BY p1 ASC, p2 ASC, p3 ASC, p4 ASC, p5 ASC, p6 ASC, p7 ASC, p8 ASC, p9 ASC", $menu_name));

      $data['node_links'] = array();
      menu_tree_collect_node_links($data['tree'], $data['node_links']);
      // May as well cache the data, if it is not already in the cache.
      $tree_cid = _menu_tree_cid($menu_name, $data);
      if (!cache_get($tree_cid, 'cache_menu')) {
        cache_set($tree_cid, $data, 'cache_menu');
      }
      // Cache the cid of the (shared) data using the menu and item-specific cid.
      cache_set($cid, $tree_cid, 'cache_menu');
    }
    
    $GLOBALS['menu_admin'] = TRUE;
    menu_tree_check_access($data['tree'], $data['node_links']);
    $GLOBALS['menu_admin'] = FALSE;
    
    _menu_links_features_export_options_recurse($data['tree'], $menu_name, '--', $menu_links);
  }
  return $menu_links;
}

function _menu_links_features_export_options_recurse($tree, $menu_name, $indent, &$menu_links) {
  // We don't want to use _menu_parents_recurse as that could potentially cut out some 
  // options as not being suitable for parents which we still want to export. Also we can
  // save on additional calls to the database.

  foreach ($tree as $data) {
    if ($data['link']['hidden'] >= 0) {
      $title = $indent . ' ' . truncate_utf8($data['link']['title'], 30, TRUE, FALSE);
      if ($data['link']['hidden']) {
        $title .= ' (' . t('disabled') . ')';
      }
      $menu_links[menu_links_features_identifier($data['link'])] = "{$menu_name}: {$title}";
      if ($data['below']) {
        _menu_links_features_export_options_recurse($data['below'], $menu_name, $indent . '--', $menu_links);
      }
    }
  } 
}

/**
 * Callback for generating the menu link exportable identifier.
 */
function menu_links_features_identifier($link) {
  return isset($link['menu_name'], $link['link_path']) ? "{$link['menu_name']}:{$link['link_path']}" : FALSE;
}

/**
 * Implementation of hook_features_export().
 */
function menu_links_features_export($data, &$export, $module_name = '') {
  // Default hooks are provided by the feature module so we need to add
  // it as a dependency.
  $export['dependencies']['features'] = 'features';
  $export['dependencies']['menu'] = 'menu';

  // Collect a link to module map
  $pipe = array();
  $map = features_get_default_map('menu_links', 'menu_links_features_identifier');
  foreach ($data as $identifier) {
    if ($link = features_menu_link_load($identifier)) {
      // If this link is provided by a different module, add it as a dependency.
      if (isset($map[$identifier]) && $map[$identifier] != $module_name) {
        $export['dependencies'][$map[$identifier]] = $map[$identifier];
      }
      else {
        $export['features']['menu_links'][$identifier] = $identifier;
      }
      // For now, exclude a variety of common menus from automatic export.
      // They may still be explicitly included in a Feature if the builder
      // chooses to do so.
      if (!in_array($link['menu_name'], array('features', 'primary-links', 'secondary-links', 'navigation', 'admin', 'devel'))) {
        $pipe['menu_custom'][] = $link['menu_name'];
      }
    }
  }
  return $pipe;
}

/**
 * Implementation of hook_features_export_render()
 */
function menu_links_features_export_render($module, $data) {
  $code = array();
  $code[] = '  $menu_links = array();';
  $code[] = '';

  $translatables = array();
  foreach ($data as $identifier) {
    if ($link = features_menu_link_load($identifier)) {
      // Replace plid with a parent path.
      if (!empty($link['plid']) && $parent = menu_link_load($link['plid'])) {
        $link['parent_path'] = $parent['link_path'];
      }
      unset($link['plid']);
      unset($link['mlid']);

      $code[] = "  // Exported menu link: {$identifier}";
      $code[] = "  \$menu_links['{$identifier}'] = ". features_var_export($link, '  ') .";";
      $translatables[] = $link['link_title'];
    }
  }
  if (!empty($translatables)) {
    $code[] = features_translatables_export($translatables, '  ');
  }

  $code[] = '';
  $code[] = '  return $menu_links;';
  $code = implode("\n", $code);
  return array('menu_default_menu_links' => $code);
}

/**
 * Implementation of hook_features_revert().
 */
function menu_links_features_revert($module) {
  menu_links_features_rebuild($module);
}

/**
 * Implementation of hook_features_rebuild().
 */
function menu_links_features_rebuild($module) {
  if ($menu_links = features_get_default('menu_links', $module)) {
    menu_links_features_rebuild_ordered($menu_links);
  }
}

/**
 * Generate a depth tree of all menu links.
 */
function menu_links_features_rebuild_ordered($menu_links, $reset = FALSE) {
  static $ordered;
  static $all_links;
  if (!isset($ordered) || $reset) {
    $ordered = array();
    $unordered = features_get_default('menu_links');

    // Order all links by depth.
    if ($unordered) {
      do {
        $current = count($unordered);
        foreach ($unordered as $key => $link) {
          $identifier = menu_links_features_identifier($link);
          $parent = isset($link['parent_path']) ? "{$link['menu_name']}:{$link['parent_path']}" : '';
          if (empty($parent) || (!empty($parent) && !isset($unordered[$parent]) && !isset($ordered[$parent]))) {
            $ordered[$identifier] = 0;
            $all_links[$identifier] = $link;
            unset($unordered[$key]);
          }
          elseif (isset($ordered[$parent])) {
            $ordered[$identifier] = $ordered[$parent] + 1;
            $all_links[$identifier] = $link;
            unset($unordered[$key]);
          }
        }
      } while (count($unordered) < $current);
    }
    asort($ordered);
  }

  // Ensure any default menu items that do not exist are created.
  foreach (array_keys($ordered) as $identifier) {
    $link = $all_links[$identifier];
    $existing = features_menu_link_load($identifier);
    if (!$existing || in_array($link, $menu_links)) {
      // Retrieve the mlid if this is an existing item.
      if ($existing) {
        $link['mlid'] = $existing['mlid'];
      }
      // Retrieve the plid for a parent link.
      if (!empty($link['parent_path']) && $parent = features_menu_link_load("{$link['menu_name']}:{$link['parent_path']}")) {
        $link['plid'] = $parent['mlid'];
      }
      else {
        $link['plid'] = 0;
      }
      menu_link_save($link);
    }
  }
}

/**
 * Load a menu link by its menu_name:link_path identifier.
 */
function features_menu_link_load($identifier) {
  list($menu_name, $link_path) = explode(':', $identifier, 2);
  $result = db_query("SELECT menu_name, mlid, plid, link_path, router_path, link_title, options, module, hidden, external, has_children, expanded, weight FROM {menu_links} WHERE menu_name = '%s' AND link_path = '%s'", $menu_name, $link_path);
  while ($link = db_fetch_array($result)) {
    $link['options'] = unserialize($link['options']);
    return $link;
  }
  return FALSE;
}
