<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function boinc_standard_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: menu-footer-links:https://boinc.berkeley.edu/
  $menu_links['menu-footer-links:https://boinc.berkeley.edu/'] = array(
    'menu_name' => 'menu-footer-links',
    'link_path' => 'https://boinc.berkeley.edu/',
    'router_path' => '',
    'link_title' => 'BOINC',
    'options' => array(
      'attributes' => array(
        'title' => '',
      ),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '1',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '10',
  );
  // Exported menu link: primary-links:<front>
  $menu_links['primary-links:<front>'] = array(
    'menu_name' => 'primary-links',
    'link_path' => '<front>',
    'router_path' => '',
    'link_title' => 'Home',
    'options' => array(
      'attributes' => array(
        'title' => '',
      ),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '1',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '-50',
  );
  // Exported menu link: primary-links:dashboard
  $menu_links['primary-links:dashboard'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'dashboard',
    'router_path' => 'dashboard',
    'link_title' => 'Dashboard',
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
    'parent_path' => 'dashboard',
  );
  // Exported menu link: primary-links:user/me/edit
  $menu_links['primary-links:user/me/edit'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'user/me/edit',
    'router_path' => 'user/%/edit',
    'link_title' => 'Credentials',
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
    'weight' => '-48',
    'parent_path' => 'dashboard',
  );
  // Translatables
  // Use bts() function to translate additional strings.
  bts('Home', array(), NULL, 'boinc:menu-link');
  bts('Credentials', array(), NULL, 'boinc:menu-link');
  bts('Dashboard', array(), NULL, 'boinc:menu-link');

  return $menu_links;
}
