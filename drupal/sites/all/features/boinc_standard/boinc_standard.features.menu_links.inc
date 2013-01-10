<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function boinc_standard_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: menu-footer-links:http://boinc.berkeley.edu/
  $menu_links['menu-footer-links:http://boinc.berkeley.edu/'] = array(
    'menu_name' => 'menu-footer-links',
    'link_path' => 'http://boinc.berkeley.edu/',
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
  // Exported menu link: menu-footer-links:node/10
  $menu_links['menu-footer-links:node/10'] = array(
    'menu_name' => 'menu-footer-links',
    'link_path' => 'node/10',
    'router_path' => 'node/%',
    'link_title' => 'Privacy Policy',
    'options' => array(
      'attributes' => array(
        'title' => 'Privacy policy',
      ),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '20',
  );
  // Exported menu link: menu-footer-links:node/7
  $menu_links['menu-footer-links:node/7'] = array(
    'menu_name' => 'menu-footer-links',
    'link_path' => 'node/7',
    'router_path' => 'node/%',
    'link_title' => 'About Us',
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
    'weight' => '0',
  );
  // Exported menu link: menu-footer-links:node/8
  $menu_links['menu-footer-links:node/8'] = array(
    'menu_name' => 'menu-footer-links',
    'link_path' => 'node/8',
    'router_path' => 'node/%',
    'link_title' => 'Development',
    'options' => array(
      'attributes' => array(
        'title' => 'Project application source code and license',
      ),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '5',
  );
  // Exported menu link: menu-footer-links:node/9
  $menu_links['menu-footer-links:node/9'] = array(
    'menu_name' => 'menu-footer-links',
    'link_path' => 'node/9',
    'router_path' => 'node/%',
    'link_title' => 'Terms of Use',
    'options' => array(
      'attributes' => array(
        'title' => 'Terms of use',
      ),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '0',
    'weight' => '15',
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
    'weight' => '0',
  );
  // Exported menu link: primary-links:dashboard
  $menu_links['primary-links:dashboard'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'dashboard',
    'router_path' => 'dashboard',
    'link_title' => 'Account',
    'options' => array(
      'attributes' => array(
        'title' => '',
      ),
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '1',
    'expanded' => '1',
    'weight' => '5',
  );
  // Exported menu link: primary-links:user/me/edit
  $menu_links['primary-links:user/me/edit'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'user/me/edit',
    'router_path' => 'user/%/edit',
    'link_title' => 'Account Info',
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
    'weight' => '10',
    'parent_path' => 'dashboard',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('About Us');
  t('Account');
  t('Account Info');
  t('BOINC');
  t('Development');
  t('Home');
  t('Privacy Policy');
  t('Terms of Use');


  return $menu_links;
}
