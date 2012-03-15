<?php
// $Id: context_reaction_menu.inc,v 1.1.2.6 2010/07/30 17:25:28 yhahn Exp $

/**
 * Expose menu items as context reactions.
 */
class context_reaction_menu extends context_reaction {
  /**
   * Provide a form element that allow the admin to chose a menu item.
   */
  function options_form($context) {
    $menus = menu_parent_options(array_reverse(menu_get_menus()), NULL);
    $root_menus = array();
    foreach ($menus as $key => $name) {
      $id = explode(':', $key);
      if ($id[1] == '0') {
        $root_menus[$id[0]] = check_plain($name);
      }
      else {
        $link = menu_link_load($id[1]);
        $identifier = $link['link_path'];
        $root_menu = $root_menus[$id[0]];
        while (isset($menus[$root_menu][$identifier])) {
          $identifier .= "'";
        }
        $menus[$root_menu][$identifier] = $name;
      }
      unset($menus[$key]);
    }
    array_unshift($menus, "-- ". t('None') ." --");
    return array(
      '#title' => $this->title,
      '#description' => $this->description,
      '#options' => $menus,
      '#type' => 'select',
      '#default_value' => $this->fetch_from_context($context),
    );
  }

  /**
   * Override of options_form_submit().
   * Trim any identifier padding for non-unique path menu items.
   */
  function options_form_submit($values) {
    return trim($values, "'");
  }

  /**
   * If primary + secondary links are pointed at the same menu, provide
   * contextual trailing by default.
   */
  function execute(&$vars) {
    if (variable_get('menu_primary_links_source', 'primary-links') == variable_get('menu_secondary_links_source', 'secondary-links')) {
      $vars['primary_links'] = theme_get_setting('toggle_primary_links') ? $this->menu_navigation_links(variable_get('menu_primary_links_source', 'primary-links')) : $vars['primary_links'];
      $vars['secondary_links'] = theme_get_setting('toggle_secondary_links') ? $this->menu_navigation_links(variable_get('menu_secondary_links_source', 'secondary-links'), 1) : $vars['secondary_links'];
    }

    $vars['primary_links'] = $this->menu_set_active($vars['primary_links']);
    $vars['secondary_links'] = $this->menu_set_active($vars['secondary_links']);
  }

  function get_active_paths() {
    $active_paths = array();
    foreach ($this->get_contexts() as $context) {
      if (isset($context->reactions[$this->plugin])) {
        $active_paths[] = $context->reactions[$this->plugin];
      }
    }
    return $active_paths;
  }

  /**
   * Iterates through a provided links array for use with theme_links()
   * (e.g. from menu_primary_links()) and provides an active class for
   * any items that have a path that matches an active context.
   *
   * @param $links
   *   An array of links.
   * @param $reset
   *   A boolean flag for resetting the static cache.
   *
   * @return
   *   A modified links array.
   */
  function menu_set_active($links = array(), $reset = FALSE) {
    $new_links = array();
    if (!empty($links)) {
      $active_paths = $this->get_active_paths();

      // Iterate through the provided links and build a new set of links
      // that includes active classes
      foreach ($links as $key => $link) {
        if (!empty($link['href']) && in_array($link['href'], $active_paths)) {
          if (isset($link['attributes']['class'])) {
            $link['attributes']['class'] .= ' active';
          }
          else {
            $link['attributes']['class'] = 'active';
          }

          if (strpos(' active', $key) === FALSE) {
            $new_links[$key .' active'] = $link;
          }
        }
        else {
          $new_links[$key] = $link;
        }
      }
    }
    return $new_links;
  }

  /**
   * Wrapper around menu_navigation_links() that gives themers the option of
   * building navigation links based on an active context trail.
   */
  function menu_navigation_links($menu_name, $level = 0) {
    // Retrieve original path so we can repair it after our hack.
    $original_path = $_GET['q'];

    // Retrieve the first active menu path found.
    if ($active_paths = $this->get_active_paths()) {
      $path = current($active_paths);
      if (menu_get_item($path)) {
        menu_set_active_item($path);
      }
    }

    // Build the links requested
    $links = menu_navigation_links($menu_name, $level);

    // Repair and get out
    menu_set_active_item($original_path);
    return $links;
  }
}
