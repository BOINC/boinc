<?php
// $Id: flag_friend.views.inc,v 1.1.2.3 2009/06/03 01:02:56 sirkitree Exp $

/**
 * @file flag_friend.views.inc
 *
 * Provide Flag Friend data to Views.
 */

/**
 * Implementation of hook_views_handlers().
 */
function flag_friend_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'flag_friend') .'/includes',
    ),
    'handlers' => array(
      'flag_friend_handler_argument_numeric' => array(
        'parent' => 'views_handler_argument_numeric',
      ),
      'flag_friend_handler_argument_apachesolr_friend' => array(
        'parent' => 'apachesolr_views_handler_argument',
      ),
      'flag_friend_handler_field_links' => array(
        'parent' => 'views_handler_field',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_data().
 */
function flag_friend_views_data() {
  $data = array();
  
  // flag_friend table
  $data['flag_friend']['table']['group'] = t('Flag friend');
  $data['flag_friend']['table']['join'] = array(
    'users' => array(
      'left_field' => 'uid',
      'field' => 'uid',
    ),
  );
  
  $data['flag_friend']['uid'] = array(
    'group' => t('Flag friend'),
    'title' => t('Flag friend links'),
    'help' => t('Remove/Deny links'),
    'real field' => 'uid', 
    'field' => array(
      'handler' => 'flag_friend_handler_field_links',
    ),
  );
  
  // argument
  $data['flag_friend']['friend_uid'] = array(
    'title' => t('Friends of'),
    'help' => t('Retrieve the friends of the user id given.'),
    'argument' => array(
      'handler' => 'flag_friend_handler_argument_numeric',
    ),
  );
  
  // field, sort, filter, argument
  $data['flag_friend']['created'] = array(
    'title' => t('Relationship created time'),
    'help' => t('Display the time the relationship was approved.'),
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
    'argument' => array(
      'handler' => 'views_handler_argument_date',
    ),
  );
  
  // flag_friend_message table
  $data['flag_friend_message']['table']['group'] = t('Flag friend');
  $data['flag_friend_message']['table']['join'] = array(
    'flag_content' => array(
      'left_field' => 'fcid',
      'field' => 'fcid',
    ),
  );
  
  // field
  $data['flag_friend_message']['message'] = array(
    'title' => t('Message'),
    'help' => t('Display the message for any friend flags. (Only useful for a list of pending friend requests.)'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => FALSE,
    ),
  );
  
  // Add the flag relationship.
  // This basically just changes the 'base field' while reusing Flag's 
  // relationship handler. In effect, we can display the user who flagged
  // the content we specify, instead of the user who is flagged.
  $data['users']['flag_friend_content_rel'] = array(
    'group' => t('Flag Friend'),
    'title' => t('User flag'),
    'help' => t('Limit results to only those users flagged by a certain flag; Displays the user who flagged this content.'),
    'relationship' => array(
      'flag type' => 'user',
      'handler' => 'flag_handler_relationship_content',
      'label' => t('flag friend'),
      'base' => 'flag_content',
      'base field' => 'uid',
      'relationship field' => 'uid',
    ),
  );
  
  return $data;
}

/**
 * Implementation of hook_views_data_alter().
 */
function flag_friend_views_data_alter(&$data) {
  if (module_exists('apachesolr_views')) {
    $data['apachesolr']['friends'] = array(
      'title' => t('Friends of'),
      'help' => t('Retrieve the friends of the user id given.'),
      'argument' => array(
        'handler' => 'flag_friend_handler_argument_apachesolr_friend',
        'field' => 'uid',
      ),
    );
  }
}