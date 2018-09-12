<?php

/**
 * @file
 * BOINC user delete - Administration page
 *
 */

/**
 * Administrative settings page
 *
 * @return array
 *   a form array
 */
function boincuser_delete_settings() {
  $form = array();

  $default = array(
    'boincuser_delete_type' => variable_get('boincuser_delete_type', ''),
    'boincuser_delete_redirect' => variable_get('boincuser_delete_redirect', '<front>'),
  );

  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
  );
  $form['options']['help'] = array(
    '#value' => t('When a user deletes their account, which option is shown to the user? A soft/obfusate delete, a hard/wipe delete, or let the user chose between the two.'),
    '#weight' => 11,
  );
  $form['options']['boincuser_delete_type'] = array(
    '#type' => 'radios',
    '#title' => t('Type of delete'),
    '#default_value' => $default['boincuser_delete_type'],
    '#options' => array(
       'soft_obfuscate' => t('A soft/obfuscate delete. User\'s account is disabled, but some data is deleted.'),
       'hard_wipe' => t('A hard/wipe delete. User\'s account is deleted along with many data.'),
       'user_decides' => t('User is presented with radio buttons where they choose between the two options above.'),
    ),
    '#weight' => 21,
  );

  $form['redirect'] = array(
    '#type' => 'fieldset',
    '#title' => t('Redirect'),
  );
  $form['redirect']['boincuser_delete_redirect'] = array(
    '#type' => 'textfield',
    '#title' => t('Redirection page'),
    '#default_value' => $default['boincuser_delete_redirect'],
    '#description' => t('Choose where to redirect your users after account deletion. Any valid Drupal path will do, e.g. %front or %node', array('%front' => '<front>', '%node' => 'node/1')),
  );

  return system_settings_form($form);
}


/**
 * Function to delete as user as an admin, there is no email or
 * password checks.
 */
function boincuser_delete_admindelete(&$form_state, $uid) {
  $form = array();

  $form['#uid'] = $uid;

  $form['account']['help'] = array(
    '#value' => "<p>" . t("This form will delete this user <strong>without any email notification</strong> sent to the user. Be very careful in deleting users using this form. Once you select the delete type, check the checkbox, enter your password, and click submit, the user's account will be deleted. This will occur <strong>immediately</strong>. There is no 'undo'!") . "</p><p>" . t("You are deleting the following user, link opens in new window:") . "</p>",
    '#weight' => -1,
    '#prefix' => "<div id='delete-instructions'>",
    '#suffix' => "</div>",
  );

  $account = user_load($uid);
  if ($account) {
    drupal_set_title($account->boincuser_name);

    $form['account']['boincuser_name'] = array(
      '#value' => t('<li>BOINC username (public displayname): ') . l("{$account->boincuser_name}", "account/{$account->uid}", array('attributes' => array('target' => '_blank'))),
    );
    $form['account']['boincuser_id'] = array(
      '#value' => t('<li>BOINC user ID: ') . $account->boincuser_id,
    );
    $form['account']['drupal_name'] = array(
      '#value' => t('<li>Drupal username (internal): ') . $account->name,
    );
    $form['account']['user_id'] = array(
      '#value' => t('<li>Drupal user ID: ') . $account->uid,
    );

    $form['account']['user_delete_action'] = array(
        '#type' => 'radios',
        '#options' => array(
          'soft_obfuscate' => bts('<b>Soft delete</b> the account. The account will be disabled, and all posts/comments will be attributed to the Anonymous User. The user profile will be deleted, the host information deleted, and the user will be removed from any team.', array(), NULL, 'boinc:delete-user-account'),
          'hard_wipe' => bts('<b>Delete</b> the account. The account will be deleted, and all posts/comments will be attributed to the Anonymous User. The user profile will be deleted.', array(), NULL, 'boinc:delete-user-account'),),
      );

    $form['account']['surecheckbox'] = array(
      '#type' => 'checkbox',
      '#title' => t('I am <strong>sure</strong> I know what I am doing. I am <u><strong>deleting user</strong></u> %name.',
        array('%name' => $account->boincuser_name)
      ),
      '#default_value' => FALSE,
    );

    $form['account']['current_pass'] = array(
      '#type' => 'password',
      '#title' => bts('Enter your password before clicking Submit, or click Cancel to return to the user\'s edit page', array(), NULL, 'boinc:delete-user-account'),
      '#size' => 17,
      '#attributes' => array(
        'autocomplete' => 'off',
      ),
      '#weight' => 25,
    );

    // Action buttons
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
    $form['actions']['cancel'] = array(
      '#type' => 'button',
      '#value' => t('Cancel'),
      '#executes_submit_callback' => TRUE,
      '#validate' => array(),
      '#submit' => array('_boincuser_delete_cancel'),
    );

    $form['#redirect'] = 'account';
  }
  else {
    $form['noaccount'] = array(
      '#value' => t('No such user with UID: %uid', array('%uid' => $uid)),
    );
  }

  return $form;
}

/**
 * Validation function for admin delete user.
 */
function boincuser_delete_admindelete_validate($form, &$form_state) {
  global $user;
  $boinc_user = boincuser_load($user->uid, TRUE);

  if ($form_state['values']['user_delete_action'] == '') {
    form_set_error('user_delete_action', t('Please select an action to perform using the radio buttons.'));
  }

  if ( ($form_state['values']['user_delete_action'] != 'soft_obfuscate') and ($form_state['values']['user_delete_action'] != 'hard_wipe') ) {
    form_set_error('user_delete_action', t('User Delete action not a predefined value, unknown error in radio buttons.'));
  }

  if (!($form_state['values']['surecheckbox'])) {
    return form_set_error('surecheckbox', t('Please confirm you are sure you want to delete this account.'));
  }

  if (_boincuser_delete_validatepasswd($boinc_user, $form_state['values']['current_pass'])) {
    return true;
  }
}

/**
 * Submit function for admin delete user.
 */
function boincuser_delete_admindelete_submit($form, &$form_state) {
  // This is the account to be deleted, and not the administrator's
  // account.
  $account = user_load(array('uid' => $form['#uid']));
  $action = $form_state['values']['user_delete_action'];

  drupal_set_message(t('WARNING: Account @displayname, Drupal UID=@uid has been deleted.',
    array(
      '@uid' => $form['#uid'],
      '@displayname' => $account->boincuser_name,
    )), 'warning');

  // Delete the user
  _boincuser_delete_deleteuser($account, $action);

  drupal_goto('/admin/boinc/user_delete');
}
