<?php
// $Id$

/**
* @file
* Administration page callbacks for the boinccore module.
*/

/**
  * The BOINC community form allows the setting of requirements for interaction
  * in the community sections of the site.
  */
function boinccore_admin_community(&$form_state) {
  $form = array();
  $default = array(
    'min_credit_to_post' => variable_get('boinc_comment_min_credit', 0),
  );
  
  // Define the form
  $form['boinc_comment_min_credit'] = array(
    '#type' => 'textfield',
    '#title' => t('Credit required to post comments'),
    '#default_value' => $default['min_credit_to_post'],
    '#description' => t('Set a minimum threshold of credit a user must have
      earned before being allowed to post comments and participate in forums.'),
  );
  
  return system_settings_form($form);
}

/**
  * Validate the BOINC community form.
  */
function boinccore_admin_community_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (!is_numeric($values['boinc_comment_min_credit'])) {
    form_set_error('boinc_comment_min_credit', t('The minimum credit required
      to post must be a number.'));
  }
}

/**
  * Handle post-validation submission of BOINC community form.
  */
function boinccore_admin_community_submit($form, &$form_state) {
  drupal_set_message(t('BOINC community settings have been updated.'));
}


