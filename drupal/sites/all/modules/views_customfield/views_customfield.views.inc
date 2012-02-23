<?php
// $Id: views_customfield.views.inc,v 1.1.2.5 2009/01/09 18:32:33 casey Exp $

/**
 * Implementation of hook_views_handlers().
 *
 * @return array
 */
function views_customfield_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views_customfield') .'/includes',
    ),
    'handlers' => array(
      'views_customfield_handler_field_markup' => array(
        'parent' => 'views_handler_field',
      ),
      'views_customfield_handler_field_phpcode' => array(
        'parent' => 'views_handler_field',
      ),
      'views_customfield_handler_field_rownumber' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_data().
 *
 * @return array
 */
function views_customfield_views_data() {
  $data['customfield']['table']['group'] = t('Customfield');
  $data['customfield']['table']['join'] = array(
    '#global' => array(),
  );
  
  $data['customfield']['markup'] = array(
    'title' => t('Markup'),
    'help' => t('Display custom markup text.'),
    'field' => array(
      'handler' => 'views_customfield_handler_field_markup',
      'click sortable' => FALSE,
      'notafield' => TRUE,
    ),
  );

  $data['customfield']['phpcode'] = array(
    'title' => t('PHP code'),
    'help' => t('Display row-specific custom text using PHP.'),
    'field' => array(
      'handler' => 'views_customfield_handler_field_phpcode',
      'click sortable' => FALSE,
      'notafield' => TRUE,
    ),
  );
  
  $data['customfield']['rownumber'] = array(
    'title' => t('Rownumber'),
    'help' => t('Display rownumber.'),
    'field' => array(
      'handler' => 'views_customfield_handler_field_rownumber',
      'click sortable' => FALSE,
      'notafield' => TRUE,
    ),
  );
  
  return $data;
}