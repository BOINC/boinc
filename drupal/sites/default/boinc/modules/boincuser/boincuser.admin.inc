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
*//*
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
}*/

/**
  * The BOINC environment form allows paths to be set to connect Drupal to an
  * existing BOINC web interface.
  */
function boincuser_admin_environment(&$form_state) {
  global $base_url;
  $form = array();
  $default = array(
    'boinc_root_dir' => variable_get('boinc_root_dir', ''),
    'boinc_config_xml_dir' => variable_get('boinc_config_xml_dir', ''),
    'boinc_html_inc_dir' => variable_get('boinc_html_inc_dir', ''),
    'boinc_project_config_dir' => variable_get('boinc_project_config_dir', ''),
    'boinc_user_image_dir' => variable_get('boinc_user_image_dir', ''),
    'boinc_host_sched_logs_dir' => variable_get('boinc_host_sched_logs_dir', ''),
    'boinc_server_status_url' => variable_get('boinc_server_status_url', ''),
  );
  //drupal_set_message(print_r($default, true));
  //drupal_set_message(print_r($form_state, true));
  // Show overrides if the root dir is set and others have not been detected
  $show_overrides = FALSE;
  if ($default['boinc_root_dir']) {
    foreach ($default as $key => $dir) {
      if ($key == 'boinc_root_dir') continue;
      if (!$dir) {
        $default[$key] = $form_state['values'][$key];
        $show_overrides = TRUE;
        break;
      }
    }
  }
  // Define the form
  $form['boinc_root_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC project root directory'),
    '#default_value' => $default['boinc_root_dir'], 
    '#description' => t('The filesystem path to the BOINC project root. This
      directory should contain config.xml as well as the html subdirectory,
      which itself contains inc/, project/, and user_profile/.'),
  );
  $form['overrides'] = array(
    '#title' => t('Overrides'),
    '#type' => 'fieldset',
    '#description' => '',
    '#collapsible' => TRUE,
    '#collapsed' => !$show_overrides,
    '#attributes' => array('class' => 'advanced-settings')
  );
  $form['overrides']['boinc_config_xml_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC config.xml directory'),
    '#default_value' => $default['boinc_config_xml_dir'],
    '#description' => t('The filesystem path that contains config.xml'),
    '#disabled' => is_readable("{$default['boinc_config_xml_dir']}/config.xml")
  );
  $form['overrides']['boinc_html_inc_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC web includes directory'),
    '#default_value' => $default['boinc_html_inc_dir'],
    '#description' => t('The filesystem path to the html/inc/ directory that
      contains util.inc'),
    '#disabled' => is_readable("{$default['boinc_html_inc_dir']}/util.inc")
  );
  $form['overrides']['boinc_project_config_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC project config directory'),
    '#default_value' => $default['boinc_project_config_dir'],
    '#description' => t('The filesystem path to the project/ directory that
      contains project.inc'),
    '#disabled' => is_readable("{$default['boinc_project_config_dir']}/project.inc")
  );
  $form['overrides']['boinc_user_image_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC user profile image directory'),
    '#default_value' => $default['boinc_user_image_dir'],
    '#description' => t('The filesystem directory that contains user images'),
    '#disabled' => is_dir($default['boinc_user_image_dir'])
  );
  $form['boinc_server_status_url'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC server status page URL (optional)'),
    '#default_value' => $default['boinc_server_status_url'],
    '#description' => t('An absolute URL or path relative to the site base to the server status page (e.g. %full_url or just %relative_url).',
      array(
        '%full_url' => $base_url . '/server_status.php',
        '%relative_url' => 'server_status.php',
      )
    ),
  );
  $form['boinc_host_sched_logs_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC host scheduler log directory (optional)'),
    '#default_value' => $default['boinc_host_sched_logs_dir'],
    '#description' => t('The filesystem directory that contains host scheduler logs'),
  );
  return system_settings_form($form);
}

/**
  * Validate the BOINC environment form.
  */
function boincuser_admin_environment_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (!$values['boinc_root_dir']) {
    form_set_error('boinc_root_dir', t('BOINC project root directory must be
      defined!'));
  }
  elseif (!is_dir($values['boinc_root_dir'])) {
    form_set_error('boinc_root_dir', t('BOINC project root not found at %directory',
      array('%directory' => $values['boinc_root_dir'])
    ));
  }
  else {
    // By default, try to automatically set specific directories from the root
    if (!$values['boinc_config_xml_dir']) {
      $values['boinc_config_xml_dir'] = $values['boinc_root_dir'];
      form_set_value($form['overrides']['boinc_config_xml_dir'], $values['boinc_config_xml_dir'], $form_state);
    }
    if (!$values['boinc_html_inc_dir']) {
      $values['boinc_html_inc_dir'] = "{$values['boinc_root_dir']}/html/inc";
      form_set_value($form['overrides']['boinc_html_inc_dir'], $values['boinc_html_inc_dir'], $form_state);
    }
    if (!$values['boinc_project_config_dir']) {
      $values['boinc_project_config_dir'] = "{$values['boinc_root_dir']}/html/project";
      form_set_value($form['overrides']['boinc_project_config_dir'], $values['boinc_project_config_dir'], $form_state);
    }
    if (!$values['boinc_user_image_dir']) {
      $values['boinc_user_image_dir'] = "{$values['boinc_root_dir']}/html/user_profile/images";
      form_set_value($form['overrides']['boinc_user_image_dir'], $values['boinc_user_image_dir'], $form_state);
    }
    // Validate that required BOINC content is found
    if (!is_readable("{$values['boinc_config_xml_dir']}/config.xml")) {
      form_set_error('boinc_config_xml_dir', t('@file not found in %directory',
        array(
          '@file' => 'config.xml',
          '%directory' => "{$values['boinc_config_xml_dir']}/"
        )
      ));
    }
    if (!is_readable("{$values['boinc_html_inc_dir']}/util.inc")) {
      form_set_error('boinc_html_inc_dir', t('@file not found in %directory',
        array(
          '@file' => 'util.inc',
          '%directory' => "{$values['boinc_html_inc_dir']}/"
        )
      ));
    }
    if (!is_readable("{$values['boinc_project_config_dir']}/project.inc")) {
      form_set_error('boinc_project_config_dir', t('@file not found in %directory',
        array(
          '@file' => 'project.inc',
          '%directory' => "{$values['boinc_project_config_dir']}/"
        )
      ));
    }
    if (!is_dir($values['boinc_user_image_dir'])) {
      form_set_error('boinc_user_image_dir', t('Profile image directory not found
        at %directory', array('%directory' => $values['boinc_user_image_dir'])
      ));
    }
    if ($values['boinc_host_sched_logs_dir']
        AND !is_dir($values['boinc_host_sched_logs_dir'])
      ) {
      form_set_error('boinc_host_sched_logs_dir', t('Host scheduler log
        directory not found at %directory',
        array('%directory' => $values['boinc_host_sched_logs_dir'])
      ));
    }
  }
}

/**
  * Handle post-validation submission of BOINC environment form.
  */
function boincuser_admin_environment_submit($form, &$form_state) {
  unset($_SESSION['boinc_env']);
  drupal_set_message(t('BOINC environment settings have been updated.'));
}

/**
  * The BOINC scheduler form provides options for how scheduler tags are
  * inserted into the front page
  */
function boincuser_admin_scheduler(&$form_state) {
  $form = array();
  $config_cgi_url = '';
  if ($config_xml_dir = variable_get('boinc_config_xml_dir', '')) {
    // Get the cgi_url option from the config file as the default scheduler URL
    $xml = new DOMDocument();
    if ($xml->load("{$config_xml_dir}/config.xml")) {
      $config = xml_to_array($xml);
      if (isset($config['boinc']['config']['cgi_url']['@value'])) {
        $config_cgi_url = rtrim(trim($config['boinc']['config']['cgi_url']['@value']), '/');
        $config_cgi_url .= '/cgi';
      }
    }
  }
  $default = array(
    'boinc_scheduler_urls' => variable_get('boinc_scheduler_urls', $config_cgi_url),
    'boinc_scheduler_tag_format_old' => variable_get('boinc_scheduler_tag_format_old', 1),
    'boinc_scheduler_tag_format_new' => variable_get('boinc_scheduler_tag_format_new', 1)
  );
  
  // Define the form
  $form['boinc_scheduler_urls'] = array(
    '#type' => 'textarea',
    '#title' => t('Scheduling server URLs'),
    '#default_value' => $default['boinc_scheduler_urls'],
    '#cols' => 60,
    '#rows' => 5,
    '#description' => t('If there are multiple URLs, please enter one per line.'),
  );
  $form['boinc_scheduler_tag_format_old'] = array(
    '#type' => 'checkbox',
    '#title' => t('Old format (@scheduler tag)', array('@scheduler' => '<scheduler>')),
    '#default_value' => $default['boinc_scheduler_tag_format_old']
  );
  $form['boinc_scheduler_tag_format_new'] = array(
    '#type' => 'checkbox',
    '#title' => t('New format (@link tag)', array('@link' => '<link>')),
    '#default_value' => $default['boinc_scheduler_tag_format_new']
  );
  
  return system_settings_form($form);
}

/**
  * Validate the BOINC scheduler form.
  */
function boincuser_admin_scheduler_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (!$values['boinc_scheduler_urls']) {
    form_set_error('boinc_scheduler_urls', t('At least one scheduling server
      URL must be specified.'));
  }
  else {
    $urls = explode("\r\n", $values['boinc_scheduler_urls']);
    foreach ($urls as $key => $url) {
      if (!$url) {
        unset($urls[$key]);
        continue;
      }
      if (!filter_var($url, FILTER_VALIDATE_URL)) {
        form_set_error('boinc_scheduler_urls', t('@url is not a valid URL
          format.', array('@url' => $url)));
        break;
      }
    }
    $form_state['values']['boinc_scheduler_urls'] = implode("\r\n", $urls);
  }
}

/**
  * Handle post-validation submission of BOINC scheduler form.
  */
function boincuser_admin_scheduler_submit($form, &$form_state) {
  drupal_set_message(t('BOINC scheduling server settings have been updated.'));
}


