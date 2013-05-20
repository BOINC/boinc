<?php

/**
 * Implementation of hook_menu_default_menu_links().
 */
function discussion_forums_menu_default_menu_links() {
  $menu_links = array();

  // Exported menu link: primary-links:forum
  $menu_links['primary-links:forum'] = array(
    'menu_name' => 'primary-links',
    'link_path' => 'forum',
    'router_path' => 'forum',
    'link_title' => 'Community',
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
  );
  // Translatables
  // Included for use with string extractors like potx.
  t('Community');


  return $menu_links;
}
