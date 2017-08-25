<?php

/**
 * Implementation of hook_views_plugins
 */
function panels_views_plugins() {
  $plugins = array(
    'row' => array(
      'panels_fields' => array(
        'title' => t('Panel fields'),
        'help' => t('Displays the fields in a panel rather than using a template.'),
        'handler' => 'panels_views_plugin_row_fields',
        'path' => drupal_get_path('module', 'panels') . '/plugins/views',
        'theme' => 'views_view_fields',
        'theme path' => drupal_get_path('module', 'views') . '/theme',
        'uses fields' => TRUE,
        'uses options' => TRUE,
        'type' => 'normal',
        'help topic' => 'style-row-panels-fields',
        'parent' => 'fields',
      ),
    ),
  );

  return $plugins;
}
