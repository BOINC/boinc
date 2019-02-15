<?php

/**
 * CTools Export UI plugin for current search block configurations.
 */
$plugin = array(
  'schema' => 'current_search',
  'access' => 'administer search',

  'menu' => array(
    'menu prefix' => 'admin/settings',
    'menu item' => 'current_search',
    'menu title' => 'Current search blocks',
    'menu description' => 'Configure current search blocks.',
  ),

  'title singular' => t('item'),
  'title plural' => t('items'),
  'title singular proper' => t('Current search item'),
  'title plural proper' => t('Current search items'),

  'form' => array(
    'settings' => 'current_search_settings_form',
    'submit' => 'current_search_settings_form_submit',
  ),

  'handler' => array(
    'class' => 'current_search_export_ui',
    'parent' => 'ctools_export_ui',
  ),
);
