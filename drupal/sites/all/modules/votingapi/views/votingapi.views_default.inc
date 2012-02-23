<?php
// $Id: votingapi.views_default.inc,v 1.1.2.6 2008/12/04 18:34:39 eaton Exp $

/**
 * @file
 * Provide default views data for votingapi.module.
 */

function votingapi_views_default_views() {
  $views = array();
    
  $view = new view;
  $view->name = 'top_content';
  $view->description = 'Top rated content';
  $view->tag = 'votingapi';
  $view->view_php = '';
  $view->base_table = 'node';
  $view->is_cacheable = FALSE;
  $view->api_version = 2;
  $view->disabled = TRUE; /* Edit this to true to make a default view disabled initially */
  $handler = $view->new_display('default', 'Defaults', 'default');
  $handler->override_option('fields', array(
    'name' => array(
      'label' => 'Author',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'relationship' => 'none',
    ),
    'title' => array(
      'label' => 'Title',
      'link_to_node' => 1,
      'exclude' => 0,
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'relationship' => 'none',
    ),
    'created' => array(
      'label' => 'Posted',
      'date_format' => 'time ago',
      'custom_date_format' => '1',
      'exclude' => 0,
      'id' => 'created',
      'table' => 'node',
      'field' => 'created',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'operator' => '=',
      'value' => 1,
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('access', array(
    'type' => 'none',
  ));
  $handler->override_option('title', 'Top rated content');
  $handler->override_option('use_pager', '1');
  $handler->override_option('style_plugin', 'table');
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 1,
    'order' => 'desc',
    'columns' => array(
      'name' => 'name',
      'title' => 'title',
      'created' => 'created',
    ),
    'info' => array(
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'title' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'created' => array(
        'sortable' => 1,
        'separator' => '',
      ),
    ),
    'default' => 'title',
  ));
  $handler = $view->new_display('page', 'Fivestar', 'page_1');
  $handler->override_option('relationships', array(
    'votingapi_cache' => array(
      'label' => 'Voting results',
      'required' => 0,
      'votingapi' => array(
        'value_type' => 'percent',
        'tag' => 'vote',
        'function' => 'average',
      ),
      'id' => 'votingapi_cache',
      'table' => 'node',
      'field' => 'votingapi_cache',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('fields', array(
    'name' => array(
      'label' => 'Author',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'relationship' => 'none',
    ),
    'title' => array(
      'label' => 'Title',
      'link_to_node' => 1,
      'exclude' => 0,
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'relationship' => 'none',
    ),
    'created' => array(
      'label' => 'Posted',
      'date_format' => 'time ago',
      'custom_date_format' => '1',
      'exclude' => 0,
      'id' => 'created',
      'table' => 'node',
      'field' => 'created',
      'relationship' => 'none',
    ),
    'value' => array(
      'label' => 'Rating',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => '',
      'appearance' => 'fivestar_views_value_display_handler',
      'exclude' => 0,
      'id' => 'value',
      'table' => 'votingapi_cache',
      'field' => 'value',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'votingapi_cache',
    ),
  ));
  $handler->override_option('sorts', array(
    'value' => array(
      'order' => 'DESC',
      'id' => 'value',
      'table' => 'votingapi_cache',
      'field' => 'value',
      'relationship' => 'votingapi_cache',
      'override' => array(
        'button' => 'Use default',
      ),
    ),
  ));
  $handler->override_option('header', 'This page is a demonstration of VotingAPI\'s integration with the Views module. It\'s set up to display nodes sorted by percentage ratings, the style used by Fivestar module.');
  $handler->override_option('header_format', '1');
  $handler->override_option('header_empty', 0);
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 1,
    'order' => 'desc',
    'columns' => array(
      'name' => 'name',
      'title' => 'title',
      'created' => 'created',
      'value' => 'value',
    ),
    'info' => array(
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'title' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'created' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'value' => array(
        'sortable' => 1,
        'separator' => '',
      ),
    ),
    'default' => 'value',
  ));
  $handler->override_option('path', 'top-rated-percent');
  $handler->override_option('menu', array(
    'type' => 'normal',
    'title' => 'Top content (Fivestar)',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'weight' => 0,
  ));
  $handler = $view->new_display('page', 'Plus1', 'page_2');
  $handler->override_option('relationships', array(
    'votingapi_cache' => array(
      'label' => 'Voting results',
      'required' => 0,
      'votingapi' => array(
        'value_type' => 'points',
        'tag' => 'vote',
        'function' => 'sum',
      ),
      'id' => 'votingapi_cache',
      'table' => 'node',
      'field' => 'votingapi_cache',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('fields', array(
    'name' => array(
      'label' => 'Author',
      'link_to_user' => 1,
      'exclude' => 0,
      'id' => 'name',
      'table' => 'users',
      'field' => 'name',
      'relationship' => 'none',
    ),
    'title' => array(
      'label' => 'Title',
      'link_to_node' => 1,
      'exclude' => 0,
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'relationship' => 'none',
    ),
    'created' => array(
      'label' => 'Posted',
      'date_format' => 'time ago',
      'custom_date_format' => '1',
      'exclude' => 0,
      'id' => 'created',
      'table' => 'node',
      'field' => 'created',
      'relationship' => 'none',
    ),
    'value' => array(
      'label' => 'Rating',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => ' points',
      'appearance' => '',
      'exclude' => 0,
      'id' => 'value',
      'table' => 'votingapi_cache',
      'field' => 'value',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'votingapi_cache',
    ),
  ));
  $handler->override_option('sorts', array(
    'value' => array(
      'order' => 'DESC',
      'id' => 'value',
      'table' => 'votingapi_cache',
      'field' => 'value',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'votingapi_cache',
    ),
  ));
  $handler->override_option('header', 'This page is a demonstration of VotingAPI\'s integration with the Views module. It\'s set up to display nodes sorted by point ratings, the style used by the Plus1 and VoteUpDown modules.');
  $handler->override_option('header_format', '1');
  $handler->override_option('header_empty', 0);
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 1,
    'order' => 'desc',
    'columns' => array(
      'name' => 'name',
      'title' => 'title',
      'created' => 'created',
      'value' => 'value',
    ),
    'info' => array(
      'name' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'title' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'created' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'value' => array(
        'sortable' => 1,
        'separator' => '',
      ),
    ),
    'default' => 'value',
  ));
  $handler->override_option('path', 'top-rated-points');
  $handler->override_option('menu', array(
    'type' => 'normal',
    'title' => 'Top content (Plus1)',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'weight' => 0,
  ));
  $handler = $view->new_display('page', 'Current user', 'page_3');
  $handler->override_option('relationships', array(
    'votingapi_vote' => array(
      'label' => 'Individual votes',
      'required' => 1,
      'votingapi' => array(
        'value_type' => '',
        'tag' => '',
      ),
      'current_user' => 0,
      'id' => 'votingapi_vote',
      'table' => 'node',
      'field' => 'votingapi_vote',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'none',
    ),
  ));
  $handler->override_option('fields', array(
    'title' => array(
      'label' => 'Title',
      'link_to_node' => 1,
      'exclude' => 0,
      'id' => 'title',
      'table' => 'node',
      'field' => 'title',
      'relationship' => 'none',
    ),
    'value_type' => array(
      'label' => 'Vote type',
      'exclude' => 0,
      'id' => 'value_type',
      'table' => 'votingapi_vote',
      'field' => 'value_type',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'votingapi_vote',
    ),
    'value' => array(
      'label' => 'Value',
      'set_precision' => FALSE,
      'precision' => 0,
      'decimal' => '.',
      'separator' => ',',
      'prefix' => '',
      'suffix' => '',
      'appearance' => '',
      'exclude' => 0,
      'id' => 'value',
      'table' => 'votingapi_vote',
      'field' => 'value',
      'override' => array(
        'button' => 'Override',
      ),
      'relationship' => 'votingapi_vote',
    ),
    'timestamp' => array(
      'label' => 'Timestamp',
      'date_format' => 'time ago',
      'custom_date_format' => '1',
      'exclude' => 0,
      'id' => 'timestamp',
      'table' => 'votingapi_vote',
      'field' => 'timestamp',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'votingapi_vote',
    ),
  ));
  $handler->override_option('sorts', array(
    'timestamp' => array(
      'order' => 'DESC',
      'id' => 'timestamp',
      'table' => 'votingapi_vote',
      'field' => 'timestamp',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'votingapi_vote',
    ),
  ));
  $handler->override_option('arguments', array(
    'uid' => array(
      'default_action' => 'default',
      'style_plugin' => 'default_summary',
      'style_options' => array(),
      'wildcard' => 'all',
      'wildcard_substitution' => 'Everyone',
      'title' => '%1\'s votes',
      'default_argument_type' => 'current_user',
      'default_argument' => '',
      'validate_type' => 'none',
      'validate_fail' => 'not found',
      'break_phrase' => 0,
      'not' => 0,
      'id' => 'uid',
      'table' => 'votingapi_vote',
      'field' => 'uid',
      'relationship' => 'votingapi_vote',
      'override' => array(
        'button' => 'Use default',
      ),
      'default_options_div_prefix' => '',
      'default_argument_user' => 0,
      'default_argument_fixed' => '',
      'default_argument_php' => '',
      'validate_argument_node_type' => array(
        'page' => 0,
        'story' => 0,
      ),
      'validate_argument_node_access' => 0,
      'validate_argument_nid_type' => 'nid',
      'validate_argument_vocabulary' => array(),
      'validate_argument_type' => 'tid',
      'validate_argument_php' => '',
    ),
  ));
  $handler->override_option('filters', array(
    'status' => array(
      'operator' => '=',
      'value' => 1,
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'status',
      'table' => 'node',
      'field' => 'status',
      'relationship' => 'none',
    ),
    'value' => array(
      'operator' => 'not empty',
      'value' => array(
        'value' => '',
        'min' => '',
        'max' => '',
      ),
      'group' => '0',
      'exposed' => FALSE,
      'expose' => array(
        'operator' => FALSE,
        'label' => '',
      ),
      'id' => 'value',
      'table' => 'votingapi_vote',
      'field' => 'value',
      'override' => array(
        'button' => 'Use default',
      ),
      'relationship' => 'votingapi_vote',
    ),
  ));
  $handler->override_option('title', 'Your votes');
  $handler->override_option('style_options', array(
    'grouping' => '',
    'override' => 1,
    'sticky' => 1,
    'order' => 'desc',
    'columns' => array(
      'title' => 'title',
      'value_type' => 'value_type',
      'value' => 'value',
      'timestamp' => 'timestamp',
    ),
    'info' => array(
      'title' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'value_type' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'value' => array(
        'sortable' => 1,
        'separator' => '',
      ),
      'timestamp' => array(
        'sortable' => 1,
        'separator' => '',
      ),
    ),
    'default' => 'timestamp',
  ));
  $handler->override_option('path', 'your-votes');
  $handler->override_option('menu', array(
    'type' => 'normal',
    'title' => 'Your votes',
    'weight' => '0',
    'name' => 'navigation',
  ));
  $handler->override_option('tab_options', array(
    'type' => 'none',
    'title' => '',
    'weight' => 0,
  ));


  $views[$view->name] = $view;
  return $views;
}
