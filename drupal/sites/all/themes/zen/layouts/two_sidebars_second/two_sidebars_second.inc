<?php
// $Id: two_sidebars_second.inc,v 1.1.2.1 2009/12/12 15:31:55 johnalbin Exp $

/**
 * Implements hook_panels_layouts().
 */
function zen_two_sidebars_second_panels_layouts() {
  $items['two_sidebars_second'] = array(
    'title' => t('Zen Layout: two sidebars after content'),
    'icon' => 'two-sidebars-second.png',
    'theme' => 'zen_two_sidebars_second',
    'admin theme' => 'zen_two_sidebars_second_admin',
    'css' => 'two-sidebars-second.css',
    'admin css' => 'two-sidebars-second-admin.css',    
    'panels' => array(
      'content' => t('Content'),
      'sidebar_first' => t('First sidebar'),
      'sidebar_second' => t('Second sidebar'),
    ),
  );

  return $items;
}
