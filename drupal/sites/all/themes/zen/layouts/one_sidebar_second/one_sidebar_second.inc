<?php
// $Id: one_sidebar_second.inc,v 1.1.2.1 2009/12/12 15:31:54 johnalbin Exp $

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
