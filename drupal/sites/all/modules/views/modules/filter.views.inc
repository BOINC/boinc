<?php
// $Id: filter.views.inc,v 1.1 2009/06/03 19:06:05 merlinofchaos Exp $
/**
 * @file
 * Provide basic views data for filter.module.
 */

/**
 * @defgroup views_filter_module filter.module handlers
 *
 * Only includes the table 'filter_formats'. 
 * @{
 */

/**
 * Implementation of hook_views_data()
 */
function filter_views_data() {
  // ----------------------------------------------------------------------
  // filter_formats table
  
  // Have not defined $data['filter_formats']['table']['group'] since
  // no fields are defined here yet.
  $data['filter_formats']['table']['join'] = array(
    'node_revisions' => array(
      'left_field' => 'format',
      'field' => 'format',
    ),
    'node' => array(
      'left_table' => 'node_revisions',
      'left_field' => 'format',
      'field' => 'format',
    ),
  );

  return $data;
}

function filter_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views') . '/modules/filter',
    ),
    'handlers' => array(
      // field handlers
      'views_handler_field_filter_format_name' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
}

/**
 * @}
 */
