<?php
// $Id: votingapi.views.inc,v 1.1.2.18 2009/08/15 17:51:38 eaton Exp $

/**
 * @file
 * Provide views data for votingapi.module.
 */

/**
 * @defgroup views_votingapi_module votingapi.module handlers.
 *
 * @{
 */


/**
 * Implementation of hook_views_handlers().
 */
function votingapi_views_handlers() {
  return array(
    'handlers' => array(
      'votingapi_views_handler_field_value' => array(
        'parent' => 'views_handler_field_numeric',
        'path' => drupal_get_path('module', 'votingapi') . '/views',
      ),
      'votingapi_views_handler_relationship' => array(
        'parent' => 'views_handler_relationship',
        'path' => drupal_get_path('module', 'votingapi') . '/views',
      ),
    ),
  );
}


/**
 * Implementation of hook_views_data().
 */
function votingapi_views_data() {
  // Basic table information.

  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['votingapi_vote']['table']['group']  = t('Votes');
  $data['votingapi_cache']['table']['group']  = t('Vote results');

  // ----------------------------------------------------------------
  // Fields

  // value
  $data['votingapi_vote']['value'] = array(
    'title' => t('Value'), // The item it appears as on the UI,
    'help' => t('The value of an individual cast vote.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'votingapi_views_handler_field_value',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
      'allow empty' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // value type
  $data['votingapi_vote']['value_type'] = array(
    'title' => t('Value type'), // The item it appears as on the UI,
    'help' => t('The nature of the vote being cast (points, percentage, etc).'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // tag
  $data['votingapi_vote']['tag'] = array(
    'title' => t('Tag'), // The item it appears as on the UI,
    'help' => t('An optional tag to group multi-criteria votes.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // uid
  $data['votingapi_vote']['uid'] = array(
    'title' => t('User'), // The item it appears as on the UI,
    'help' => t('The user who cast the vote.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_user',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_user_name',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_user_uid',
    ),
    'relationship' => array(
      'handler' => 'views_handler_relationship',
      'base' => 'users',
      'field' => 'uid',
      'label' => t('Individual voter'),
    ),
  );

  // uid
  $data['votingapi_vote']['current_uid'] = array(
    'title' => t('Current user'), // The item it appears as on the UI,
    'help' => t('Restrict votes to those cast by the current user.'), // The help that appears on the UI,
    'real field' => 'uid',
    'filter' => array(
      'handler' => 'views_handler_filter_user_current',
    ),
  );

  // vote_source
  $data['votingapi_vote']['vote_source'] = array(
    'title' => t('IP Address'), // The item it appears as on the UI,
    'help' => t('The IP address of the user who cast the vote.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );


  // timestamp
  $data['votingapi_vote']['timestamp'] = array(
    'title' => t('Timestamp'), // The item it appears as on the UI,
    'help' => t('The time the vote was cast.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );


  // Cache table fields

  // value
  $data['votingapi_cache']['value'] = array(
    'title' => t('Value'), // The item it appears as on the UI,
    'help' => t('The value of an individual cast vote.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'votingapi_views_handler_field_value',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
      'allow empty' => TRUE,
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // value type
  $data['votingapi_cache']['value_type'] = array(
    'title' => t('Value type'), // The item it appears as on the UI,
    'help' => t('The nature of the results in question (points, percentage, etc).'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );

  // tag
  $data['votingapi_cache']['tag'] = array(
    'title' => t('Tag'), // The item it appears as on the UI,
    'help' => t('An optional tag to group multi-criteria results.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );


  // function
  $data['votingapi_cache']['function'] = array(
    'title' => t('Function'), // The item it appears as on the UI,
    'help' => t('The aggregate function used to calculate the result.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );


  // timestamp
  $data['votingapi_cache']['timestamp'] = array(
    'title' => t('Timestamp'), // The item it appears as on the UI,
    'help' => t('The time the results were calculated.'), // The help that appears on the UI,
     // Information for displaying a title as a field
    'field' => array(
      'handler' => 'views_handler_field_date',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_date',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
  );
  return $data;
}

/**
 * @}
 */

function votingapi_views_data_alter(&$views_data) {
  // Add relationship handlers for both tables, for any base tables currently
  // available to Views.

  $default_relationships = module_invoke_all('votingapi_relationships');

  $default_relationships[] = array(
    'description' => t('nodes'),
    'content_type' => 'node',
    'base_table' => 'node',
    'content_id_column' => 'nid',
    'pseudo_vote' => 'votingapi_vote',    // for legacy compatability w/RC1.
    'pseudo_cache' => 'votingapi_cache',  // for legacy compatability w/RC1.
  );
  $default_relationships[] = array(
    'description' => t('comments'),
    'content_type' => 'comment',
    'base_table' => 'comments',
    'content_id_column' => 'cid',
    'pseudo_vote' => 'votingapi_vote',    // for legacy compatability w/RC1.
    'pseudo_cache' => 'votingapi_cache',  // for legacy compatability w/RC1.
  );
  $default_relationships[] = array(
    'description' => t('users'),
    'content_type' => 'user',
    'base_table' => 'users',
    'content_id_column' => 'uid',
    'pseudo_vote' => 'votingapi_vote',    // for legacy compatability w/RC1.
    'pseudo_cache' => 'votingapi_cache',  // for legacy compatability w/RC1.
  );

  foreach ($default_relationships as $data) {
    $pseudo = str_replace(array(' ','-','.'), '_', $data['content_type'] . '_' . $data['content_id_column']);
    $pseudo_vote = empty($data['pseudo_vote']) ? 'vapi_'. $pseudo : $data['pseudo_vote'];
    $pseudo_cache = empty($data['pseudo_cache']) ? 'vapic_'. $pseudo : $data['pseudo_cache'];
    
    $views_data[$data['base_table']][$pseudo_vote]['relationship'] = array(
      'title' => 'Votes',
      'help' => 'Votes cast by users on ' . $data['description'] . '.',
      'base' => 'votingapi_vote',
      'field' => 'content_id',
      'relationship field' => $data['content_id_column'],
      'handler' => 'votingapi_views_handler_relationship',
      'extra' => array(
        array(
          'field' => 'content_type',
          'value' => $data['content_type'],
          'numeric' => FALSE
        ),
      ),
    );

    $views_data[$data['base_table']][$pseudo_cache]['relationship'] = array(
      'title' => 'Vote results',
      'help' => 'Aggregate results of votes cast on ' . $data['description'] . '.',
      'base' => 'votingapi_cache',
      'field' => 'content_id',
      'relationship field' => $data['content_id_column'],
      'handler' => 'votingapi_views_handler_relationship',
      'extra' => array(
        array(
          'field' => 'content_type',
          'value' => $data['content_type'],
          'numeric' => FALSE
        ),
      ),
    );
  }
}


function _votingapi_views_sql_safe_content_type($string) {
  return str_replace(array(' ','-','.'), '_', $string);
}


/**
 * Implementation of hook_views_analyze().
 *
 * Warns admins if a VotingAPI relationship has been defined, but filters on the
 * relationship haven't been set.
 */
function votingapi_views_analyze($view) {
  $ret = array();
  // Check for something other than the default display:
  foreach ($view->display as $id => $display) {
    if (!$display->handler->is_defaulted('access') || !$display->handler->is_defaulted('filters')) {
      $relationships = $display->handler->get_option('relationships');
      foreach ($relationships as $relationship) {
        if ($relationship['field'] == 'votingapi_vote') {
          if (empty($relationship['votingapi']['value_type']) ||
              empty($relationship['votingapi']['tag'])) {
            $ret[] = views_ui_analysis(t('Display %display defines a VotingAPI vote relationship, but does not restrict the relationship data. This may result in duplicate entries in the resulting view.', array('%display' => $display->display_title)), 'warning');
          }
        }
        elseif ($relationship['field'] == 'votingapi_cache') {
          if (empty($relationship['votingapi']['value_type']) ||
              empty($relationship['votingapi']['tag']) ||
              empty($relationship['votingapi']['function'])) {
            $ret[] = views_ui_analysis(t('Display %display defines a VotingAPI results relationship, but does not restrict the relationship data. This may result in duplicate entries in the resulting view.', array('%display' => $display->display_title)), 'warning');
          }
        }
      }
    }
  }

  return $ret;
}
