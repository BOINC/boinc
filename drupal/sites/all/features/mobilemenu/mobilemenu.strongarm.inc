<?php

/**
 * Implementation of hook_strongarm().
 */
function mobilemenu_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'dhtml_menu_settings';
  $strongarm->value = array(
    'nav' => 'none',
    'animation' => array(
      'effects' => array(
        'height' => 'height',
        'opacity' => 'opacity',
        'width' => 0,
      ),
      'speed' => '500',
    ),
    'effects' => array(
      'siblings' => 'close-same-tree',
      'children' => 'none',
      'remember' => '0',
    ),
    'filter' => array(
      'type' => 'whitelist',
      'list' => array(
        'primary-links' => 'primary-links',
        'devel' => 0,
        'menu-footer-links' => 0,
        'navigation' => 0,
      ),
    ),
  );
  $export['dhtml_menu_settings'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'dhtml_menu_theme';
  $strongarm->value = array(
    'garland' => array(
      'menu_item' => 'theme_menu_item',
      'menu_item_link' => 'theme_menu_item_link',
    ),
    'einstein' => array(
      'menu_item' => 'theme_menu_item',
      'menu_item_link' => 'zen_menu_item_link',
    ),
    'boinc' => array(
      'menu_item' => 'theme_menu_item',
      'menu_item_link' => 'zen_menu_item_link',
    ),
  );
  $export['dhtml_menu_theme'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'mobile_menu_block';
  $strongarm->value = 0;
  $export['mobile_menu_block'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'mobile_menu_css';
  $strongarm->value = '#navigation-mmt';
  $export['mobile_menu_css'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'mobile_menu_menu';
  $strongarm->value = 'primary-links';
  $export['mobile_menu_menu'] = $strongarm;

  return $export;
}
