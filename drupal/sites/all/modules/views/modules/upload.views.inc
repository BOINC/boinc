<?php
// $Id: upload.views.inc,v 1.16.2.1 2009/11/02 23:25:14 merlinofchaos Exp $
/**
 * @file
 *
 * Provide views data and handlers for upload tables that are not represented by
 * their own module.
 */

/**
 * @defgroup views_upload_module upload.module handlers
 *
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function upload_views_data() {
  $data = array();

  // ----------------------------------------------------------------------
  // upload table

  $data['upload']['table']['group']  = t('Upload');

  $data['upload']['table']['join'] = array(
    'node' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'node_revisions' => array(
      'left_field' => 'vid',
      'field' => 'vid',
    ),
    'files' => array(
      'left_field' => 'fid',
      'field' => 'fid',
    ),
  );

  $data['upload']['vid'] = array(
    'title' => t('Node'),
    'help' => t('The node the uploaded file is attached to'),
    'relationship' => array(
      'label' => t('upload'),
      'base' => 'node',
      'base field' => 'vid',
      // This allows us to not show this relationship if the base is already
      // node so users won't create circular relationships.
      'skip base' => array('node', 'node_revisions'),
    ),
  );

  $data['upload']['description'] = array(
    'title' => t('Description'),
    'help' => t('The description of the uploaded file.'),
    'field' => array(
      'handler' => 'views_handler_field_upload_description',
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

  $data['upload']['list'] = array(
    'title' => t('Listed'),
    'help' => t('Whether or not the file is marked to be listed.'),
    'field' => array(
      'handler' => 'views_handler_field_boolean',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_boolean_operator',
      'label' => t('Published'),
      'type' => 'yes-no',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  $data['upload']['weight'] = array(
    'title' => t('Weight'),
    'help' => t('The weight, used for sorting.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
      'click sortable' => TRUE,
     ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  return $data;
}

/**
 * Implementation of hook_views_data_alter()
 */
function upload_views_data_alter(&$data) {
  $data['node']['upload_fid'] = array(
    'group' => t('Upload'),
    'title' => t('Attached files'),
    'help' => t('All files attached to a node with upload.module.'),
    'real field' => 'vid',
    'field' => array(
      'handler' => 'views_handler_field_upload_fid',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_upload_fid',
      'title' => t('Has attached files'),
      'type' => 'yes-no',
      'help' => t('Only display items with attached files. This can cause duplicates if there are multiple attached files.'),
    ),
    'relationship' => array(
      'title' => t('Attached files'),
      'help' => t('Add a relationship to gain access to more file data for files uploaded by upload.module. Note that this relationship will cause duplicate nodes if there are multiple files attached to the node.'),
      'relationship table' => 'upload',
      'relationship field' => 'fid',
      'base' => 'files',
      'base field' => 'fid',
      'handler' => 'views_handler_relationship',
      'label' => t('Files'),
    ),
  );
}

/**
 * Implementation of hook_views_handlers() to register all of the basic handlers
 * views uses.
 */
function upload_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views') . '/modules/upload',
    ),
    'handlers' => array(
      'views_handler_field_upload_fid' => array(
        'parent' => 'views_handler_field_prerender_list',
      ),
      'views_handler_field_upload_description' => array(
        'parent' => 'views_handler_field',
      ),
      'views_handler_filter_upload_fid' => array(
        'parent' => 'views_handler_filter_boolean_operator',
      ),
    ),
  );
}

/**
 * @}
 */
