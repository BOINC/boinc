<?php

/**
 * Implementation of hook_menu_default_menu_custom().
 */
function boinc_standard_menu_default_menu_custom() {
  $menus = array();

  // Exported menu: menu-footer-links
  $menus['menu-footer-links'] = array(
    'menu_name' => 'menu-footer-links',
    'title' => 'Footer links',
    'description' => 'Auxiliary menu, typically at the bottom of the page in the footer, that includes links to content such as "about us" or "privacy policy" etc.',
  );
  // Translatables
  // Title and description for footer menu appears in admin menu: Admin> Site Building> Menus
  t('Auxiliary menu, typically at the bottom of the page in the footer, that includes links to content such as "about us" or "privacy policy" etc.');
  t('Footer links');


  return $menus;
}
