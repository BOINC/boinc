<?php
// $Id: views_or.views.inc,v 1.12 2009/05/14 21:13:16 darrenoh Exp $

/**
 * @file
 * Main Views file.
 *
 * This file tells Views what capabilities we provide and where classes are
 * stored.
 */

/**
 * Implementation of hook_views_data_alter().
 */
function views_or_views_data_alter(&$cache) {
  $cache['node']['views_or_begin'] = array(
    'group' => t('Views Or'),
    'title' => t('Begin alternatives'),
    'help' => t('When using Views Or, this starts a section of alternatives'),
    'filter' => array(
      'handler' => 'views_or_handler_filter_begin_alternatives',
    ),
    'field' => array(
      'handler' => 'views_or_handler_field_begin_alternatives',
    ),
    'argument' => array(
      'handler' => 'views_or_handler_argument_begin_alternatives',
    ),
  );
  $cache['node']['views_or_next'] = array(
    'group' => t('Views Or'),
    'title' => t('Next alternative'),
    'help' => t('When using Views Or, delimitates an alternative with a begin alternatives/end alternatives block'),
    'filter' => array(
      'handler' => 'views_or_handler_filter_next_alternative',
    ),
    'argument' => array(
      'handler' => 'views_or_handler_argument_next_alternative',
    ),
  );
  $cache['node']['views_or_end'] = array(
    'group' => t('Views Or'),
    'title' => t('End alternatives'),
    'help' => t('When using Views Or, finishes a block of alternatives.'),
    'filter' => array(
      'handler' => 'views_or_handler_filter_end_alternatives',
    ),
    'field' => array(
      'handler' => 'views_or_handler_field_end_alternatives',
    ),
    'argument' => array(
      'handler' => 'views_or_handler_argument_end_alternatives',
    ),
  );
  
  // PATCH: ADD SUPPORT FOR BOINC DATA
  $cache['user']['views_or_begin'] = array(
    'group' => t('Views Or'),
    'title' => t('Begin alternatives'),
    'help' => t('When using Views Or, this starts a section of alternatives'),
    'filter' => array(
      'handler' => 'views_or_handler_filter_begin_alternatives',
    ),
    'field' => array(
      'handler' => 'views_or_handler_field_begin_alternatives',
    ),
  );
  $cache['user']['views_or_next'] = array(
    'group' => t('Views Or'),
    'title' => t('Next alternative'),
    'help' => t('When using Views Or, delimitates an alternative with a begin alternatives/end alternatives block'),
    'filter' => array(
      'handler' => 'views_or_handler_filter_next_alternative',
    ),
  );
  $cache['user']['views_or_end'] = array(
    'group' => t('Views Or'),
    'title' => t('End alternatives'),
    'help' => t('When using Views Or, finishes a block of alternatives.'),
    'filter' => array(
      'handler' => 'views_or_handler_filter_end_alternatives',
    ),
    'field' => array(
      'handler' => 'views_or_handler_field_end_alternatives',
    ),
  );

  $cache['result']['views_or_begin'] = array(
    'group' => t('Views Or'),
    'title' => t('Begin alternatives'),
    'help' => t('When using Views Or, this starts a section of alternatives'),
    'filter' => array(
      'handler' => 'views_or_handler_filter_begin_alternatives',
    ),
    'field' => array(
      'handler' => 'views_or_handler_field_begin_alternatives',
    ),
  );
  $cache['result']['views_or_next'] = array(
    'group' => t('Views Or'),
    'title' => t('Next alternative'),
    'help' => t('When using Views Or, delimitates an alternative with a begin alternatives/end alternatives block'),
    'filter' => array(
      'handler' => 'views_or_handler_filter_next_alternative',
    ),
  );
  $cache['result']['views_or_end'] = array(
    'group' => t('Views Or'),
    'title' => t('End alternatives'),
    'help' => t('When using Views Or, finishes a block of alternatives.'),
    'filter' => array(
      'handler' => 'views_or_handler_filter_end_alternatives',
    ),
    'field' => array(
      'handler' => 'views_or_handler_field_end_alternatives',
    ),
  );
  
}

/**
 * Implementation of hook_views_handlers().
 */
function views_or_views_handlers() {
  return array(
    'handlers' => array(
      'views_or_handler_filter' => array(
        'parent' => 'views_handler_filter',
      ),
      'views_or_handler_filter_begin_alternatives' => array(
        'parent' => 'views_or_handler_filter',
        'file' => 'views_or_handler_filter.inc',
      ),
      'views_or_handler_filter_next_alternative' => array(
        'parent' => 'views_or_handler_filter',
        'file' => 'views_or_handler_filter.inc',
      ),
      'views_or_handler_filter_end_alternatives' => array(
        'parent' => 'views_or_handler_filter',
        'file' => 'views_or_handler_filter.inc',
      ),
      'views_or_handler_field' => array(
        'parent' => 'views_handler_field',
      ),
      'views_or_handler_field_begin_alternatives' => array(
        'parent' => 'views_or_handler_field',
        'file' => 'views_or_handler_field.inc',
      ),
      'views_or_handler_field_end_alternatives' => array(
        'parent' => 'views_or_handler_field',
        'file' => 'views_or_handler_field.inc',
      ),
      'views_or_handler_argument' => array(
        'parent' => 'views_handler_argument',
      ),
      'views_or_handler_argument_begin_alternatives' => array(
        'parent' => 'views_or_handler_argument',
        'file' => 'views_or_handler_argument.inc',
      ),
      'views_or_handler_argument_next_alternative' => array(
        'parent' => 'views_or_handler_argument',
        'file' => 'views_or_handler_argument.inc',
      ),
      'views_or_handler_argument_end_alternatives' => array(
        'parent' => 'views_or_handler_argument',
        'file' => 'views_or_handler_argument.inc',
      ),
    ),
  );
}
