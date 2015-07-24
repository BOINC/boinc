<?php
/**
 * Implements hook_panels_layouts().
 */
function zen_two_sidebars_first_panels_layouts() {
  $items['two_sidebars_first'] = array(
    'title' => t('Zen Layout: two sidebars before content'),
    'icon' => 'two-sidebars-first.png',
    'theme' => 'zen_two_sidebars_first',
    'admin theme' => 'zen_two_sidebars_first_admin',
    'css' => 'two-sidebars-first.css',
    'admin css' => 'two-sidebars-first-admin.css',    
    'panels' => array(
      'content' => t('Content'),
      'sidebar_first' => t('First sidebar'),
      'sidebar_second' => t('Second sidebar'),
    ),
  );

  return $items;
}
