<?php
/**
 * @file
 * Provide views data and handlers for contact.module
 */

function contact_views_data() {
  $data['users']['contact'] = array(
    'field' => array(
      'title' => t('Link to contact page'),
      'help' => t('Provide a simple link to the user contact page.'),
      'handler' => 'views_handler_field_contact_link',
    ),
  );
  return $data;
}
/**
 * Implementation of hook_views_handlers() to register all of the basic handlers
 * views uses.
 */
function contact_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'views') . '/modules/contact',
    ),
    'handlers' => array(
      'views_handler_field_contact_link' => array(
        'parent' => 'views_handler_field_user_link',
      ),
    ),
  );
}
