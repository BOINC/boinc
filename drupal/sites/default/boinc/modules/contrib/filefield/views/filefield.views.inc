<?php
/**
 * @file
 * Provide views data for filefield.module.
 */

/**
 * @defgroup views_filefield_module filefield.module handlers
 *
 * Includes the handler for the FileField data column.
 * @{
 */


/**
 * Implementation of hook_views_data()
 */
function filefield_views_data() {
  $data = array();

  // Extend the files table with an icon field.
  $data['files']['icon'] = array(
    'title' => t('Icon'),
    'help' => t('An icon corresponding to the file MIME type.'),
    'real field' => 'filemime',
    'field' => array(
      'handler' => 'filefield_handler_field_icon',
      'click sortable' => FALSE,
     ),
  );

  return $data;
}

/**
 * Implementation of hook_views_handlers().
 */
function filefield_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'filefield') . '/views',
    ),
    'handlers' => array(
      // field handlers
      'filefield_handler_field_data' => array(
        'parent' => 'views_handler_field_node',
      ),
      'filefield_handler_field_icon' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
}

/**
 * @}
 */
