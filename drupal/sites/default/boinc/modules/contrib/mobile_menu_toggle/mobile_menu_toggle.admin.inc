<?php
/**
 * @file
 * Administration pages for mobile menu toggle module.
 */

/**
 * System settings form page callback.
 */
function mobile_menu_toggle_form() {
  $form = array();

  $form['mobile_menu_menu'] = array(
    '#title' => t('Menu selection'),
    '#description' => t('Which menu should the toggle open?'),
    '#type' => 'select',
    '#options' => menu_get_menus(),
    '#default_value' => variable_get('mobile_menu_menu', 'main-menu'),
  );

  $form['mobile_menu_block'] = array(
    '#title' => t('Menu loaded in block?'),
    '#description' => t('Will this menu be loaded via a block? Check this box if you are loading your menu on the page via context or blocks.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('mobile_menu_block', FALSE),
  );

  $form['mobile_menu_css'] = array(
    '#title' => t('(optional) CSS Selector'),
    '#description' => t('Enter an optional CSS selector. If this option is used the menu selected above will be ignored'),
    '#type' => 'textfield',
    '#default_value' => variable_get('mobile_menu_css', ''),
  );

  return system_settings_form($form);
}
