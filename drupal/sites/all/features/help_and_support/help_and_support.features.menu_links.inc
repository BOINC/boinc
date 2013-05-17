<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function help_and_support_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: primary-links:http://boinc.berkeley.edu/help.php
  $menu_links['primary-links:http://boinc.berkeley.edu/help.php'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'http://boinc.berkeley.edu/help.php',
    'router_path' => '',
    'link_title' => 'Help',
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
    'weight' => '-46',
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Help');


  return $menu_links;
}
