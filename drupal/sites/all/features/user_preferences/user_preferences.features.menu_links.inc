<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function user_preferences_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: primary-links:account/prefs
  $menu_links['primary-links:account/prefs'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'account/prefs',
    'router_path' => 'account/prefs',
    'link_title' => 'Preferences',
    'options' => array(
      'attributes' => array(
        'title' => '',
      ),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-47',
    'parent_path' => 'dashboard',
  );
  // Translatables
  // Use bts() function to translate additional strings.
  bts('Preferences', array(), NULL, 'boinc:menu-link');
  bts('Community', array(), NULL, 'boinc:menu-link');
  bts('Privacy', array(), NULL, 'boinc:menu-link');
  bts('Subscriptions', array(), NULL, 'boinc:menu-link:-1:ignoreoverwrite');

  return $menu_links;
}
