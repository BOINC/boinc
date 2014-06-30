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
    '#value' => array(1 => t('yes'), 0 => t('no')),
  );
  
  // Vocabulary based option sets
  $form['type_options'] = array(
    '#type' => 'value',
    '#value' => array(0 => t('Choose type'))
  );
  $vocabs = taxonomy_get_vocabularies(NULL);
  foreach ($vocabs as $vocab) {
    switch ($vocab->name) {
    case 'Teams':
      $team_types = taxonomy_get_tree($vocab->vid);
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
    '#title' => t('Team name'),
    '#type' => 'textfield',
    '#default_value' => $default['name'],
    '#size' => 34,
    '#description' => t('Text only, no HTML tags'),
  );
  $form['name_html'] = array(
    '#title' => t('Team name -- HTML version (optional)'),
    '#type' => 'textfield',
    '#default_value' => $default['name_html'],
    '#size' => 34,
    '#description' => t('You may use limited HTML tags'),
  );
  $form['website'] = array(
    '#title' => t('Team website (optional)'),
    '#type' => 'textfield',
    '#default_value' => $default['website'],
    '#size' => 34,
    '#description' => t('Displayed on the team\'s page'),
  );
  $form['type'] = array(
    '#title' => t('Type of team'),
    '#type' => 'select',
    '#options' => $form['type_options']['#value'],
    '#default_value' => $default['type'],
  );
  $form['country'] = array(
    '#title' => t('Country'),
    '#type' => 'select',
    '#options' => $form['country_options']['#value'],
    '#default_value' => $default['country'],
  );
  $form['joinable'] = array(
    '#title' => t('Accept new members?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['joinable'],
    '#size' => 34,
  );
  $form['description'] = array(
    '#title' => t('Description of team'),
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
    '#value' => t('Save team'),
    '#suffix' => '</li>',
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), $_GET['q']) . '</li>'
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
    form_set_error('name', t('Team name is required.'));
  }
  else {
    require_boinc('boinc_db');
    if (BoincTeam::lookup_name($name)) {
      form_set_error('name', t('A team named "@name" already exists.',
        array('@name' => $name)));
    }
  }
  
  if (!$values['type'] OR !isset($values['type_options'][$values['type']])) {
    form_set_error('type', t('Please select a team type.'));
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
    drupal_set_message(t('Teams cannot be created at this time. Please 
      contact the @project administrators!', array('@project' => PROJECT)));
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
    drupal_set_message(t('Teams cannot be created at this time. Please 
      contact the @project administrators!', array('@project' => PROJECT)));
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
  
  $form_state['storage']['team_id'] = $team_id;
  
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
    '#value' => array(1 => t('yes'), 0 => t('no')),
  );
  
  // Vocabulary based option sets
  $form['type_options'] = array(
    '#type' => 'value',
    '#value' => array(0 => t('Choose type'))
  );
  $vocabs = taxonomy_get_vocabularies(NULL);
  foreach ($vocabs as $vocab) {
    switch ($vocab->name) {
    case 'Teams':
      $team_types = taxonomy_get_tree($vocab->vid);
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
    '#title' => t('Team name'),
    '#type' => 'textfield',
    '#default_value' => $default['name'],
    '#size' => 34,
    '#description' => t('Text only, no HTML tags'),
  );
  $form['name_html'] = array(
    '#title' => t('Team name -- HTML version (optional)'),
    '#type' => 'textfield',
    '#default_value' => $default['name_html'],
    '#size' => 34,
    '#description' => t('You may use limited HTML tags'),
  );
  $form['website'] = array(
    '#title' => t('Team website (optional)'),
    '#type' => 'textfield',
    '#default_value' => $default['website'],
    '#size' => 34,
    '#description' => t('Displayed on the team\'s page'),
  );
  $form['type'] = array(
    '#title' => t('Type of team'),
    '#type' => 'select',
    '#options' => $form['type_options']['#value'],
    '#default_value' => $default['type'],
  );
  $form['country'] = array(
    '#title' => t('Country'),
    '#type' => 'select',
    '#options' => $form['country_options']['#value'],
    '#default_value' => $default['country'],
  );
  $form['joinable'] = array(
    '#title' => t('Accept new members?'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['joinable'],
    '#size' => 34,
  );
  $form['description'] = array(
    '#title' => t('Description of team'),
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
    '#value' => t('Save changes'),
    '#suffix' => '</li>',
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), strstr($_GET['q'], '/edit', TRUE)) . '</li>'
  );
  
  $form['#redirect'] = "community/teams/{$team_id}";
  
  return $form;
}

/**
 * The edit team validation handler
 */
function boincteam_edit_form_validate($form, &$form_state) {

  $values = $form_state['values'];
  $name = $values['name'];

  if (!$name) {
    form_set_error('name', t('Team name is required.'));
  }
  else {
    require_boinc('boinc_db');
    if (BoincTeam::lookup_name($name)) {
      form_set_error('name', t('A team named "@name" already exists.',
        array('@name' => $name)));
    }
  }
  
  if (!$values['type'] OR !isset($values['type_options'][$values['type']])) {
    form_set_error('type', t('Please select a team type.'));
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
  
  // Update the team in the BOINC db
  db_set_active('boinc');
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
    'email' => '',
  );
  
  // Form elements
  $form['email'] = array(
    '#title' => t('Enter email address'),
    '#type' => 'textfield',
    '#default_value' => $default['email'],
    '#size' => 34,
  );
  
  // Form control
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">'
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => t('Add'),
    '#suffix' => '</li>',
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(t('Cancel'), strstr($_GET['q'], '/edit', TRUE)) . '</li>'
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
  
  if (!$values['email']) {
    form_set_error('email', t('Email address is required.'));
  }
  else {
    $account = user_load(array('mail' => $values['email']));
    if ((!$account) OR ($account->team != $team_id)) {
      form_set_error('email', t('There is no user on your team with that email
        address.'));
    }
    elseif (boincteam_is_founder($team_id, $account->uid)) {
      form_set_error('email', t('@user is the founder of @team!',
        array(
          '@user' => $account->boincuser_name,
          '@team' => $team->title,
        )
      ));
    }
    elseif (boincteam_is_admin($team_id, $account->uid)) {
      form_set_error('email', t('@user is already an admin of @team.',
        array(
          '@user' => $account->boincuser_name,
          '@team' => $team->title,
        )
      ));
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
  db_set_active('boinc');
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
