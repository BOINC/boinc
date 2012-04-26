<?php
/**
 * @file
 *
 * Provide views data and handlers for system tables that are not represented by
 * their own module.
 */

/**
 * @defgroup views_system_module system.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function system_views_data() {
  $data = array();

  // ----------------------------------------------------------------------
  // files table

  $data['files']['table']['group']  = t('File');

  // Advertise this table as a possible base table
  $data['files']['table']['base'] = array(
    'field' => 'fid',
    'title' => t('File'),
    'help' => t("Files maintained by Drupal and various modules."),
  );

  // The files table does not inherently join to the node table,
  // but may things (such as upload.module) can add relationships
  // that allow file fields to be used.

  // For other base tables, explain how we join
  $data['files']['table']['join'] = array(
    'users' => array(
      // direct join to the users table via 'uid' field.
      'left_field' => 'uid',
      'field' => 'uid',
     ),
  );

  // fid
  $data['files']['fid'] = array(
    'title' => t('File ID'),
    'help' => t('The ID of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_file_fid',
      'name field' => 'filename', // the field to display in the summary.
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // filename
  $data['files']['filename'] = array(
    'title' => t('Name'),
    'help' => t('The name of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // filepath
  $data['files']['filepath'] = array(
    'title' => t('Path'),
    'help' => t('The path of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // filemime
  $data['files']['filemime'] = array(
    'title' => t('Mime type'),
    'help' => t('The mime type of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  // filesize
  $data['files']['filesize'] = array(
    'title' => t('Size'),
    'help' => t('The size of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file_size',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  // status
  $data['files']['status'] = array(
    'title' => t('Status'),
    'help' => t('The status of the file.'),
    'field' => array(
      'handler' => 'views_handler_field_file_status',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_file_status',
    ),
  );

  // timestamp field
  $data['files']['timestamp'] = array(
    'title' => t('Upload date'),
    'help' => t('The date the file was uploaded.'),
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort_date',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
  );

  // uid field
  $data['files']['uid'] = array(
    'title' => t('User'),
    'help' => t('The user, who uploaded the file.'),
    'relationship' => array(
      'base' => 'users',
      'base field' => 'uid',
      'handler' => 'views_handler_relationship',
      'label' => t('User'),
    ),
  );


  return $data;
}

/**
 * Implementation of hook_views_handlers() to register all of the basic handlers
 * views uses.
 */
function system_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views') . '/modules/system',
    ),
    'handlers' => array(
      'views_handler_field_file' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_field_file_status' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_filter_file_status' => array(
        'parent' => 'views_handler_filter_in_operator',
      ),
      'views_handler_argument_file_fid' => array(
        'parent' => 'views_handler_argument',
      ),
    ),
  );
}

function _views_file_status($choice = NULL) {
  $status = array(
    FILE_STATUS_TEMPORARY => t('Temporary'),
    FILE_STATUS_PERMANENT => t('Permanent'),
  );

  if (isset($choice)) {
    return isset($status[$choice]) ? $status[$choice] : t('Unknown');
  }

  return $status;
}


/**
 * @}
 */
