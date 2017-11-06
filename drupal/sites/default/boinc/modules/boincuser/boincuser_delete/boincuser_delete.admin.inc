<?php

/**
 * @file
 * BOINC user delete - Administration page
 *
 */

/**
 * Administrative settings page
 *
 * @return array
 *   a form array
 */
function boincuser_delete_settings() {
  $form = array();

  $default = array(
    'boincuser_delete_redirect' => variable_get('boincuser_delete_redirect', '<front>'),
    'boincuser_delete_grace_period' => variable_get('boincuser_delete_grace_period', 60*60*24*7*2), // Default is two weeks
  );

  $grace_period_options = array(
    60*60*24*7 => format_interval(60*60*24*7, 2),
    60*60*24*7*2 => format_interval(60*60*24*7*2, 2),
    60*60*24*7*4 => format_interval(60*60*24*7*4, 2),
    60*60*24*7*8 => format_interval(60*60*24*7*8, 2),
    60*60*24*7*12 => format_interval(60*60*24*7*12, 2),
    60*60*24*7*16 => format_interval(60*60*24*7*16, 2),
    60*60*24*7*24 => format_interval(60*60*24*7*24, 2),
  );

  $form['redirect'] = array(
    '#type' => 'fieldset',
    '#title' => t('Redirect'),
  );
  $form['redirect']['boincuser_delete_redirect'] = array(
    '#type' => 'textfield',
    '#title' => t('Redirection page'),
    '#default_value' => $default['boincuser_delete_redirect'],
    '#description' => t('Choose where to redirect your users after account deletion. Any valid Drupal path will do, e.g. %front or %node', array('%front' => '<front>', '%node' => 'node/1')),
  );
  $form['grace'] = array(
    '#type' => 'fieldset',
    '#title' => t('Grace Period'),
  );
  $form['grace']['boincuser_delete_grace_period'] = array(
    '#type' => 'select',
    '#title' => t('Grace Period before account is disabled/deleted'),
    '#default_value' => $default['boincuser_delete_grace_period'],
    '#options' => $grace_period_options,
    '#description' => t('The time frame after which the user\'s account will be disabled or deleted.'),
  );

  return system_settings_form($form);
}
