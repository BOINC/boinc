<?php
// $Id: flag.views.inc,v 1.1.2.9 2008/12/03 14:09:59 mooffie Exp $

/**
 * @file
 * Provides support for the Views module.
 */

/**
 * Implementation of hook_views_handlers().
 */
function flag_views_handlers() {
  return array(
    'info' => array(
      'path' => drupal_get_path('module', 'flag') . '/includes',
    ),
    'handlers' => array(
      'flag_handler_relationship_content' => array(
        'parent' => 'views_handler_relationship',
        'file' => 'flag_handler_relationships.inc',
      ),
      'flag_handler_relationship_counts' => array(
        'parent' => 'views_handler_relationship',
        'file' => 'flag_handler_relationships.inc',
      ),
      'flag_handler_relationship_user_content' => array(
        'parent' => 'views_handler_relationship',
        'file' => 'flag_handler_relationships.inc',
      ),
      'flag_handler_field_ops' => array(
        'parent' => 'views_handler_field',
      ),
      'flag_handler_filter_flagged' => array(
        'parent' => 'views_handler_filter_boolean_operator',
      ),
      'flag_handler_argument_content_id' => array(
        'parent' => 'views_handler_argument_numeric',
      ),
    ),
  );
}

/**
 * Implementation of hook_views_plugins().
 */
function flag_views_plugins() {
  return array(
    'argument validator' => array(
      'flag_flaggable_node' => array(
        'title' => t('Flaggable node'),
        'flag type' => 'node',
        'handler' => 'flag_plugin_argument_validate_flaggability',
        'path' => drupal_get_path('module', 'flag') . '/includes',
      ),
      'flag_flaggable_user' => array(
        'title' => t('Flaggable user'),
        'flag type' => 'user',
        'handler' => 'flag_plugin_argument_validate_flaggability',
        'path' => drupal_get_path('module', 'flag') . '/includes',
      ),
      // A comment validator won't be very useful. Moreover, having it will
      // contribute to the argument object's $options polution, so let's skip
      // it.
    ),
  );
}
/**
 * Implementation of hook_views_data().
 */
function flag_views_data() {
  $data = array();

  $data['flag_content']['table']['group'] = t('Flags');
  $data['flag_counts']['table']['group'] = t('Flags');

  $data['flag_content']['uid'] = array(
    'title' => t('User'),
    'help' => t('The user that flagged an item.'),
    'relationship' => array(
      'base' => 'users',
      'handler' => 'views_handler_relationship',
      'label' => t('Flag user'),
    ),
  );

  $data['flag_content']['timestamp'] = array(
    'title' => t('Flagged time'),
    'help' => t('Display the time the content was flagged by a user.'),
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

  // Argument for content ID, used for "Who's flagged this" views.
  $data['flag_content']['content_id'] = array(
    'title' => t('Content ID'),
    'help' => t('The unique ID of the object that has been flagged.'),
    'argument' => array(
      'handler' => 'flag_handler_argument_content_id',
    ),
  );

  // Specialized is null/is not null filter.
  $data['flag_content']['flagged'] = array(
    'title' => t('Flagged'),
    'help' => t('Filter to ensure content has or has not been flagged.'),
    'real field' => 'uid',
    'filter' => array(
      'handler' => 'flag_handler_filter_flagged',
      'label' => t('Flagged'),
    ),
  );

  // Flag content links.
  $data['flag_content']['ops'] = array(
    'title' => t('Flag link'),
    'help' => t('Display flag/unflag link.'),
    'field' => array(
      'handler' => 'flag_handler_field_ops',
    ),
  );

  $data['flag_counts']['count'] = array(
    'title' => t('Flag counter'),
    'help' => t('The number of times a piece of content is flagged by any user.'),
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
    'argument' => array(
      'handler' => 'views_handler_argument_numeric',
    ),
  );

  return $data;
}

/**
 * Implementation of hook_views_data_alter().
 */
function flag_views_data_alter(&$data) {

  foreach (array_keys(flag_fetch_definition()) as $flag_type) {
    $flag = flag_flag::factory_by_content_type($flag_type);
    $info = $flag->get_views_info();

    // Add the flag relationship.
    $data[$info['views table']]['flag_content_rel'] = array(
      'group' => t('Flags'),
      'title' => $info['title'],
      'help' => $info['help'],
      'relationship' => array(
        'flag type' => $flag_type,
        'handler' => 'flag_handler_relationship_content',
        'label' => t('flag'),
        'base' => 'flag_content',
        'base field' => 'content_id',
        'relationship field' => $info['join field'],
      ),
    );

    // Add the flag counter relationship.
    $data[$info['views table']]['flag_count_rel'] = array(
      'group' => t('Flags'),
      'title' => $info['counter title'],
      'help' => $info['counter help'],
      'relationship' => array(
        'flag type' => $flag_type,
        'handler' => 'flag_handler_relationship_counts',
        'label' => t('counter'),
        'base' => 'flag_counts',
        'base field' => 'content_id',
        'relationship field' => $info['join field'],
      ),
    );
  }

  // Add a relationship for the user that flagged any type of content.
  $data['users']['flag_user_content_rel'] = array(
    'group' => t('Flags'),
    'title' => t("User's flagged content"),
    'help' => t('Limit results to users that have flagged certain content.'),
    'relationship' => array(
      'base' => 'flag_content',
      'base field' => 'uid',
      'relationship field' => 'uid',
      'handler' => 'flag_handler_relationship_user_content',
      'label' => t('user flagged content'),
    ),
  );
}

/**
 * A helper function that creates a radio list of available flags.
 *
 * This function is used to select the desired flag when setting up flag
 * relationships and fields.
 */
function flag_views_flag_config_form($form_type, $content_type, $current_flag) {
  $flags = flag_get_flags($content_type);

  $options = array();
  foreach ($flags as $flag) {
    $options[$flag->name] = $flag->get_title();
  }

  $form = array(
    '#type' => $form_type,
    '#title' => t('Flag'),
    '#options' => $options,
    '#default_value' => $current_flag,
    '#required' => TRUE,
  );

  return $form;
}

/**
 * Helper function that gets the first defined flag and returns its name.
 */
function flag_views_flag_default($content_type) {
  static $default_flag = array();

  if (!array_key_exists($content_type, $default_flag)) {
    $flag = array_shift(flag_get_flags($content_type));
    $default_flag[$content_type] = $flag ? $flag->name : NULL;
  }

  return $default_flag[$content_type];
}
