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

function boincuser_delete_rules_event_info() {
  return array(
    'boincuser_delete_general_error' => array(
      'label' => bts('Error from user deleting own account.', array(), NULL, 'boinc:rule-event'),
      'module' => 'Boincuser_Delete',
      'arguments' => rules_events_hook_boincuser_adminnotify_argument(),
    ),
    'boincuser_delete_uid_invalid_error' => array(
      'label' => bts('Error from user deleting own account. User ID is invalid', array(), NULL, 'boinc:rule-event'),
      'module' => 'Boincuser_Delete',
      'arguments' => rules_events_hook_boincuser_adminnotify_argument(),
    ),
  );
}

// function rules_events_hook_boincuser_adminnotify_argument is located in boincuser_rules.inc