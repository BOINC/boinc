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
  $initialized = FALSE;
  $import_enabled = FALSE;
  $default = array(
    'transifex_user' => variable_get('boinc_translate_transifex_user', ''),
    'transifex_pass' => variable_get('boinc_translate_transifex_pass', ''),
    'transifex_boinc_name' => variable_get(
      'boinc_translate_transifex_standard_name', 'boinc'
    ),
    'transifex_boinc_resources' => variable_get(
      'boinc_translate_transifex_standard_resources',
      "project-generic\nweb"
    ),
    'transifex_project_name' => variable_get(
      'boinc_translate_transifex_project_name', ''
    ),
    'transifex_project_resources' => variable_get(
      'boinc_translate_transifex_project_resources', ''
    ), 
  );
  
  if ($default['transifex_user'] AND $default['transifex_pass']) {
    if (trim($default['transifex_boinc_resources'])
    OR trim($default['transifex_project_resources'])) {
      $import_enabled = TRUE;
    }
    if ($default['transifex_boinc_name']
    AND $default['transifex_project_name']
    AND trim($default['transifex_boinc_resources'])
    AND trim($default['transifex_project_resources'])) {
      $initialized = TRUE;
    }
  }
  
  // Define the form
  
  $form['transifex'] = array(
    '#title' => t('Transifex settings'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => $initialized,
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
  $form['transifex']['boinc_translate_transifex_standard_name'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC Transifex project'),
    '#default_value' => $default['transifex_boinc_name'],
    '#description' => t('The portion of the Transifex URL that identifies BOINC.'),
  );
  $form['transifex']['boinc_translate_transifex_standard_resources'] = array(
    '#type' => 'textarea',
    '#title' => t('BOINC resources'),
    '#default_value' => $default['transifex_boinc_resources'],
    '#description' => t('List BOINC Transifex resources to be used for
      translating strings on this site (one resource string per line).
      Resources will be imported in the order they are given, so
      translations from the resources at the top of the list will be overridden
      by any matching translations found in resources further down the list.'),
  );
  $form['transifex']['boinc_translate_transifex_project_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Project-specific Transifex project'),
    '#default_value' => $default['transifex_project_name'],
    '#description' => t('The portion of the Transifex URL that identifies this project.'),
  );
  $form['transifex']['boinc_translate_transifex_project_resources'] = array(
    '#type' => 'textarea',
    '#title' => t('Project-specific resources'),
    '#default_value' => $default['transifex_project_resources'],
    '#description' => t('List project-specific Transifex resources to be used
      for translating strings on this site (one resource string per line).
      Resources will be imported in the order they are given, so
      translations from the resources at the top of the list will be overridden
      by any matching translations found in resources further down the list.'),
  );
  $form['transifex']['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  
  $form['tools'] = array(
    '#title' => t('Tools'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#attributes' => array('class' => '')
  );
  $form['tools']['import_now'] = array(
    '#type' => 'button',
    '#value' => t('Import translations'),
    '#executes_submit_callback' => TRUE,
    '#submit' => array('boinctranslate_admin_settings_import_now'),
    '#disabled' => !$import_enabled,
  );
  $form['tools']['initialize_languages'] = array(
    '#type' => 'button',
    '#value' => t('Install official BOINC languages'),
    '#executes_submit_callback' => TRUE,
    '#submit' => array('boinctranslate_admin_settings_initialize_languages'),
    '#disabled' => !$initialized,
  );
  
  $form['#submit'][] = 'system_settings_form_submit';
  $form['#theme'] = 'system_settings_form'; 
  
  return $form;
}

function boinctranslate_admin_settings_import_now() {
  drupal_goto('admin/boinc/translation/import');
}

function boinctranslate_admin_settings_initialize_languages() {
  drupal_goto('admin/boinc/translation/initialize-languages');
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
  if (!$values['boinc_translate_transifex_standard_name']) {
    form_set_error('boinc_translate_transifex_standard_name', 
      t('BOINC Transifex project name is required.')
    );
  }
  if (!$values['boinc_translate_transifex_standard_resources']) {
    form_set_error('boinc_translate_transifex_standard_resources',
      t('At least one BOINC Transifex project resource is required.')
    );
  }
  
  $username = $values['boinc_translate_transifex_user'];
  $password = ($values['boinc_translate_transifex_pass']) ? $values['boinc_translate_transifex_pass'] : variable_get('boinc_translate_transifex_pass', '');
  $boinc_name = $values['boinc_translate_transifex_standard_name'];
  $boinc_resources = boinctranslate_parse_resources(
    $values['boinc_translate_transifex_standard_resources']
  );
  
  if ($username AND $password AND $boinc_name AND $boinc_resources) {
    // Test authentication
    $authenticated = FALSE;
    $ch = curl_init();
    $resource_url = "{$api_base_url}/project/{$boinc_name}";
    $resource_url .= "/resource/{$boinc_resources[0]}";
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
      // Prepare list of resources to validate
      $transifex_resources = array(
        'boinc' => $boinc_resources,
      );
      // Parse project-specific resources
      $project_name = trim($values['boinc_translate_transifex_project_name']);
      $project_resources = boinctranslate_parse_resources(
        $values['boinc_translate_transifex_project_resources']
      );
      if ($project_name AND $project_resources) {
        $transifex_resources[$project_name] = $project_resources;
      }
      elseif ($project_name AND !$project_resources) {
        drupal_set_message(
          t('No project-specific resources were provided'),
          'warning'
        );
      }
      elseif ($project_resources AND !$project_name) {
        drupal_set_message(
          t('No project-specific Transifex project name was provided'),
          'warning'
        );
      }
      
      // Try to access the given resources
      foreach ($transifex_resources as $project => $resources) {
        foreach ($resources as $resource) {
          
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
          list($response_header, $response_body) = explode("\r\n\r\n", $result, 2);
          
          if (strstr($response_header, '404 NOT FOUND')) {
            form_set_error(
              'boinc_translate_transifex_' . ($project == $boinc_name ? 'standard' : 'project') . '_resources',
              t('Resource %name not found in %project.',
                array(
                  '%name' => $resource,
                  '%project' => $project,
                )
              )
            );
          }
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


