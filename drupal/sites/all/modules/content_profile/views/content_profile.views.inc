<?php
// $Id: content_profile.views.inc,v 1.1.2.1 2008/09/26 16:09:43 fago Exp $

/**
 * @file content_profile.views.inc
 * Provides support for the Views module.
 */

/**
 * Implementation of hook_views_data_alter().
 */
function content_profile_views_data_alter(&$data) {
  // node relationship for profiles
  $data['users']['content_profile_rel'] = array(
    'group' => t('Node'),
    'title' => t('Content Profile'),
    'help' => t('Create a relationship to a content profile of the user.'),
    'relationship' => array(
      'handler' => 'content_profile_views_handler_relationship',
      'base' => 'node',
      'base field' => 'uid',
      'label' => t('Content Profile'),
    ),
  );
}


/**
 * Implementation of hook_views_handlers().
 */
function content_profile_views_handlers() {
  return array(
  'info' => array(
    'path' => drupal_get_path('module', 'content_profile') .'/views',
    ),
  'handlers' => array(
    'content_profile_views_handler_relationship' => array(
     'parent' => 'views_handler_relationship',
     ),
    ),
  );
}