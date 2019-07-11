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

function boincwork_rules_event_info() {
  return array(
    'boincwork_circular_merge_error' => array(
      'label' => bts('A circular merge of hosts was attempted with error.', array(), NULL, 'boinc:rule-event'),
      'module' => 'BOINCwork',
      'arguments' => rules_events_hook_boincwork_arguments(),
    ),
    'boincwork_zombie_merge_error' => array(
      'label' => bts('A host merge into a zombie host was attempted with error.', array(), NULL, 'boinc:rule-event'),
      'module' => 'BOINCwork',
      'arguments' => rules_events_hook_boincwork_arguments(),
    ),
  );
}

/**
 * Arguments suitable for boincwork
 **/
function rules_events_hook_boincwork_arguments() {
  return array(
    'oldhost_id' => array(
      'type' => 'number',
      'label' => 'old host id',
      'required' => TRUE,
    ),
    'targethost_id' => array(
      'type' => 'number',
      'label' => 'target host id',
      'required' => TRUE,
    ),
  ) + rules_events_hook_boincwork_adminnotify_argument();
}

function rules_events_hook_boincwork_adminnotify_argument() {
  return array(
    'subject_tag' => array(
      'type' => 'string',
      'label' => 'Subject tag for email filtering',
      'required' => TRUE,
    ),
  ) + rules_events_global_user_argument();
}