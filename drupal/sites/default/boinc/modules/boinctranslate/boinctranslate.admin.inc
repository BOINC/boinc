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
    'translation_file_urls_generic' => variable_get('boinc_translation_service_files', ''),
    'translation_file_urls_project' => variable_get('boinc_translation_service_files_project', ''),
  );
  
  // Define the form
  
  if (trim($default['translation_file_urls_generic'])) {
    $form['import_now'] = array(
      '#value' => '<div class="form-item">'
      . '<label>' . t('Import translations') . ':</label>'
      . '<p>'
      . l(t('Run the import now'), 'admin/boinc/translation/import')
      . '</p>'
      . '</div>',
    );
  }
  
  $form['boinc_translation_service_files'] = array(
    '#type' => 'textarea',
    '#title' => t('Generic translation file URLs'),
    '#default_value' => $default['translation_file_urls_generic'],
    '#description' => t('List URLs to translation files (.po) that should be
      used for translating strings on this site (enter one URL per line). Files
      will be imported in the order they are given, so translations from the
      files at the top of the list will be overridden by any matching
      translations that appear in files further down the list.'),
  ); 
  
  $form['boinc_translation_service_files_project'] = array(
    '#type' => 'textarea',
    '#title' => t('Project-specific translation file URLs'),
    '#default_value' => $default['translation_file_urls_project'],
    '#description' => t('As above, list URLs to project-specific translation
      files (.po) to use for translation of strings that were added by
      customizations to this project (e.g. project-specific preferences).'),
  );
  
  return system_settings_form($form);
}

/**
  * Validate the BOINC translation settings form.
  */
function boinctranslate_admin_settings_validate($form, &$form_state) {
  $values = $form_state['values'];
  $errors = array();
  $url_groups = array(
    'boinc_translation_service_files',
    'boinc_translation_service_files_project',
  );
  foreach ($url_groups as $url_group) {
    // Try to retrieve the files in the given URLs
    $urls = explode("\n", $values[$url_group]);
    foreach ($urls as $url) {
      $url = trim($url);
      if (!$url OR $url[0] == '#') {
        // Ignore empty lines and comments
        continue;
      }
      $result = drupal_http_request($url, array(), 'GET', NULL, 1, 30);
      if (!$result OR $result->code == '404' OR !$result->data) {
        form_set_error(
          $url_group, 
          t('@url is not accessible', array('@url' => $url))
        );
      }
    }
  }
}

/**
  * Handle post-validation submission of BOINC translation settings form.
  */
function boinctranslate_admin_settings_submit($form, &$form_state) {
  drupal_set_message(t('BOINC translation settings have been updated.'));
}

