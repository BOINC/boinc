<?php
// $Id$

/**
* @file
* Administration page callbacks for the boinctranslate module.
*/

/**
  * The BOINC translation settings form allows configuration of BTS integration
  * (and other translation related settings)
  */
function boinctranslate_admin_settings(&$form_state) {
  $form = array();
  $default = array(
    'translation_file_urls' => variable_get('boinc_translation_service_files', ''),
  );
  
  // Define the form
  $form['boinc_translation_service_files'] = array(
    '#type' => 'textarea',
    '#title' => t('Translation file URLs'),
    '#default_value' => $default['translation_file_urls'],
    '#description' => t('List URLs to translation files (.po) that should be
      used for translating strings on this site (enter one URL per line).'),
  );
  
  $form['import_now'] = array(
    '#value' => '<p>' . l(t('Run the import now'), 'admin/boinc/translation/import') . '</p>',
  );
  
  return system_settings_form($form);
}

/**
  * Validate the BOINC translation settings form.
  */
function boinctranslate_admin_settings_validate($form, &$form_state) {
  $values = $form_state['values'];
  $errors = array();
  // Try to retrieve the files in the given URLs
  $urls = explode("\n", $values['boinc_translation_service_files']);
  foreach ($urls as $url) {
    $url = trim($url);
    $result = drupal_http_request($url, array(), 'GET', NULL, 1, 30);
    if (!$result OR $result->code == '404' OR !$result->data) {
      form_set_error('boinc_translation_service_files', 
        t('@url is not accessible', array('@url' => $url))
      );
    }
  }
}

/**
  * Handle post-validation submission of BOINC translation settings form.
  */
function boinctranslate_admin_settings_submit($form, &$form_state) {
  drupal_set_message(t('BOINC translation settings have been updated.'));
}

