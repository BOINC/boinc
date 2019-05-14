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

function boincuser_rules_event_info() {
  return array(
    'boincuser_change_email_error' => array(
      'label' => bts('Error from user changing e-mail address.', array(), NULL, 'boinc:rule-event'),
      'module' => 'Boincuser',
      'arguments' => rules_events_hook_boincuser_chemail_arguments(),
    ),
    'boincuser_revert_email_error' => array(
      'label' => bts('Error from user reverting to previous e-mail address.', array(), NULL, 'boinc:rule-event'),
      'module' => 'Boincuser',
      'arguments' => rules_events_hook_boincuser_chemail_arguments(),
    ),
    'boincuser_general_consent_error' => array(
      'label' => bts('Error from user consenting to terms-of-use.', array(), NULL, 'boinc:rule-event'),
      'module' => 'Boincuser',
      'arguments' => rules_events_hook_boincuser_adminnotify_argument(),
    ),
    'boincuser_general_consent_type_error' => array(
      'label' => bts('Error dealing with missing consent type.', array(), NULL, 'boinc:rule-event'),
      'module' => 'Boincuser',
      'arguments' => rules_events_hook_boincuser_consenttype_argument(),
    ),
  );
}

/**
 * Arguments suitable for boincuser
 **/
function rules_events_hook_boincuser_chemail_arguments() {
  return array(
    'new_email_addr' => array(
      'type' => 'string',
      'label' => 'New email address',
      'required' => TRUE,
    ),
  ) + rules_events_hook_boincuser_adminnotify_argument();
}

function rules_events_hook_boincuser_consenttype_argument() {
  return array(
    'consent_type' => array(
      'type' => 'string',
      'label' => 'Consent type id that caused the error',
      'required' => TRUE,
    ),
  ) + rules_events_hook_boincuser_adminnotify_argument();
}

function rules_events_hook_boincuser_adminnotify_argument() {
  return array(
    'subject_tag' => array(
      'type' => 'string',
      'label' => 'Subject tag for email filtering',
      'required' => TRUE,
    ),
  ) + rules_events_global_user_argument();
}