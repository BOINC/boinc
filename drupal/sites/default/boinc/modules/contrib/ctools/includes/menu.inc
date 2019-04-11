<?php

/**
 * @file
 * General menu helper functions.
 *
 * The menu system was completely revamped in Drupal 6; as such it is not as
 * mature as some other systems and is missing some API functions. This
 * file helps smooth some edges out.
 */

/**
 * Dynamically add a tab to the current path.
 *
 * This function allows you to dynamically add tabs to the current path.
 * There are several important considerations to this:
 *
 * - First, Drupal doesn't really allow this. CTools lets this happen by
 *   overriding the theme function that displays the tabs. That means that
 *   custom themes which do not use CTools functions will not get the new
 *   tabs. You can provide instructions to your users about how to deal with
 *   this, but you should be prepared for some users not getting the new tabs
 *   and not knowing why.
 * - Second, if there is only 1 tab, Drupal will not show it. Therefore, if
 *   you are only adding one tab, you should find a way to make sure there is
 *   already tab, or instead add 2.
 * - Third, the caller is responsible for providing access control to these
 *   links.
 *
 * @param $link
 *   An array describing this link. It must contain:
 *   - 'title': The printed title of the link.
 *   - 'href': The path of the link. This is an argument to l() so it has all
 *     of those features and limitations.
 *   - 'options': Any options that go to l, including query, fragment and html
 *     options necessary.
 *   - 'weight': The weight to use in ordering the tabs.
 *   - 'type': Optional. If set to MENU_DEFAULT_LOCAL_TASK this can be used to
 *     add a fake 'default' local task, which is useful if you have to add
 *     tabs to a page that has noen.
 */
function ctools_menu_add_tab($link = NULL) {
  static $links = array();
  if (isset($link)) {
    $links[$link['href']] = $link;
  }

  return $links;
}

/**
 * CTools replacement for menu_get_menu_trail that allows us to take apart
 * the menu trail if necessary.
 */
function ctools_get_menu_trail($path = NULL) {
  $trail   = array();
  $trail[] = array('title' => t('Home'), 'href' => '<front>', 'localized_options' => array(), 'type' => 0);
  $item    = menu_get_item($path);

  // Check whether the current item is a local task (displayed as a tab).
  if ($item['tab_parent']) {
    // The title of a local task is used for the tab, never the page title.
    // Thus, replace it with the item corresponding to the root path to get
    // the relevant href and title.  For example, the menu item corresponding
    // to 'admin' is used when on the 'By module' tab at 'admin/by-module'.
    $parts = explode('/', $item['tab_root']);
    $args = arg();
    // Replace wildcards in the root path using the current path.
    foreach ($parts as $index => $part) {
      if ($part == '%') {
        $parts[$index] = $args[$index];
      }
    }
    // Retrieve the menu item using the root path after wildcard replacement.
    $root_item = menu_get_item(implode('/', $parts));
    if ($root_item && $root_item['access']) {
      $item = $root_item;
    }
  }

  $tree = ctools_menu_tree_page_data($item, menu_get_active_menu_name());
  list($key, $curr) = each($tree);

  while ($curr) {
    // Terminate the loop when we find the current path in the active trail.
    if ($curr['link']['href'] == $item['href']) {
      $trail[] = $curr['link'];
      $curr = FALSE;
    }
    else {
      // Add the link if it's in the active trail, then move to the link below.
      if ($curr['link']['in_active_trail']) {
        $trail[] = $curr['link'];
        $tree = $curr['below'] ? $curr['below'] : array();
      }
      list($key, $curr) = each($tree);
    }
  }
  // Make sure the current page is in the trail (needed for the page title),
  // but exclude tabs and the front page.
  $last = count($trail) - 1;
  if ($trail[$last]['href'] != $item['href'] && !(bool)($item['type'] & MENU_IS_LOCAL_TASK) && !drupal_is_front_page()) {
    $trail[] = $item;
  }

  return $trail;
}

/**
 * Get the data structure representing a named menu tree, based on the current page.
 *
 * The tree order is maintained by storing each parent in an individual
 * field, see http://drupal.org/node/141866 for more.
 *
 * @param $menu_name
 *   The named menu links to return
 *
 * @return
 *   An array of menu links, in the order they should be rendered. The array
 *   is a list of associative arrays -- these have two keys, link and below.
 *   link is a menu item, ready for theming as a link. Below represents the
 *   submenu below the link if there is one, and it is a subtree that has the
 *   same structure described for the top-level array.
 */
function ctools_menu_tree_page_data($item, $menu_name = 'navigation') {
  static $tree = array();

  // Generate a cache ID (cid) specific for this page.
  $cid = 'links:'. $menu_name .':page-cid:'. $item['href'] .':'. (int)$item['access'];

  if (!isset($tree[$cid])) {
    // If the static variable doesn't have the data, check {cache_menu}.
    $cache = cache_get($cid, 'cache_menu');
    if ($cache && isset($cache->data)) {
      // If the cache entry exists, it will just be the cid for the actual data.
      // This avoids duplication of large amounts of data.
      $cache = cache_get($cache->data, 'cache_menu');
      if ($cache && isset($cache->data)) {
        $data = $cache->data;
      }
    }
    // If the tree data was not in the cache, $data will be NULL.
    if (!isset($data)) {
      // Build and run the query, and build the tree.
      if ($item['access']) {
        // Check whether a menu link exists that corresponds to the current path.
        $args = array($menu_name, $item['href']);
        $placeholders = "'%s'";
        if (drupal_is_front_page()) {
          $args[] = '<front>';
          $placeholders .= ", '%s'";
        }
        $parents = db_fetch_array(db_query("SELECT p1, p2, p3, p4, p5, p6, p7, p8 FROM {menu_links} WHERE menu_name = '%s' AND link_path IN (". $placeholders .")", $args));

        if (empty($parents)) {
          // If no link exists, we may be on a local task that's not in the links.
          // TODO: Handle the case like a local task on a specific node in the menu.
          $parents = db_fetch_array(db_query("SELECT p1, p2, p3, p4, p5, p6, p7, p8 FROM {menu_links} WHERE menu_name = '%s' AND link_path = '%s'", $menu_name, $item['tab_root']));
        }
        // We always want all the top-level links with plid == 0.
        $parents[] = '0';

        // Use array_values() so that the indices are numeric for array_merge().
        $args = $parents = array_unique(array_values($parents));
        $placeholders = implode(', ', array_fill(0, count($args), '%d'));
        $expanded = variable_get('menu_expanded', array());
        // Check whether the current menu has any links set to be expanded.
        if (in_array($menu_name, $expanded)) {
          // Collect all the links set to be expanded, and then add all of
          // their children to the list as well.
          do {
            $result = db_query("SELECT mlid FROM {menu_links} WHERE menu_name = '%s' AND expanded = 1 AND has_children = 1 AND plid IN (". $placeholders .') AND mlid NOT IN ('. $placeholders .')', array_merge(array($menu_name), $args, $args));
            $num_rows = FALSE;
            while ($item = db_fetch_array($result)) {
              $args[] = $item['mlid'];
              $num_rows = TRUE;
            }
            $placeholders = implode(', ', array_fill(0, count($args), '%d'));
          } while ($num_rows);
        }
        array_unshift($args, $menu_name);
      }
      else {
        // Show only the top-level menu items when access is denied.
        $args = array($menu_name, '0');
        $placeholders = '%d';
        $parents = array();
      }
      // Select the links from the table, and recursively build the tree. We
      // LEFT JOIN since there is no match in {menu_router} for an external
      // link.
      $data['tree'] = menu_tree_data(db_query("
        SELECT m.load_functions, m.to_arg_functions, m.access_callback, m.access_arguments, m.page_callback, m.page_arguments, m.title, m.title_callback, m.title_arguments, m.type, m.description, ml.*
        FROM {menu_links} ml LEFT JOIN {menu_router} m ON m.path = ml.router_path
        WHERE ml.menu_name = '%s' AND ml.plid IN (". $placeholders .")
        ORDER BY p1 ASC, p2 ASC, p3 ASC, p4 ASC, p5 ASC, p6 ASC, p7 ASC, p8 ASC, p9 ASC", $args), $parents);
      $data['node_links'] = array();
      menu_tree_collect_node_links($data['tree'], $data['node_links']);
      // Cache the data, if it is not already in the cache.
      $tree_cid = _menu_tree_cid($menu_name, $data);
      if (!cache_get($tree_cid, 'cache_menu')) {
        cache_set($tree_cid, $data, 'cache_menu');
      }
      // Cache the cid of the (shared) data using the page-specific cid.
      cache_set($cid, $tree_cid, 'cache_menu');
    }
    // Check access for the current user to each item in the tree.
    menu_tree_check_access($data['tree'], $data['node_links']);
    $tree[$cid] = $data['tree'];
  }
  return $tree[$cid];
}

function ctools_menu_set_trail_parent($path) {
  $current = menu_get_active_trail();
  $keep = array_pop($current);

  $trail = ctools_get_menu_trail($path);
  $trail[] = $keep;

  menu_set_active_trail($trail);
}

/**
 * An alternative to theme_menu_local_tasks to add flexibility to tabs.
 *
 * The code in theme_menu_local_tasks has no entry points to put hooks.
 * Therefore, what CTools does is, if that theme function is not overridden,
 * it uses hook_theme_registry_alter() to use its own version. This version
 * then allows modules to use ctools_menu_add_local_task() to add a dynamic
 * local task to the list.
 *
 * If a theme *does* override theme_menu_local_tasks, it can still get this
 * functionality by using ctools versions of menu_primary_local_tasks() and
 * menu_secondary_local_tasks().
 */
function ctools_theme_menu_local_tasks() {
  $output = '';

  if ($primary = ctools_menu_primary_local_tasks()) {
    $output .= "<ul class=\"tabs primary\">\n". $primary ."</ul>\n";
  }
  if ($secondary = ctools_menu_secondary_local_tasks()) {
    $output .= "<ul class=\"tabs secondary\">\n". $secondary ."</ul>\n";
  }

  return $output;
}

/**
 * CTools variant of menu_primary_local_tasks().
 *
 * This can be called by themes which implement their own theme_menu_local_tasks
 * in order to get local tasks that include CTools' additions.
 */
function ctools_menu_primary_local_tasks() {
  return ctools_menu_local_tasks(0);
}

/**
 * CTools variant of menu_secondary_local_tasks().
 *
 * This can be called by themes which implement their own theme_menu_local_tasks
 * in order to get local tasks that include CTools' additions.
 */
function ctools_menu_secondary_local_tasks() {
  return ctools_menu_local_tasks(1);
}

/**
 * CTools' variant of menu_local_tasks.
 *
 * This function is a new version of menu_local_tasks that is meant to be more
 * flexible and allow for modules to dynamically add items to the local tasks
 * using a simple function.
 *
 * One downside to using this is that the code to build the tabs could be run
 * twice on a page if something
 */
function ctools_menu_local_tasks($level = 0, $return_root = FALSE) {
  static $tabs;
  static $root_path;

  if (!isset($tabs)) {
    $tabs = array();

    $router_item = menu_get_item();
    if (!$router_item || !$router_item['access']) {
      return '';
    }
    // Get all tabs and the root page.
    $result = db_query("SELECT * FROM {menu_router} WHERE tab_root = '%s' ORDER BY weight, title", $router_item['tab_root']);
    $map = arg();
    $children = array();
    $tasks = array();
    $root_path = $router_item['path'];

    while ($item = db_fetch_array($result)) {
      _menu_translate($item, $map, TRUE);
      if ($item['tab_parent']) {
        // All tabs, but not the root page.
        $children[$item['tab_parent']][$item['path']] = $item;
      }
      // Store the translated item for later use.
      $tasks[$item['path']] = $item;
    }

    // Find all tabs below the current path.
    $path = $router_item['path'];

    _ctools_menu_add_dynamic_items($children[$path]);

    // Tab parenting may skip levels, so the number of parts in the path may not
    // equal the depth. Thus we use the $depth counter (offset by 1000 for ksort).
    $depth = 1001;
    while (isset($children[$path])) {
      $tabs_current = '';
      $next_path = '';
      $count = 0;
      foreach ($children[$path] as $item) {
        if ($item['access']) {
          $count++;
          // The default task is always active.
          if ($item['type'] == MENU_DEFAULT_LOCAL_TASK) {
            // Find the first parent which is not a default local task.
            if (isset($item['tab_parent'])) {
              for ($p = $item['tab_parent']; $tasks[$p]['type'] == MENU_DEFAULT_LOCAL_TASK; $p = $tasks[$p]['tab_parent']);
              $href = $tasks[$p]['href'];
              $next_path = $item['path'];
            }
            else {
              $href = $item['href'];
            }
            $link = theme('menu_item_link', array('href' => $href) + $item);
            $tabs_current .= theme('menu_local_task', $link, TRUE);
          }
          else {
            $link = theme('menu_item_link', $item);
            $tabs_current .= theme('menu_local_task', $link);
          }
        }
      }
      $path = $next_path;
      $tabs[$depth]['count'] = $count;
      $tabs[$depth]['output'] = $tabs_current;
      $depth++;
    }

    // Find all tabs at the same level or above the current one.
    $parent = $router_item['tab_parent'];
    $path = $router_item['path'];
    $current = $router_item;
    $depth = 1000;
    while (isset($children[$parent])) {
      $tabs_current = '';
      $next_path = '';
      $next_parent = '';
      $count = 0;
      foreach ($children[$parent] as $item) {
        if ($item['access']) {
          $count++;
          if ($item['type'] == MENU_DEFAULT_LOCAL_TASK) {
            // Find the first parent which is not a default local task.
            for ($p = $item['tab_parent']; $tasks[$p]['type'] == MENU_DEFAULT_LOCAL_TASK; $p = $tasks[$p]['tab_parent']);
            $link = theme('menu_item_link', array('href' => $tasks[$p]['href']) + $item);
            if ($item['path'] == $router_item['path']) {
              $root_path = $tasks[$p]['path'];
            }
          }
          else {
            $link = theme('menu_item_link', $item);
          }
          // We check for the active tab.
          if ($item['path'] == $path) {
            $tabs_current .= theme('menu_local_task', $link, TRUE);
            $next_path = $item['tab_parent'];
            if (isset($tasks[$next_path])) {
              $next_parent = $tasks[$next_path]['tab_parent'];
            }
          }
          else {
            $tabs_current .= theme('menu_local_task', $link);
          }
        }
      }
      $path = $next_path;
      $parent = $next_parent;
      $tabs[$depth]['count'] = $count;
      $tabs[$depth]['output'] = $tabs_current;
      $depth--;
    }
    // Sort by depth.
    ksort($tabs);
    // Remove the depth, we are interested only in their relative placement.
    $tabs = array_values($tabs);
  }

  if ($return_root) {
    return $root_path;
  }
  else {
    // We do not display single tabs.
    return (isset($tabs[$level]) && $tabs[$level]['count'] > 1) ? $tabs[$level]['output'] : '';
  }
}

/**
 * Re-sort menu items after we have modified them.
 */
function ctools_menu_sort($a, $b) {
  $a_weight = (is_array($a) && isset($a['weight'])) ? $a['weight'] : 0;
  $b_weight = (is_array($b) && isset($b['weight'])) ? $b['weight'] : 0;
  if ($a_weight == $b_weight) {
    $a_title = (is_array($a) && isset($a['title'])) ? $a['title'] : 0;
    $b_title = (is_array($b) && isset($b['title'])) ? $b['title'] : 0;
    if ($a_title == $b_title) {
      return 0;
    }

    return ($a_title < $b_title) ? -1 : 1;
  }

  return ($a_weight < $b_weight) ? -1 : 1;
}

/**
 * Theme override to use CTools to call help instead of the basic call.
 *
 * This is used only to try and improve performance; this calls through to
 * the same functions that tabs do and executes a complex build. By overriding
 * this to use the CTools version, we can prevent this query from being run
 * twice on the same page.
 */
function ctools_menu_help() {
  if ($help = ctools_menu_get_active_help()) {
    return '<div class="help">'. $help .'</div>';
  }
}

/**
 * CTools' replacement for ctools_menu_get_active_help()
 */
function ctools_menu_get_active_help() {
  $output = '';
  $router_path = ctools_menu_tab_root_path();
  // We will always have a path unless we are on a 403 or 404.
  if (!$router_path) {
    return '';
  }

  $arg = drupal_help_arg(arg(NULL));
  $empty_arg = drupal_help_arg();

  foreach (module_list() as $name) {
    if (module_hook($name, 'help')) {
      // Lookup help for this path.
      if ($help = module_invoke($name, 'help', $router_path, $arg)) {
        $output .= $help ."\n";
      }
      // Add "more help" link on admin pages if the module provides a
      // standalone help page.
      if ($arg[0] == "admin" && module_exists('help') && module_invoke($name, 'help', 'admin/help#'. $arg[2], $empty_arg) && $help) {
        $output .= theme("more_help_link", url('admin/help/'. $arg[2]));
      }
    }
  }
  return $output;
}

/**
 * CTools' replacement for menu_tab_root_path()
 */
function ctools_menu_tab_root_path() {
  return ctools_menu_local_tasks(0, TRUE);
}

/**
 * Returns the rendered local tasks. The default implementation renders
 * them as tabs. Overridden to split the secondary tasks.
 *
 * @ingroup themeable
 */
function ctools_garland_menu_local_tasks() {
  return ctools_menu_primary_local_tasks();
}

function _ctools_menu_add_dynamic_items(&$links) {
  // TESTING: add hardcoded values.
  if ($additions = ctools_menu_add_tab()) {
    foreach ($additions as $addition) {
      $links[$addition['href']] = $addition + array(
        'access' => TRUE,
        'type' => MENU_LOCAL_TASK,
      );
    }
    uasort($links, 'ctools_menu_sort');
  }
}
