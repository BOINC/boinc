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

function boincteam_rules_event_info() {
  return array(
    'boincteam_create_team_error' => array(
      'label' => bts('Error creating a new BOINC team - general error.', array(), NULL, 'boinc:rule-event'),
      'module' => 'Boincteam',
      'arguments' => rules_events_hook_boincteam_arguments(),
    ),
    'boincteam_create_team_nopathauto_error' => array(
      'label' => bts('Error creating a new BOINC team - no pathauto module.', array(), NULL, 'boinc:rule-event'),
      'module' => 'Boincteam',
      'arguments' => rules_events_hook_boincteam_arguments(),
    ),
  );
}

/**
 * Arguments suitable for boincteam
 **/
function rules_events_hook_boincteam_arguments() {
  return array(
    'team_name' => array(
      'type' => 'string',
      'label' => 'team name',
      'required' => TRUE,
    ),
  ) + rules_events_hook_boincteam_adminnotify_argument();
}

function rules_events_hook_boincteam_adminnotify_argument() {
  return array(
    'subject_tag' => array(
      'type' => 'string',
      'label' => 'Subject tag for email filtering',
      'required' => TRUE,
    ),
  ) + rules_events_global_user_argument();
}