<?php
// $Id$

/**
 * Form submission handlers and data processing functions are contained
 * herein to prevent littering of the main module file.
 */

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * New team form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * The definition of the create team form
 */
function boincteam_create_form(&$form_state) {
  $form = array();

  $default = array(
    'name' => '',
    'name_html' => '',
    'website' => '',
    'type' => 0,
    'country' => 0,
    'joinable' => TRUE,
    'description' => '',
  );

  // Standard option sets
  $form['boolean_options'] = array(
    '#type' => 'value',
    '#value' => array(1 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), 0 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes')),
  );

  // Vocabulary based option sets
  $form['type_options'] = array(
    '#type' => 'value',
    '#value' => array(0 => bts('Choose type', array(), NULL, 'boinc:form-choose'))
  );
  $vocabs = taxonomy_get_vocabularies(NULL);
  foreach ($vocabs as $vocab) {
    switch ($vocab->name) {
    case 'Teams':
      $team_types = taxonomy_get_tree($vocab->vid);
      if (module_exists('internationalization')) {
        $team_types = i18ntaxonomy_localize_terms($team_types);
      }
      foreach ($team_types as $team_type) {
        $form['type_options']['#value'][$team_type->tid] = $team_type->name;
      }
      break;
    default:
    }
  }
  $form['country_options'] = array(
    '#type' => 'value',
    '#value' => boinccore_get_country_list(),
  );

  // Form elements
  $form['name'] = array(
    '#title' => bts('Team name', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['name'],
    '#size' => 34,
    '#description' => bts('Text only, no HTML tags', array(), NULL, 'boinc:team-form-help'),
  );
  $form['name_html'] = array(
    '#title' => bts('Team name -- HTML version (optional)', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['name_html'],
    '#size' => 34,
    '#description' => bts('You may use limited HTML tags', array(), NULL, 'boinc:team-form-help'),
  );
  $form['website'] = array(
    '#title' => bts('Team website (optional)', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['website'],
    '#size' => 34,
    '#description' => bts("Displayed on the team's page", array(), NULL, 'boinc:team-create/edit'),
  );
  $form['type'] = array(
    '#title' => bts('Type of team', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'select',
    '#options' => $form['type_options']['#value'],
    '#default_value' => $default['type'],
  );
  $form['country'] = array(
    '#title' => bts('Country', array(), NULL, 'boinc:country-of-origin'),
    '#type' => 'select',
    '#options' => $form['country_options']['#value'],
    '#default_value' => $default['country'],
  );
  $form['joinable'] = array(
    '#title' => bts('Accept new members?', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['joinable'],
    '#size' => 34,
  );
  $form['description'] = array(
    '#title' => bts('Description of team', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'textarea',
    '#default_value' => $default['description'],
    '#size' => 5,
    '#description' => NULL,
  );
  $form['format'] = filter_form();

  // Form control
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">'
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => bts('Save team', array(), NULL, 'boinc:team-form-save'),
    '#suffix' => '</li>',
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), $_GET['q']) . '</li>'
  );

  return $form;
}

/**
 * The create team validation handler
 */
function boincteam_create_form_validate($form, &$form_state) {

  $values = $form_state['values'];
  $name = $values['name'];

  if (!$name) {
    form_set_error('name', bts('Team name is required.', array(), NULL, 'boinc:team-create/edit'));
  }
  else {
    require_boinc('boinc_db');
    if (BoincTeam::lookup_name($name)) {
      form_set_error('name', bts('A team named "@name" already exists.',
        array('@name' => $name), NULL, 'boinc:team-create/edit'));
    }
  }

  if (!$values['type'] OR !isset($values['type_options'][$values['type']])) {
    form_set_error('type', bts('Please select a team type.', array(), NULL, 'boinc:team-create/edit'));
  }
}

/**
 * The create team submit handler
 */
function boincteam_create_form_submit($form, &$form_state) {
  global $user;
  $account = user_load($user->uid);
  $values = $form_state['values'];
  //drupal_set_message('<pre>' . print_r($values,true) . '</pre>');

  $input_format = !empty($values['format']) ? $values['format'] : 4;
  $values['description'] = check_markup($values['description'], $input_format);

  // Create the team in the BOINC db

  require_boinc(array('user','team'));
  $boinc_team = make_team(
    $account->boincuser_id,
    $values['name'],
    $values['website'],
    $values['type'],
    $values['name_html'],
    $values['description'],
    $values['country']
  );

  if ($boinc_team) {
    $boinc_user = BoincUser::lookup_id($account->boincuser_id);
    user_join_team($boinc_team, $boinc_user);
  }
  else {
    drupal_set_message(t('Teams cannot be created at this time. The @project administrators have been notified.', array('@project' => PROJECT)));
    rules_invoke_event('boincteam_create_team_error', $values['name'], variable_get('boinc_admin_mailing_list_subject_tag', ''));
    watchdog('BOINC team', 'BOINC teams cannot be created for an unknown
      reason.', 'error');
    return FALSE;
  }

  // Create the team node in Drupal

  $teaser = node_teaser($values['description']);
  $created_time = time();
  $input_format = variable_get('filter_default_format', 1);

  $node = array(
    'type' => 'team',
    'title' => $values['name'],
    'body' => $values['description'],
    'teaser' => $teaser,
    'uid' => $user->uid,
    'path' => null,
    'status' => 1,  // published or not - always publish
    'promote' => 0,
    'created' => $created_time,
    'comment' => 0,  // comments disabled
    'moderate' => 0,
    'sticky' => 0,
    'format' => $input_format
  );

  // Use pathauto function, if available, to clean up the path
  if (module_exists('pathauto')) {
    module_load_include('inc', 'pathauto', 'pathauto');
    $node['path'] = pathauto_cleanstring($values['name']);
  }
  else {
    drupal_set_message(t('Teams cannot be created at this time. The @project administrators have been notified.', array('@project' => PROJECT)));
    rules_invoke_event('boincteam_create_team_nopathauto_error', $values['name'], variable_get('boinc_admin_mailing_list_subject_tag', ''));
    watchdog('BOINC team', 'BOINC teams require the Pathauto module. Teams
      cannot be created.', 'error');
  }

  /*
  // Add special organic group properties
  $node['og_description'] = strip_tags($boincteam->description);
  $node['og_selective'] = OG_OPEN;
  $node['og_register'] = OG_REGISTRATION_NEVER;
  $node['og_directory'] = OG_DIRECTORY_CHOOSE_FALSE;
  $node['og_private'] = 0;
  */

  $node = (object) $node; // node_save requires an object form

  /*
  $node->field_description[]['value'] = $boincteam->description;
  $node->field_url[]['value'] = $boincteam->url;
  $node->field_country[]['value'] = $boincteam->country;
  */

  $node->taxonomy[] = taxonomy_get_term($values['type']);

  // Save the team node
  node_save($node);

  // Save the team IDs to a BOINC <--> Drupal reference table.
  db_query('INSERT INTO {boincteam} (team_id, nid) VALUES (%d, %d)', $boinc_team->id, $node->nid);

  drupal_set_message(t('Team "@name" has been created.',
    array('@name' => $values['name'])));

  $form_state['redirect'] = "community/teams/{$node->nid}";
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Edit team form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * The definition of the edit team form
 */
function boincteam_edit_form(&$form_state, $team_id) {
  $form = array();

  $team = node_load($team_id);
  $boincteam = boincteam_load(boincteam_lookup_id($team_id));

  $is_boinc_wide = ($boincteam->seti_id > 0) ? TRUE : FALSE;

  $form_state['storage']['team_id'] = $team_id;
  $form_state['storage']['is_boinc_wide'] = $is_boinc_wide;

  $default = array(
    'name' => $boincteam->name,
    'name_html' => $boincteam->name_html,
    'website' => $boincteam->url,
    'type' => $boincteam->type,
    'country' => $boincteam->country,
    'joinable' => $boincteam->joinable,
    'description' => $boincteam->description,
  );

  // Standard option sets
  $form['boolean_options'] = array(
    '#type' => 'value',
    '#value' => array(1 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), 0 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes')),
  );

  // Vocabulary based option sets
  $form['type_options'] = array(
    '#type' => 'value',
    '#value' => array(0 => bts('Choose type', array(), NULL, 'boinc:form-choose'))
  );
  $vocabs = taxonomy_get_vocabularies(NULL);
  foreach ($vocabs as $vocab) {
    switch ($vocab->name) {
    case 'Teams':
      $team_types = taxonomy_get_tree($vocab->vid);
      if (module_exists('internationalization')) {
        $team_types = i18ntaxonomy_localize_terms($team_types);
      }
      foreach ($team_types as $team_type) {
        $form['type_options']['#value'][$team_type->tid] = $team_type->name;
      }
      break;
    default:
    }
  }
  $form['country_options'] = array(
    '#type' => 'value',
    '#value' => boinccore_get_country_list(),
  );

  // Form elements
  $form['name'] = array(
    '#title' => bts('Team name', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['name'],
    '#size' => 34,
    '#description' => bts('Text only, no HTML tags', array(), NULL, 'boinc:team-form-help'),
    '#disabled' => $is_boinc_wide,
  );
  $form['name_html'] = array(
    '#title' => bts('Team name -- HTML version (optional)', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['name_html'],
    '#size' => 34,
    '#description' => bts('You may use limited HTML tags', array(), NULL, 'boinc:team-form-help'),
    '#disabled' => $is_boinc_wide,
  );
  $form['website'] = array(
    '#title' => bts('Team website (optional)', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['website'],
    '#size' => 34,
    '#description' => bts("Displayed on the team's page", array(), NULL, 'boinc:team-create/edit'),
    '#disabled' => $is_boinc_wide,
  );
  $form['type'] = array(
    '#title' => bts('Type of team', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'select',
    '#options' => $form['type_options']['#value'],
    '#default_value' => $default['type'],
    '#disabled' => $is_boinc_wide,
  );
  $form['country'] = array(
    '#title' => bts('Country', array(), NULL, 'boinc:country-of-origin'),
    '#type' => 'select',
    '#options' => $form['country_options']['#value'],
    '#default_value' => $default['country'],
    '#disabled' => $is_boinc_wide,
  );
  $form['joinable'] = array(
    '#title' => bts('Accept new members?', array(), NULL, 'boinc:team-create/edit'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['joinable'],
    '#size' => 34,
    '#disabled' => $is_boinc_wide,
  );
  if (!$is_boinc_wide) {
    $form['description'] = array(
      '#title' => bts('Description of team', array(), NULL, 'boinc:team-create/edit'),
      '#type' => 'textarea',
      '#default_value' => $default['description'],
      '#size' => 5,
      '#description' => NULL,
    );
    $form['format'] = filter_form();

    // Form control
    $form['form control tabs prefix'] = array(
      '#value' => '<ul class="form-control tab-list">'
    );
    $form['submit'] = array(
      '#prefix' => '<li class="first tab">',
      '#type' => 'submit',
      '#value' => bts('Save changes', array(), NULL, 'boinc:form-save'),
      '#suffix' => '</li>',
    );
    $form['form control tabs'] = array(
      '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), strstr($_GET['q'], '/edit', TRUE)) . '</li>'
    );
  }
  else {
    $form['description'] = array(
      '#prefix' => '<div class="form-item"><label>' . bts('Description', array(), NULL, 'boinc:team-description') . ':</label></div><div class="form-item">',
      '#value' => $default['description'],
      '#suffix' => '</div>',
    );
    drupal_set_message(
      bts('This is a BOINC-wide team. Changes can be made at the !site.',
        array('!site' => l(bts('BOINC-wide teams site', array(), NULL, 'boinc:link-to-BOINC-wide-teams-Website'),
          'http://boinc.berkeley.edu/teams/'
        )),
        NULL, 'boinc:team-edit-warning-message'),
      'warning');
  }

  $form['#redirect'] = "community/teams/{$team_id}";

  return $form;
}

/**
 * The edit team validation handler
 */
function boincteam_edit_form_validate($form, &$form_state) {

  $values = $form_state['values'];
  $name = $values['name'];

  if ($form_state['storage']['is_boinc_wide']) {
    form_set_error('none', bts('This team is managed by the BOINC-wide teams system and cannot be updated here.', array(), NULL, 'boinc:team-create/edit'));
  }
  else {
    if (!$name) {
      form_set_error('name', bts('Team name is required.', array(), NULL, 'boinc:team-create/edit'));
    }
    else {
      $team = node_load($form_state['storage']['team_id']);
      if ($name != $team->title) {
        // If changing the name ("title" in Drupal terms), check that the new name is available
        require_boinc('boinc_db');
        if (BoincTeam::lookup_name($name)) {
          form_set_error('name', bts('A team named "@name" already exists.',
            array('@name' => $name), NULL, 'boinc:team-create/edit'));
        }
      }
    }

    if (!$values['type'] OR !isset($values['type_options'][$values['type']])) {
      form_set_error('type', bts('Please select a team type.', array(), NULL, 'boinc:team-create/edit'));
    }
  }
}

/**
 * The edit team submit handler
 */
function boincteam_edit_form_submit($form, &$form_state) {

  $team_id = $form_state['storage']['team_id'];
  $values = $form_state['values'];

  $team = node_load($team_id);
  $boincteam_id = boincteam_lookup_id($team_id);

  $input_format = !empty($values['format']) ? $values['format'] : 4;
  $values['description'] = check_markup($values['description'], $input_format);

  // Update the team in the BOINC db
  db_set_active('boinc_rw');
  db_query("
    UPDATE {team} SET
      name = '%s',
      name_lc = '%s',
      name_html = '%s',
      url = '%s',
      description = '%s',
      type = %d,
      country='%s',
      joinable=%d
    WHERE id='%d'",
    $values['name'],
    strtolower($values['name']),
    $values['name_html'],
    $values['website'],
    $values['description'],
    $values['type'],
    $values['country'],
    $values['joinable'],
    $boincteam_id
  );
  db_set_active('default');

  // Update the team node in Drupal

  $team->title = $values['name'];
  $team->body = $values['description'];
  $team->teaser = node_teaser($values['description']);

  /*
  $node->field_description[]['value'] = $boincteam->description;
  $node->field_url[]['value'] = $boincteam->url;
  $node->field_country[]['value'] = $boincteam->country;
  */

  // Replace any existing taxonomy with the new one
  $team->taxonomy = array(taxonomy_get_term($values['type']));

  // Save the team node
  node_save($team);

  drupal_set_message(t('Details for "@team" have been updated.',
    array('@team' => $values['name'])));

  // The storage variable quietly kills redirection for some reason... unset it
  unset($form_state['storage']);
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Add team admin handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * The definition of the add team admin form
 */
function boincteam_add_admin_form(&$form_state, $team_id) {
  $form = array();

  $team = node_load($team_id);
  $boincteam = boincteam_load(boincteam_lookup_id($team_id));

  $form_state['storage']['team_id'] = $team_id;

  $default = array(
    'username' => '',
  );

  // Form elements
  $form['username'] = array(
    '#title' => bts('Enter BOINC username', array(), NULL, 'boinc:team--add-admin'),
    '#description' => bts('The number appearing in the suffix is the BOINC id. You can find a user\'s BOINC id on their user profile page.'),
    '#type' => 'textfield',
    '#default_value' => $default['username'],
    '#size' => 34,
    '#autocomplete_path' => "community/teams/${team_id}/user-name-autocomplete",
  );

  // Form control
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">'
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => bts('Add', array(), NULL, 'boinc:form-add'),
    '#suffix' => '</li>',
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), strstr($_GET['q'], '/edit', TRUE)) . '</li>'
  );

  //$form['#redirect'] = "community/teams/{$team_id}";

  return $form;
}

/**
 * The add team admin validation handler
 */
function boincteam_add_admin_form_validate($form, &$form_state) {

  $team_id = $form_state['storage']['team_id'];
  $values = $form_state['values'];
  $team = node_load($team_id);

  if (!$values['username']) {
    form_set_error('username', bts('BOINC username is required.', array(), NULL, 'boinc:team-add-admin'));
  }
  else {
    // Load user account associated with username
    $account = boincuser_privatemsg_name_lookup($values['username']);
    // Validate the account
    if ((!$account) OR ($account->team != $team_id)) {
      form_set_error('username', bts('There is no user on your team with name @username.',
        array(
          '@username' => $account->boincuser_name
        ),
      NULL, 'boinc:team-add-admin'));
    }
    elseif (boincteam_is_founder($team_id, $account->uid)) {
      form_set_error('username', bts('@user is the founder of @team! Team founder already have all admin privileges.',
        array(
          '@user' => $account->boincuser_name,
          '@team' => $team->title,
        ),
        NULL, 'boinc:team-add-admin'));
    }
    elseif (boincteam_is_admin($team_id, $account->uid)) {
      form_set_error('username', bts('@user is already an admin of @team.',
        array(
          '@user' => $account->boincuser_name,
          '@team' => $team->title,
        ),
        NULL, 'boinc:team-add-admin'));
    }
    else {
      $form_state['storage']['boincuser_id'] = $account->boincuser_id;
      $form_state['storage']['boincuser_name'] = $account->boincuser_name;
    }
  }
}

/**
 * The add team admin submit handler
 */
function boincteam_add_admin_form_submit($form, &$form_state) {

  $team_id = $form_state['storage']['team_id'];
  $boincuser_id = $form_state['storage']['boincuser_id'];
  $user_name = $form_state['storage']['boincuser_name'];
  $values = $form_state['values'];

  $team = node_load($team_id);
  $boincteam_id = boincteam_lookup_id($team_id);

  // Update the team in the BOINC db
  db_set_active('boinc_rw');
  db_query("
    INSERT INTO {team_admin} SET
      teamid = '%d',
      userid = '%d',
      create_time = '%d'",
    $boincteam_id,
    $boincuser_id,
    time()
  );
  db_set_active('default');

  // Could assign a role in Drupal here, as needed


  drupal_set_message(t('@user has been added as an admin.',
    array('@user' => $user_name)));

  // The storage variable quietly kills redirection for some reason... unset it
  unset($form_state['storage']);
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Team email functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */


/**
 * Send all team members an email message function
 */
function boincteam_sendmessagetoteam(&$form_state, $team_id) {
  global $user;
  $form = array();

  if (isset($form_state['values'])) {
    $subject   = $form_state['values']['subject'];
    $body      = $form_state['values']['body'];
  }
  else {
    $subject   = '';
    $body      = '';
  }

  // Title
  $mytitle = bts('Send E-mail To All Team Members', array(), NULL, 'boinc:team-message-form');
  drupal_set_title($mytitle);

  $form['emailteam']['header'] = array(
    '#value'              => '<h1>' . $mytitle . '</h1>',
    '#weight'             => '-10',
    '#prefix'             => "<div id='sendmessageteammembers-header'>",
    '#suffix'             => "</div>",
  );

  $form['emailteam']['instructions'] = array(
    '#value'              => bts('This will send an email to all team members. If a team member has opt-ed out of notification e-mails, they will not receive your message. At the bottom of your message, there will be a link allowing team members to send you a Direct Message. This link is added automatically, you do not need to add it below in the Message box. Your email address will not be shown to the recipients.', array(), NULL, 'boinc:team-message-form'),
    '#weight'             => '-8',
    '#prefix'             => "<div id='sendmessageteammembers-instructions'>",
    '#suffix'             => "</div>",
  );

  // Subject
  $form['emailteam']['subject'] = array(
    '#type'               => 'textfield',
    '#title'              => bts('Subject', array(), NULL, 'boinc:team-message-form'),
    '#size'               => 50,
    '#maxlength'          => 255,
    '#default_value'      => $subject,
    '#weight'             => -5,
  );

  // Body
  $form['emailteam']['body'] = array(
    '#type'               => 'textarea',
    '#title'              => bts('Message', array(), NULL, 'boinc:team-message-form'),
    '#rows'               => 8,
    '#weight'             => 0,
    '#default_value'      => $body,
    '#resizable'          => TRUE,
  );

  // checkbox for 'sent to self'
  $form['emailteam']['selfsend'] = array(
    '#type'    => 'checkbox',
    '#title'   => bts('Check this box if you wish to be sent a copy of your message to your email address.', array(), NULL, 'boinc:team-message-form'),
    '#weight'  => 10,
    '#prefix'  => '<div class="clearfix" id="confirm-checkbox">',
    '#suffix'  => '</div>',
  );

  // Add a captcha to form
  if (module_exists('captcha')) {
    $form['register_captcha'] = array(
        '#type' => 'captcha',
        '#weight' => 1000,
    );
  }

  // form buttons
  $form['emailteam']['form control tabs prefix'] = array(
    '#value'  => '<ul class="form-control tab-list">',
    '#weight' => 1000,
  );

  $form['emailteam']['submit'] = array(
    '#type'               => 'submit',
    '#value'              => bts('Send message', array(), NULL, 'boinc:form-submit'),
    '#submit'             => array('boincteam_sendmessagetoteam_submit'),
    '#validate'           => array('boincteam_sendmessagetoteam_validate'),
    '#weight'             => 1001,
    '#prefix'             => '<li class="first tab">',
    '#suffix'             => '</li>'
  );

  $title = bts('Cancel', array(), NULL, 'boinc:form-cancel');
  $url = "community/teams/{$team_id}";
  $form['emailteam']['cancel'] = array(
    '#value'              => l($title, $url, array('attributes' => array('id' => 'edit-cancel'))),
    '#weight'             => 1005,
    '#prefix'             => '<li class="tab">',
    '#suffix'             => '</li>'
  );

  $form['emailteam']['form control tabs suffix'] = array(
    '#value'  => '</ul>',
    '#weight' => 1010,
  );

  // Add team id information to form_state
  $form['_team_id'] = array(
    '#type'  => 'value',
    '#value' => $team_id,
  );

  // Add sender user object to form_state
  $form['_senderuid'] = array(
    '#type'  => 'value',
    '#value' => $user->uid,
  );

  // redirect after sending message
  $form['#redirect'] = "community/teams/{$team_id}";

  return $form;
}

/**
 * Send all team members validation handler
 */
function boincteam_sendmessagetoteam_validate($form, &$form_state) {
  $message = $form_state['values'];
  $message['timestamp'] = time();
  // Avoid subjects which only consist of a space as these can not be clicked.
  $message['subject'] = trim($message['subject']);

  $trimed_body = trim(truncate_utf8(strip_tags($message['body']), 50, TRUE, TRUE));
  if (empty($message['subject']) && !empty($trimed_body)) {
    $message['subject'] = $trimed_body;
    $form_state['values']['subject'] = $message['subject'];
  }

  // Check subject line
  $message['subject'] = trim($message['subject']);
  if (empty($message['subject'])) {
    form_set_error('subject',
      bts('Not allowed to send a message without subject', array(), NULL, 'boinc:team-message-form')
    );
  }

  // Check body
  if ( ($message['body'] === NULL || $message['body'] === '') ) {
    form_set_error('body',
      bts('Not allowed to send an empty message.', array(), NULL, 'boinc:team-message-form')
    );
  }

  // Check team id
  if (empty($message['_team_id'])) {
    form_set_error('team_id',
      bts('Error, no team_id supplied.', array(), NULL, 'boinc:team-message-form')
    );
  }

  $form_state['validated_built_message'] = $message;
}

/**
 * Send all team members submit handler
 */
function boincteam_sendmessagetoteam_submit($form, &$form_state) {
  module_load_include('inc', 'rules', 'modules/system.rules');
  global $base_url;
  global $base_path;

  $suid = $form_state['values']['_senderuid'];
  $account = user_load(array('uid' => $suid ));
  $site_url = $base_url . $base_path . "messages/new/" . $suid;

  // Form is validated, now send to all team members. Lookup boinc
  // id of all team members, and then use user_load() to load user
  // object. Send message using email.
  //
  $boinc_id = boincteam_lookup_id($form_state['values']['_team_id']);
  // Email addresses, with those who have opt-ed out of notification
  // emails removed.
  $member_emails = _boincteam_emails($boinc_id);
  // Remove sender's email address, unless selfsend is true.
  if ( !$form_state['values']['selfsend'] ) {
    $pos = array_search($account->mail, $member_emails);
    unset($member_emails[$pos]);
  }

  // Add link at bottom of body
  $mybody = $form_state['validated_built_message']['body'];
  $mybody .= "\n\n"
      . "To reply to the sender using a Direct Message (DM), please use this link:\n"
      . "${site_url}";

  // Transform HTML body into plain text
  $mybody = drupal_html_to_text($mybody);

  $emailsettings = array(
    'from'    => '',
    'to'      => "webmaster@{$base_url}",
    'boincteam_headers' => array(
      'Bcc'     => implode(', ', $member_emails),
    ),
    'subject' => $form_state['validated_built_message']['subject'],
    'message' => $mybody,
  );
  rules_action_mail($emailsettings);

  $om = bts(
    'Sent your email message to !number team members.',
    array( '!number' => count($member_emails) ),
    'NULL',
    'boinc:team-message-sent'
  );
  drupal_set_message($om, 'info');

  $form_state['redirect'] = "community/teams/{$form_state['values']['_team_id']}";
}
