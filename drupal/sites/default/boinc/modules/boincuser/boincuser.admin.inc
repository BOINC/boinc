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
    'boinc_app_list_url' => variable_get('boinc_app_list_url', ''),
    'boinc_admin_mailing_list' => variable_get('boinc_admin_mailing_list', ''),
    'boinc_admin_mailing_list_subject_tag' => variable_get('boinc_admin_mailing_list_subject_tag', ''),
    'boinc_debug_mode' => variable_get('boinc_debug_mode', 0),
    'boinc_project_config_keywords' => variable_get('boinc_project_config_keywords', ''),
  );
  //drupal_set_message(print_r($default, true));
  //drupal_set_message(print_r($form_state, true));
  // Show overrides if the root dir is set and others have not been detected
  $show_overrides = FALSE;
  if ($default['boinc_root_dir']) {
    foreach ($default as $key => $dir) {
      switch ($key) {
      case 'boinc_config_xml_dir':
      case 'boinc_html_inc_dir':
      case 'boinc_project_config_dir':
      case 'boinc_user_image_dir':
        // If any of these values are not set, show the overrides section
        if (!$dir) {
          $default[$key] = $form_state['values'][$key];
          $show_overrides = TRUE;
          break;
        }
        break;
      default:
      }
      if ($show_overrides) {
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
  );
  $form['overrides']['boinc_html_inc_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC web includes directory'),
    '#default_value' => $default['boinc_html_inc_dir'],
    '#description' => t('The filesystem path to the html/inc/ directory that
      contains util.inc'),
  );
  $form['overrides']['boinc_project_config_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC project config directory'),
    '#default_value' => $default['boinc_project_config_dir'],
    '#description' => t('The filesystem path to the project/ directory that
      contains project.inc'),
  );
  $form['overrides']['boinc_user_image_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC user profile image directory'),
    '#default_value' => $default['boinc_user_image_dir'],
    '#description' => t('The filesystem directory that contains user images'),
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
  $form['boinc_app_list_url'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC applications page URL (optional)'),
    '#default_value' => $default['boinc_app_list_url'],
    '#description' => t('An absolute URL or path relative to the site base to the applications page (e.g. %full_url or just %relative_url).',
      array(
        '%full_url' => $base_url . '/apps.php',
        '%relative_url' => 'apps.php',
      )
    ),
  );
  $form['boinc_host_sched_logs_dir'] = array(
    '#type' => 'textfield',
    '#title' => t('BOINC host scheduler log directory (optional)'),
    '#default_value' => $default['boinc_host_sched_logs_dir'],
    '#description' => t('The filesystem directory that contains host scheduler logs'),
  );
  $form['boinc_admin_mailing_list'] = array(
    '#type' => 'textfield',
    '#title' => t('Admin mailing list'),
    '#default_value' => $default['boinc_admin_mailing_list'],
    '#description' => t('Enter an email-address which will be used to contact admisistrators. If empty, the site email configured in Settings -> Site Information will be used.'),
  );
  $form['boinc_admin_mailing_list_subject_tag'] = array(
    '#type' => 'textfield',
    '#title' => t('Admin mailing list - subject tag'),
    '#default_value' => $default['boinc_admin_mailing_list_subject_tag'],
    '#description' => t('Text which will appear at the beginning of the subject for any admin notification emails. If empty, no tag will be included. Examples: [drupal-error].'),
  );
  $form['boinc_debug_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show debug messages in system log'),
    '#default_value' => $default['boinc_debug_mode']
  );
  $form['boinc_project_config_keywords'] = array(
    '#type' => 'textarea',
    '#title' => t('Project/Job Keywords for get_project_config RPC'),
    '#default_value' => $default['boinc_project_config_keywords'],
    '#description' => t('XML from this text box will be appended to the XML output from the get_project_config RPC. There is no check for XML validity! See !link for more details. It is okay to leave this blank.',
    array(
      '!link' => l('BOINC wiki page', 'https://boinc.berkeley.edu/trac/wiki/JobKeywords')
    )),
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


/**
  * Drupal-BOINC Web site related options.
  */
function boincuser_admin_weboptions(&$form_state) {
  global $base_url;
  $form = array();

  //form defaults
  $default = array(
    'boinc_weboptions_enableaccountcreateRPC' => variable_get('boinc_weboptions_enableaccountcreateRPC', TRUE),
    'boinc_weboptions_registrationtitle' => variable_get('boinc_weboptions_registrationtitle', 'Please read and acknowledge our terms of use'),
    'boinc_weboptions_termsofuse' => variable_get('boinc_weboptions_termsofuse', ''),
    'boinc_weboptions_overrideboinctou' => variable_get('boinc_weboptions_overrideboinctou', FALSE),
    'boinc_weboptions_agreequestion' => variable_get('boinc_weboptions_agreequestion', 'Do you agree with the above terms of use?'),
    'boinc_weboptions_registrationtitle2' => variable_get('boinc_weboptions_registrationtitle2', 'Fill in your name, email, and choose a secure passphrase.'),
    'boinc_weboptions_existinguser_tou' => variable_get('boinc_weboptions_existinguser_tou', FALSE),
    'boinc_weboptions_pathstoignore' => variable_get('boinc_weboptions_pathstoignore', "moderation\ncontent/moderation\nprivacy"),
    'boinc_weboptions_accountfinish' => variable_get('boinc_weboptions_accountfinish', ''),
    'boinc_weboptions_moderationpage' => variable_get('boinc_weboptions_moderationpage', ''),
    'boinc_weboptions_rulespolicies' => variable_get('boinc_weboptions_rulespolicies', ''),
    'boinc_other_frontpage' => variable_get('boinc_other_frontpage', ''),
    'boinc_weboptions_blacklisted_usernames' => variable_get('boinc_weboptions_blacklisted_usernames', "admin\nadministrator\nmoderator"),
  );

  // Define the form
  $form['registrationtitle'] = array(
    '#value' => '<h3>BOINC Registration</h3>',
  );

  $form['boinc_weboptions_enableaccountcreateRPC'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable create_account.php RPC'),
    '#default_value' => $default['boinc_weboptions_enableaccountcreateRPC'],
    '#description' => t('If checked, users will be able to create an account remotely using the create_account.php RPC. This option is independent of the user regsitration option found in ') . l(t('User management -> User settings'), '/admin/user/settings') . '. If enabled, user <b>may not be able to see and agree to the terms-of-use</b> to your site\'s privacy and data retention policies!',
  );

  $form['boinc_weboptions_registrationtitle'] = array(
    '#type' => 'textfield',
    '#title' => t('Title of regsistration page'),
    '#description' => t('Title text presented above the terms of use text.'),
    '#default_value' => $default['boinc_weboptions_registrationtitle'],
  );

  $form['boinc_weboptions_termsofuse'] = array(
    '#type' => 'textarea',
    '#title' => t('Terms of Use Message for User Registration Page'),
    '#default_value' => $default['boinc_weboptions_termsofuse'],
    '#cols' => 60,
    '#rows' => 8,
    '#description' => t('Text to be displayed on site\'s user registration page. Privacy policy and other data retention information goes here. If empty, there will be no terms of use message, and the title above and checkbox below will not be shown.'),
  );

  $form['boinc_weboptions_overrideboinctou'] = array(
    '#type' => 'checkbox',
    '#title' => t('Override BOINC terms of use.'),
    '#default_value' => $default['boinc_weboptions_overrideboinctou'],
    '#description' => t('Override the BOINC terms_of_use.txt file for get_profject_config.php RPC. If TRUE, the get_project_config.php RPC will use the above terms of use text entered above. Otherwise, the original terms_of_use.txt text file in the project directory is used. And you will need to be responsible for keeping both the text file and the above terms of use text up-to-date and in sync with each other.'),
  );

  $form['boinc_weboptions_agreequestion'] = array(
    '#type' => 'textfield',
    '#title' => t('Terms of use agreement question'),
    '#description' => t('This text is presented to the user as the question next to the \'I agree\' checkbox.'),
    '#default_value' => $default['boinc_weboptions_agreequestion'],
  );

  $form['boinc_weboptions_registrationtitle2'] = array(
    '#type' => 'textfield',
    '#title' => t('Instructions for username/email/password questions'),
    '#description' => t('Title text presented below terms of use and above username/email/password textfields.'),
    '#default_value' => $default['boinc_weboptions_registrationtitle2'],
  );

  $form['boinc_weboptions_existinguser_tou'] = array(
    '#type' => 'checkbox',
    '#title' => t('Are existing users forced to agree to the Terms of Use?'),
    '#default_value' => $default['boinc_weboptions_existinguser_tou'],
    '#description' => t('If TRUE, existing users are forced to agree to a terms of use (if present) when the login and the system has detected they have not agreed. Otherwise they may login as normal. This option has no affect on whether or not new users must agree to the terms of use.'),
  );

  $form['boinc_weboptions_pathstoignore'] = array(
    '#type' => 'textarea',
    '#title' => t('Paths to ignore the Terms of Use page'),
    '#default_value' => $default['boinc_weboptions_pathstoignore'],
    '#cols' => 60,
    '#rows' => 8,
    '#description' => t('A list of Drupal URLs/paths to ignore for Terms of Use (ToU). If the option above \'Are existing users forced to agree to the Terms of Use?\' is activated, then some paths need to be ignored when checking if a user has agreed to the ToU. A good example is the logout path, "logout", or else users will not be able to logout!
<p>There is a default list of paths that must be ignored or else the site will not function. They are not included in this box. Here you may provide additional paths to be ignored, for example the privacy policy page may be accessible so that users may read it before agreeing to the site\'s ToU.
<p>Paths should be entered one per line. All paths should be <em>lower-case</em> and  should not include a leading \'/\'. Example: account/info/edit will allow the user to visit ' . $base_url . '/account/info/edit without first agreeing to the ToU. Regexp are allowed. Example: account/* will allow the user to visit any path starting with ' . $base_url . '/account/.'),
  );

  $form['pathtitle'] = array(
    '#value' => '<h3>Path Options</h3>',
  );

  $form['boinc_weboptions_accountfinish'] = array (
    '#type' => 'textfield',
    '#title' => t('Path to a custom account_finish.php page, should be a path to a node'),
    '#description' => t('Provide a path to a node which will serve as your site\'s landing page for users create an account using the BOINC client manager. They will be directed to this page after the account is created. If blank, a default account_finish page will be used.<br>Examples: account_finish, content/welcome, node/123'),
    '#default_value' => $default['boinc_weboptions_accountfinish'],
  );
  $form['boinc_weboptions_moderationpage'] = array (
    '#type' => 'textfield',
    '#title' => t('Path to the site\'s content moderation info page, should be a path to a node'),
    '#description' => t('Provide a path to a node which will serve as your site\'s page for account/content moderation information. This will be used on the default account_finish page. If blank, no link to a moderation page will be provided. If a path is provided to the custom account_finish.php page (above), this field will be ignored.<br>Examples: moderation, content/moderation, node/456'),
    '#default_value' => $default['boinc_weboptions_moderationpage'],
  );
  $form['boinc_weboptions_rulespolicies'] = array (
    '#type' => 'textfield',
    '#title' => t('Path to the site\'s rule and policies page, should be a path to a node'),
    '#description' => t('Provide a path to a node which will serve as your site\'s rules and policies page. This will be used on the Join page shown to new users. If blank, no link to a rules and policies page will be provided.<br>Examples: rules-and-policies, node/789'),
    '#default_value' => $default['boinc_weboptions_rulespolicies'],
  );
  $form['boinc_other_frontpage'] = array (
    '#type' => 'textarea',
    '#title' => bts('Message for site\'s Home Page', array(), NULL, 'boinc:admin-boinc-other-options'),
    '#default_value' => $default['boinc_other_frontpage'],
    '#cols' => 60,
    '#rows' => 8,
    '#description' => bts('Text to be displayed on the site\'s Home landing page.', array(), NULL, 'boinc:admin-boinc-other-options'),
  );

  $form['Othertitle'] = array(
    '#value' => '<h3>Other Options</h3>',
  );

  $form['boinc_weboptions_blacklisted_usernames'] = array (
    '#type' => 'textarea',
    '#title' => t('BOINC username blacklist'),
    '#default_value' => $default['boinc_weboptions_blacklisted_usernames'],
    '#cols' => 60,
    '#rows' => 7,
    '#description' => t('Username blacklist: List of names that users will not be able to choose as their BOINC username. This will only affect Web registration and when a user changes their name using the Web site. Names should be entered one per line.<p>All names should be <em>lower-case</em>. The comparison made is case-insensitive.<p>If you wish to <em>disable</em> this feature, remove all names from this textbox; the blacklist will be empty.'),
  );

  return system_settings_form($form);
}

/**
  * Validate BOINC other form
  */
function boincuser_admin_weboptions_validate($form, &$form_state) {
  $values = $form_state['values'];

  $accountfinish = $values['boinc_weboptions_accountfinish'];
  if ( ($accountfinish) AND (!drupal_lookup_path('source', $accountfinish)) ) {
    form_set_error('boinc weboptions_accountfinish', t('Path to custom account finish page not found. Please provide a valid path, or leave blank to unset.'));
  }

  $moderationpage = $values['boinc_weboptions_moderationpage'];
  if ( ($moderationpage) AND (!drupal_lookup_path('source', $moderationpage)) ) {
    form_set_error('boinc weboptions_moderationpage', t('Path to moderation page not found. Please provide a valid path, or leave blank to unset.'));
  }

  $rulespolicies = $values['boinc_weboptions_rulespolicies'];
  if ( ($rulespolicies) AND (!drupal_lookup_path('source', $rulespolicies)) ) {
    form_set_error('boinc weboptions_rulespolicies', t('Path to rules and policies page not found. Please provide a valid path, or leave blank to unset.'));
  }
}

/**
  * Submit BOINC other form
  */
function boincuser_admin_weboptions_submit($form, &$form_state) {
  drupal_set_message( bts("Status: Drupa-BOINC Web site options have been updated", array(), NULL, 'boinc:admin-boinc-website-options') );
}
