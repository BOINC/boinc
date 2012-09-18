<?php

/**
 * @file
 * Provide views data and handlers for vud_node.module
 */

/**
 * Implementation of hook_views_data_alter().
 */
function vud_node_views_data_alter(&$data) {
  $data['node']['vud_node_widget'] = array(
    'group' => t('Vote Up/Down'),
    'title' => t('Node Widget'),
    'help' => t('Provide a node widget for the Vote up/Down module.'),
    'field' => array(
      'handler' => 'vud_node_handler_field_widget',
    ),
  );
}

/**
 * Implementation of hook_views_handlers().
 */
function vud_node_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'vud_node') . '/views'
    ),
    'handlers' => array(
      'vud_node_handler_field_widget' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
}
