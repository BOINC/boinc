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
    'boincuser_delete_redirect' => variable_get('boincuser_delete_redirect', '<front>'),
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
    '#value' => "<p>" . t("This form will delete this user <strong>without any email notification</strong> sent to the user. Be very careful in deleting users using this form. Once you enter your password, check the checkbox, and click submit, the user's account will be deleted. This will occur <strong>immediately</strong>. There is no 'undo'!") . "</p><p>" . t("You are deleting the following user:") . "</p>",
    '#weight' => -1,
    '#prefix' => "<div id='delete-instructions'>",
    '#suffix' => "</div>",
  );

  $account = user_load($uid);
  if ($account) {
    drupal_set_title($account->boincuser_name);

    $form['account']['boincuser_name'] = array(
      '#value' => t('<li>BOINC username (public displayname): ') . $account->boincuser_name,
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
  require_boinc('user_util');
  require_boinc('delete_account');

  // This is the account to be deleted, and not the administrator's
  // account.
  $account = user_load(array('uid' => $form['#uid']));

  drupal_set_message(t('WARNING: Account @displayname, Drupal UID=@uid has been deleted.',
    array(
      '@uid' => $form['#uid'],
      '@displayname' => $account->boincuser_name,
    )), 'warning');

  // watchdog message
  watchdog('boincuser_delete', 'Deleting account drupal UID: %uid, BOINC id: %boincuser_id., BOINC displayname: %displayname',
  array(
    '%uid' => $form['#uid'],
    '%boincuser_id' => $account->boincuser_id,
    '%displayname' => $account->boincuser_name,
  ), WATCHDOG_NOTICE);

  // Delete the user
  $boinc_user = boincuser_load($account->uid, TRUE);
  wipe_account($boinc_user);
  user_delete(array(), $account->uid);

  drupal_goto('/admin/boinc/user_delete');
}
