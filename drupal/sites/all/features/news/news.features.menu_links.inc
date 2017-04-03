<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function news_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: primary-links:news
  $menu_links['primary-links:news'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'news',
    'router_path' => 'news',
    'link_title' => 'News',
    'options' => array(
      'attributes' => array(
        'title' => '',
      ),
      'alter' => TRUE,
    ),
    'module' => 'menu',
    'hidden' => '0',
    'external' => '0',
    'has_children' => '0',
    'expanded' => '1',
    'weight' => '-48',
  );
  // Translatables
  // Use bts() function to translate additional strings.
  bts('News', array(), NULL, 'boinc:menu-link');


  return $menu_links;
}
