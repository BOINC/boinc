<?php
// $Id: context_condition_menu.inc,v 1.1.2.5 2010/07/30 06:28:43 yhahn Exp $

/**
 * Expose menu items as a context condition.
 */
class context_condition_menu extends context_condition {
  /**
   * Override of condition_values().
   */
  function condition_values() {
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
    return $menus;
  }

  /**
   * Override of condition_form().
   * Use multiselect widget.
   */
  function condition_form($context) {
    $form = parent::condition_form($context);
    $form['#type'] = 'select';
    $form['#multiple'] = TRUE;
    return $form;
  }

  /**
   * Override of condition_form_submit().
   * Trim any identifier padding for non-unique path menu items.
   */
  function condition_form_submit($values) {
    // Trim any identifier padding for non-unique path menu items.
    $values = parent::condition_form_submit($values);
    $trimmed = array();
    foreach ($values as $key => $value) {
      $trimmed[trim($key, "'")] = trim($value, "'");
    }
    return $trimmed;
  }

  /**
   * Override of execute().
   */
  function execute() {
    if ($this->condition_used()) {
      // Menu trail condition integration. Note that because of the way
      // menu_get_active_trail() is written this will often not work for active
      // menu items outside the standard navigation tree without the additional
      // helper code below.
      if (menu_get_active_menu_name() === 'navigation') {
        $item = menu_get_item();
        if ($menu_name = db_result(db_query("SELECT menu_name FROM {menu_links} WHERE link_path = '%s'", $item['href']))) {
          menu_set_active_menu_name($menu_name);
        }
      }
      $trail = menu_get_active_trail();
      foreach ($trail as $item) {
        if (!empty($item['href'])) {
          foreach ($this->get_contexts($item['href']) as $context) {
            $this->condition_met($context, $item['href']);
          }
        }
      }
    }
  }
}
