<?php
/**
 * Implements hook_panels_layouts().
 */
function zen_two_sidebars_panels_layouts() {
  $items['two_sidebars'] = array(
    'title' => t('Zen Layout: two sidebars with content'),
    'icon' => 'two-sidebars.png',
    'theme' => 'zen_two_sidebars',
    'admin theme' => 'zen_two_sidebars_admin',
    'css' => 'two-sidebars.css',
    'admin css' => 'two-sidebars-admin.css',    
    'panels' => array(
      'content' => t('Content'),
      'sidebar_first' => t('First sidebar'),
      'sidebar_second' => t('Second sidebar'),
    ),
  );

  return $items;
}
