<?php

/**
 * Implementation of hook_strongarm().
 */
function private_messages_strongarm() {
  $export = array();

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pm_email_notify_body';
  $strongarm->value = 'Hi !boincuser_name,

This is an automatic reminder from the site !site. You have received a new private message from !author_boincuser_name.

To read your message, follow this link:
!message

If you don\'t want to receive these emails again, change your preferences here:
!settings';
  $export['pm_email_notify_body'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pm_email_notify_default';
  $strongarm->value = 1;
  $export['pm_email_notify_default'] = $strongarm;

  $strongarm = new stdClass;
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'pm_email_notify_subject';
  $strongarm->value = 'New private message at !site.';
  $export['pm_email_notify_subject'] = $strongarm;

  return $export;
}
