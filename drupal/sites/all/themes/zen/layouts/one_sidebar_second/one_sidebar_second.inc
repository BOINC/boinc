<?php
/**
 * Implements hook_panels_layouts().
 */
function zen_one_sidebar_second_panels_layouts() {
  $items['one_sidebar_second'] = array(
    'title' => t('Zen Layout: one sidebar after content'),
    'icon' => 'one-sidebar-second.png',
    'theme' => 'zen_one_sidebar_second',
    'admin theme' => 'zen_one_sidebar_second_admin',
    'css' => 'one-sidebar-second.css',
    'admin css' => 'one-sidebar-second-admin.css',    
    'panels' => array(
      'content' => t('Content'),
      'sidebar_first' => t('First sidebar'),
    ),
  );

  return $items;
}
