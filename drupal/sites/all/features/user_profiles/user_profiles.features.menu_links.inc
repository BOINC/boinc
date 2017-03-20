<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function user_profiles_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: primary-links:account/profile
  $menu_links['primary-links:account/profile'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'account/profile',
    'router_path' => 'account/profile',
    'link_title' => 'Profile',
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
    'weight' => '-49',
    'parent_path' => 'dashboard',
  );
  // Exported menu link: primary-links:moderate/profiles
  $menu_links['primary-links:moderate/profiles'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'moderate/profiles',
    'router_path' => 'moderate/profiles',
    'link_title' => 'Queue',
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
    'weight' => '-50',
    'parent_path' => 'moderate',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Queue');
  t('Profile');


  return $menu_links;
}
