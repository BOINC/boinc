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
    'transifex_user' => variable_get('boinc_translate_transifex_user', ''),
    'transifex_pass' => variable_get('boinc_translate_transifex_pass', ''),
    'transifex_resources' => variable_get(
      'boinc_translate_transifex_project_resources', ''
    ), 
  );
  
  // Define the form
  
  if ($default['transifex_user'] AND $default['transifex_pass']) {
    if (trim($default['transifex_resources'])) {
      $form['import_now'] = array(
        '#value' => '<div class="form-item">'
        . '<label>' . t('Import translations') . ':</label>'
        . '<p>'
        . l(t('Run the import now'), 'admin/boinc/translation/import')
        . '</p>'
        . '</div>',
      );
    }
  }
  
  $form['transifex'] = array(
    '#title' => t('Transifex settings'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#attributes' => array('class' => '')
  );
  $form['transifex']['boinc_translate_transifex_user'] = array(
    '#type' => 'textfield',
    '#title' => t('User name'),
    '#default_value' => $default['transifex_user'],
  );
  $form['transifex']['boinc_translate_transifex_pass'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#attributes' => array(
      'placeholder' => $default['transifex_pass'] ? '********' : '',
    ),
  );
  $form['transifex']['boinc_translate_transifex_project_resources'] = array(
    '#type' => 'textarea',
    '#title' => t('Project-specific resources'),
    '#default_value' => $default['transifex_resources'],
    '#description' => t('List Transifex project resources to be used for
      translating strings on this site (one per line in "project:resource"
      format). Resources will be imported in the order they are given, so
      translations from the resources at the top of the list will be overridden
      by any matching translations found in resources further down the list.'),
  ); 
  
  return system_settings_form($form);
}

/**
  * Validate the BOINC translation settings form.
  */
function boinctranslate_admin_settings_validate($form, &$form_state) {
  $values = $form_state['values'];
  $errors = array();
  $api_base_url = 'https://www.transifex.com/api/2';
  
  if (!$values['boinc_translate_transifex_user']) {
    form_set_error('boinc_translate_transifex_user', t('User name is required.'));
  }
  if (!$values['boinc_translate_transifex_pass']) {
    if (!variable_get('boinc_translate_transifex_pass', '')) {
      form_set_error('boinc_translate_transifex_pass', t('Password is required.'));
    }
    else {
      unset($form_state['values']['boinc_translate_transifex_pass']);
    }
  }
  
  $username = $values['boinc_translate_transifex_user'];
  $password = ($values['boinc_translate_transifex_pass']) ? $values['boinc_translate_transifex_pass'] : variable_get('boinc_translate_transifex_pass', '');
  
  if ($username AND $password) {
    // Test authentication
    $authenticated = FALSE;
    $ch = curl_init();
    $resource_url = "{$api_base_url}/project/boinc/resource/project-generic";
    curl_setopt($ch, CURLOPT_URL, "{$resource_url}/translation/en");
    curl_setopt($ch, CURLOPT_HTTPHEADER,
      array(
        "Authorization: Basic " . base64_encode($username . ":" . $password)
      )
    );
    curl_setopt($ch, CURLOPT_HEADER, 1);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    $result = curl_exec($ch);
    curl_close($ch);
    
    if ($result) {
      if (strstr($result, '401 UNAUTHORIZED')) {
        form_set_error(
          'boinc_translate_transifex_pass',
          t('Transifex authentication failed.')
        );
      }
      else {
        $authenticated = TRUE;
      }
    }
    
    if ($authenticated) {
      // Try to access the given resources
      $project_resources = explode("\n", $values['boinc_translate_transifex_project_resources']);
      foreach ($project_resources as $line => $project_resource) {
        $project_resource = trim($project_resource);
        if (!$project_resource OR $project_resource[0] == '#') {
          // Ignore empty lines and comments
          continue;
        }
        // Check that resources can be parsed
        list($project, $resource) = array_pad(explode(':', $project_resource), 2, '');
        if (!$project OR !$resource) {
          form_set_error(
            'boinc_translate_transifex_project_resources',
            t('Project resource could not be parsed at line @line',
              array('@line' => $line+1)
            )
          );
          break;
        }
        
        $ch = curl_init();
        $resource_url = "{$api_base_url}/project/{$project}/resource/{$resource}";
        curl_setopt($ch, CURLOPT_URL, "{$resource_url}/translation/en");
        curl_setopt($ch, CURLOPT_HTTPHEADER,
          array(
            "Authorization: Basic " . base64_encode($username . ":" . $password)
          )
        );
        curl_setopt($ch, CURLOPT_HEADER, 1);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_TIMEOUT, 10);
        $result = curl_exec($ch);
        
        if (strstr($result, '404 NOT FOUND')) {
          form_set_error(
            'boinc_translate_transifex_project_resources',
            t('Project resource %name not found.',
              array('%name' => "{$project}:{$resource}")
            )
          );
        }
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

