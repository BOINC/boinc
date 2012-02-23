<?php
// $Id$

/**
* @file
* Administration page callbacks for the boincuser module.
*/

/**
* Form builder. Configure annotations.
*
* @ingroup forms
* @see system_settings_form().
*/
function boincuser_admin_settings() {
  // Get an array of node types with internal names as keys and
  // "friendly names" as values. E.g.,
  // array('page' => 'Page', 'story' => 'Story')
  $options = node_get_types('names');
  $form['boincuser_integration'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Integrate BOINC user data with Drupal user data'),
    '#options' => array('boincuser_integration' => 'Enabled'),
    '#default_value' => variable_get('boincuser_integration', array()),
    '#description' => t('A placeholder for BOINC user settings.'),
  );
  return system_settings_form($form);
}


