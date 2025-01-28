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
    'transifex_boinc_drupal_resource' => variable_get(
      'boinc_translate_transifex_boinc_drupal_resource',
      'drupal'
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
  $form['transifex']['boinc_translate_transifex_boinc_drupal_resource'] = array(
    '#type' => 'textfield',
    '#title' => t('Official BOINC Drupal Resource'),
    '#default_value' => $default['transifex_boinc_drupal_resource'],
    '#description' => t('The portion of the Transifex URL that identifies
      the standard BOINC Resource to use for translation of the public
      content defined in the Drupal system.'),
  );
  $form['transifex']['boinc_translate_transifex_standard_resources'] = array(
    '#type' => 'textarea',
    '#title' => t('Additional BOINC Resources'),
    '#default_value' => $default['transifex_boinc_resources'],
    '#description' => t('List additional BOINC Transifex Resources to be used
      for translating strings on this site (one Resource string per line).
      Resources will be imported in the order they are given, so
      translations from the Resources at the top of the list will be overridden
      by any matching translations found in Resources further down the list.
      Note that strings from the Official BOINC Drupal Resource will override
      any strings from these additional Resources.'),
  );
  $form['transifex']['boinc_translate_transifex_project_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Project-specific Transifex project'),
    '#default_value' => $default['transifex_project_name'],
    '#description' => t('The portion of the Transifex URL that identifies this project.'),
  );
  $form['transifex']['boinc_translate_transifex_project_resources'] = array(
    '#type' => 'textarea',
    '#title' => t('Project-specific Resources'),
    '#default_value' => $default['transifex_project_resources'],
    '#description' => t('List project-specific Transifex Resources to be used
      for translating strings on this site (one Resource string per line).
      Resources will be imported in the order they are given, so
      translations from the Resources at the top of the list will be overridden
      by any matching translations found in Resources further down the list.
      <br/>
      NOTE: The first Resource listed here is considered the primary Resource
      for project-specific translations and will be updated by the "Update
      project-specific Resources" button below!'),
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
  $form['tools']['initialize_languages_text'] = array(
    '#type' => 'item',
    '#title' => t('Install official BOINC languages'),
    '#value' => t('Installs all languages from the BOINC Transifex project. Also installs missing languages in Drupal. <br>Languages must be enabled manually in Administer > Site configuration > Languages.'),
  );
  $form['tools']['initialize_languages'] = array(
    '#type' => 'button',
    '#value' => t('Install official BOINC languages'),
    '#executes_submit_callback' => TRUE,
    '#submit' => array('boinctranslate_admin_settings_initialize_languages'),
    '#disabled' => !$initialized,
  );
  $form['tools']['import_now_text'] = array(
    '#type' => 'item',
    '#title' => t('Import all translations'),
    '#value' => t('Imports from Tranifex all configured Resources into their respective text groups.'),
  );
  $form['tools']['import_now'] = array(
    '#type' => 'button',
    '#value' => t('Import all translations'),
    '#executes_submit_callback' => TRUE,
    '#submit' => array('boinctranslate_admin_settings_import_now'),
    '#disabled' => !$import_enabled,
  );
  $form['tools']['download_boinc_pot_text'] = array(
    '#type' => 'item',
    '#title' => t('Download official BOINC-Drupal .pot'),
    '#value' => t('Downloads the generic BOINC-Drupal .pot file. This is the template file for the boinc text group.'),
  );
  $form['tools']['download_boinc_pot'] = array(
    '#type' => 'button',
    '#value' => t('Download official BOINC-Drupal .pot'),
    '#executes_submit_callback' => TRUE,
    '#submit' => array('boinctranslate_admin_settings_download_boinc_pot'),
    '#disabled' => !$import_enabled,
  );
  $form['tools']['download_project_pot_text'] = array(
    '#type' => 'item',
    '#title' => t('Download project-specific .pot'),
    '#value' => t('Downloads the project-specific .pot file. This is the template file for the project text group.'),
  );
  $form['tools']['download_project_pot'] = array(
    '#type' => 'button',
    '#value' => t('Download project-specific .pot'),
    '#executes_submit_callback' => TRUE,
    '#submit' => array('boinctranslate_admin_settings_download_project_pot'),
    '#disabled' => !$import_enabled,
  );
  if (user_access('update official BOINC translations')) {
    $form['tools']['update_official_boinc_text'] = array(
      '#type' => 'item',
      '#title' => t('Update official BOINC translations'),
      '#value' => t('Updates the official BOINC-Drupal Resource (translatable strings templates) to the BOINC Transifex project, based on the consolidated imported translations.<br><b>WARNING: Do not use unless you have write-access to the BOINC Transifex project.</b>'),
    );
    $form['tools']['update_official_boinc'] = array(
      '#type' => 'button',
      '#value' => t('Update official BOINC translations'),
      '#executes_submit_callback' => TRUE,
      '#submit' => array('boinctranslate_admin_settings_update_official_boinc'),
      '#disabled' => !$import_enabled,
    );
  }
  $form['tools']['export_now_text'] = array(
    '#type' => 'item',
    '#title' => t('Update project-specific Resources'),
    '#value' => t('Updates the configured Resources (translatable strings templates) to the project\'s Transifex project.<br><b>WARNING: Do not use unless you have write-access to your project\'s Transifex project.</b>'),
  );
  $form['tools']['export_now'] = array(
    '#type' => 'button',
    '#value' => t('Update project-specific Resources'),
    '#executes_submit_callback' => TRUE,
    '#submit' => array('boinctranslate_admin_settings_export_now'),
    '#disabled' => !$import_enabled,
  );

  $form['#submit'][] = 'system_settings_form_submit';
  $form['#theme'] = 'system_settings_form';

  return $form;
}

function boinctranslate_admin_settings_export_now() {
  drupal_goto('admin/boinc/translation/export');
}

function boinctranslate_admin_settings_import_now() {
  drupal_goto('admin/boinc/translation/import');
}

function boinctranslate_admin_settings_initialize_languages() {
  drupal_goto('admin/boinc/translation/initialize-languages');
}

function boinctranslate_admin_settings_update_official_boinc() {
  drupal_goto('admin/boinc/translation/update-official-boinc');
}

function boinctranslate_admin_settings_download_boinc_pot() {
  drupal_goto('admin/boinc/translation/download-pot/boinc');
}

function boinctranslate_admin_settings_download_project_pot() {
  drupal_goto('admin/boinc/translation/download-pot/project');
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
      t('At least one BOINC Transifex project Resource is required.')
    );
  }
  if (!$values['boinc_translate_transifex_project_resources']) {
    form_set_error('boinc_translate_transifex_project_resources',
      t('At least one project-specific Transifex Resource is required.')
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
    $path = "project/{$boinc_name}/resource/{$boinc_resources[0]}/translation/en";
    $response = boinctranslate_transifex_request($path, NULL, TRUE, FALSE, $username, $password);

    if ($response) {
      if ($response == '401 UNAUTHORIZED') {
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

          $path = "project/{$project}/resource/{$resource}/translation/en";
          $response = boinctranslate_transifex_request($path);

          if ($response == '404 NOT FOUND') {
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


