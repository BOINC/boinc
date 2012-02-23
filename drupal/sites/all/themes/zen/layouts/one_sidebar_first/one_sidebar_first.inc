<?php
// $Id: one_sidebar_first.inc,v 1.1.2.1 2009/12/12 15:31:54 johnalbin Exp $

/**
 * Implements hook_panels_layouts().
 */
function zen_one_sidebar_first_panels_layouts() {
  $items['one_sidebar_first'] = array(
    'title' => t('Zen Layout: one sidebar before content'),
    'icon' => 'one-sidebar-first.png',
    'theme' => 'zen_one_sidebar_first',
    'admin theme' => 'zen_one_sidebar_first_admin',
    'css' => 'one-sidebar-first.css',
    'admin css' => 'one-sidebar-first-admin.css',    
    'panels' => array(
      'content' => t('Content'),
      'sidebar_first' => t('First sidebar'),
    ),
  );

  return $items;
}
