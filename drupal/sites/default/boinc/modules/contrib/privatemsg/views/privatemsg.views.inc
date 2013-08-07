<?php
/**
 * @file
 * Provide views data and handlers for privatemsg.module
 */

function privatemsg_views_data() {
  $data['users']['privatemsg_link'] = array(
    'group' => t('Privatemsg'),
    'title' => t('Send message'),
    'field' => array(
      'title' => t('Send Privatemsg link'),
      'help' => t('Displays a link to send a private message to a user.'),
      'handler' => 'views_handler_field_privatemsg_link',
    ),
  );
  return $data;
}


/**
 * Implementation of hook_views_handlers().
 */
function privatemsg_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'privatemsg') . '/views',
    ),
    'handlers' => array(
      // field handlers
      'views_handler_field_privatemsg_link' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
}



