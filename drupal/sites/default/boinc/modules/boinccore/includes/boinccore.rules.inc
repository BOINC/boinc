<?php
// $Id$

/**
 * Rules: Custom defined events for BOINC
 */

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Hooks into rules module
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Implementation of hook_rules_event_info()
 *
 * Defines the custom events.
 */

function boinccore_rules_event_info() {
  return array(
    'boinccore_comment_convert' => array(
      'label' => bts('A comment was converted to a new forum topic.', array(), NULL, 'boinc:rule-event'),
      'module' => 'BOINCcore',
      'arguments' => rules_events_hook_comment_arguments(t('converted comment')),
    ),
    'boinccore_comment_unhidden' => array(
      'label' => bts('A comment was unhidden.', array(), 'boinc:rule-event'),
      'module' => 'BOINCcore',
      'arguments' => rules_events_hook_comment_arguments(t('unhidden comment')),
    ),
  );
}