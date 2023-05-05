<?php
// $Id$

/**
 * Form submission handlers and data processing functions are contained
 * herein to prevent littering of the main module file.
 */

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * New team forum form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * The definition of the create team form
 */
function boincteam_forum_create_form(&$form_state) {
  $form = array();

  global $user;
  $account = user_load($user->uid);

  $default = array(
    'title' => '',
    'min_time_between_posts' => '',
    'min_total_credit_to_post' => '',
    'min_avg_credit_to_post' => '',
    'public' => 0,
    'description' => '',
  );

  // Standard option sets
  $form['boolean_options'] = array(
    '#type' => 'value',
    '#value' => array(1 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), 0 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes')),
  );

  // Form elements
  $form['title'] = array(
    '#title' => bts('Message board title', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['title'],
    '#size' => 34,
    '#description' => bts('Text only, no HTML tags', array(), NULL, 'boinc:team-form-help'),
  );
  $form['min_time_between_posts'] = array(
    '#title' => bts('Minimum time between posts', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['min_time_between_posts'],
    '#size' => 12,
    '#description' => bts('seconds', array(), NULL, 'boinc:unit-of-time'),
  );
  $form['min_total_credit_to_post'] = array(
    '#title' => bts('Minimum total credit to post', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['min_total_credit_to_post'],
    '#size' => 12,
  );
  $form['min_avg_credit_to_post'] = array(
    '#title' => bts('Minimum avg credit to post', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['min_avg_credit_to_post'],
    '#size' => 12,
  );
  $form['public'] = array(
    '#title' => bts('Allow public to read board?', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['public'],
  );
  $form['description'] = array(
    '#title' => bts('Description', array(), NULL, 'boinc:team-forum-create/edit:-1:ignoreoverwrite'),
    '#type' => 'textarea',
    '#default_value' => $default['description'],
    '#size' => 5,
  );

  // Form control
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">'
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => bts('Create message board', array(), NULL, 'boinc:team-forum-create/edit'),
    '#suffix' => '</li>',
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), strstr($_GET['q'], '/forum/add', TRUE)) . '</li>'
  );

  $form['#redirect'] = "community/teams/{$account->team}";

  return $form;
}

/**
 * The create team forum validation handler
 */
function boincteam_forum_create_form_validate($form, &$form_state) {

  $values = $form_state['values'];

  if (!$values['title']) {
    form_set_error('name', bts('Message board title is required.', array(), NULL, 'boinc:team-forum-create/edit'));
  }
  if (!is_numeric($values['min_time_between_posts']) OR
      $values['min_time_between_posts'] < 0) {
    form_set_error('min_time_between_posts',
      bts('Please set the minimum time required between posts.', array(), NULL, 'boinc:team-forum-create/edit')
    );
  }
  if (!is_numeric($values['min_total_credit_to_post']) OR
      $values['min_total_credit_to_post'] < 0) {
    form_set_error('min_total_credit_to_post',
      bts('Please set the minimum total credit that a user'
        . ' must earn in order to post to this message board.',
        array(), NULL, 'boinc:team-forum-create/edit')
    );
  }
  if (!is_numeric($values['min_avg_credit_to_post']) OR
      $values['min_avg_credit_to_post'] < 0) {
    form_set_error('min_avg_credit_to_post',
      bts('Please set the minimum average credit that a user'
        . ' is required to have in order to post to this message board.',
        array(), NULL, 'boinc:team-forum-create/edit')
    );
  }
}

/**
 * The create team forum submit handler
 */
function boincteam_forum_create_form_submit($form, &$form_state) {

  $values = $form_state['values'];

  global $user;
  $account = user_load($user->uid);

  // Add the team forum to the db
  db_query("
    INSERT INTO {boincteam_forum} SET
      nid = %d,
      title = '%s',
      min_time_between_posts = '%d',
      min_total_credit_to_post = '%d',
      min_avg_credit_to_post = '%d',
      public = %d,
      description = '%s',
      created = UNIX_TIMESTAMP(),
      updated = UNIX_TIMESTAMP()",
    $account->team,
    $values['title'],
    (int) $values['min_time_between_posts'],
    (int) $values['min_total_credit_to_post'],
    (int) $values['min_avg_credit_to_post'],
    ($values['public']) ? 1 : 0,
    $values['description']
  );

  drupal_set_message(t('Message board "@forum" has been created.',
    array('@forum' => $values['title'])));
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Edit team form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * The definition of the edit team forum form
 */
function boincteam_forum_edit_form(&$form_state, $tfid) {
  $form = array();
  $boincteam_forum = boincteam_forum_load($tfid);

  $form_state['storage']['tfid'] = $tfid;

  $default = array(
    'title' => $boincteam_forum->title,
    'min_time_between_posts' => $boincteam_forum->min_time_between_posts,
    'min_total_credit_to_post' => $boincteam_forum->min_total_credit_to_post,
    'min_avg_credit_to_post' => $boincteam_forum->min_avg_credit_to_post,
    'public' => ($boincteam_forum->public) ? 1 : 0,
    'description' => $boincteam_forum->description,
  );

  // Standard option sets
  $form['boolean_options'] = array(
    '#type' => 'value',
    '#value' => array(1 => bts('yes', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-no'), 0 => bts('no', array(), NULL, 'boinc:form-yes-no:-1:binary-form-option-pairs-with-yes')),
  );

  // Form elements
  $form['title'] = array(
    '#title' => bts('Message board title', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['title'],
    '#size' => 34,
    '#description' => bts('Text only, no HTML tags', array(), NULL, 'boinc:team-form-help'),
  );
  $form['min_time_between_posts'] = array(
    '#title' => bts('Minimum time between posts', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['min_time_between_posts'],
    '#size' => 12,
    '#description' => bts('seconds', array(), NULL, 'boinc:unit-of-time'),
  );
  $form['min_total_credit_to_post'] = array(
    '#title' => bts('Minimum total credit to post', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['min_total_credit_to_post'],
    '#size' => 12,
  );
  $form['min_avg_credit_to_post'] = array(
    '#title' => bts('Minimum avg credit to post', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'textfield',
    '#default_value' => $default['min_avg_credit_to_post'],
    '#size' => 12,
  );
  $form['public'] = array(
    '#title' => bts('Allow public to read board?', array(), NULL, 'boinc:team-forum-create/edit'),
    '#type' => 'radios',
    '#options' => $form['boolean_options']['#value'],
    '#attributes' => array('class' => 'fancy'),
    '#default_value' => $default['public'],
  );
  $form['description'] = array(
    '#title' => bts('Description', array(), NULL, 'boinc:team-forum-create/edit:-1:ignoreoverwrite'),
    '#type' => 'textarea',
    '#default_value' => $default['description'],
    '#size' => 5,
  );

  // Form control
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">'
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => bts('Save message board', array(), NULL, 'boinc:team-forum-create/edit'),
    '#suffix' => '</li>',
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), strstr($_GET['q'], '/forum', TRUE)) . '</li>'
  );

  $form['#redirect'] = "community/teams/{$boincteam_forum->nid}";

  return $form;
}

/**
 * The edit team forum validation handler
 */
function boincteam_forum_edit_form_validate($form, &$form_state) {

  $values = $form_state['values'];

  if (!$values['title']) {
    form_set_error('name', bts('Message board title is required.', array(), NULL, 'boinc:team-forum-create/edit'));
  }
  if (!is_numeric($values['min_time_between_posts']) OR
      $values['min_time_between_posts'] < 0) {
    form_set_error('min_time_between_posts',
      bts('Please set the minimum time required between posts.', array(), NULL, 'boinc:team-forum-create/edit')
    );
  }
  if (!is_numeric($values['min_total_credit_to_post']) OR
      $values['min_total_credit_to_post'] < 0) {
    form_set_error('min_total_credit_to_post',
      bts('Please set the minimum total credit that a user'
        . ' must earn in order to post to this message board.',
        array(), NULL, 'boinc:team-forum-create/edit')
    );
  }
  if (!is_numeric($values['min_avg_credit_to_post']) OR
      $values['min_avg_credit_to_post'] < 0) {
    form_set_error('min_avg_credit_to_post',
      bts('Please set the minimum average credit that a user'
        . ' is required to have in order to post to this message board.',
        array(), NULL, 'boinc:team-forum-create/edit')
    );
  }
}

/**
 * The edit team forum submit handler
 */
function boincteam_forum_edit_form_submit($form, &$form_state) {

  $tfid = $form_state['storage']['tfid'];
  $values = $form_state['values'];

  // Update the team forum in the db
  db_query("
    UPDATE {boincteam_forum} SET
      title = '%s',
      min_time_between_posts = '%d',
      min_total_credit_to_post = '%d',
      min_avg_credit_to_post = '%d',
      public = %d,
      description = '%s',
      updated = UNIX_TIMESTAMP()
    WHERE tfid='%d'",
    $values['title'],
    (int) $values['min_time_between_posts'],
    (int) $values['min_total_credit_to_post'],
    (int) $values['min_avg_credit_to_post'],
    ($values['public']) ? 1 : 0,
    $values['description'],
    $tfid
  );

  drupal_set_message(t('Details for message board "@forum" have been updated.',
    array('@forum' => $values['title'])));

  // The storage variable quietly kills redirection for some reason... unset it
  unset($form_state['storage']);
}
