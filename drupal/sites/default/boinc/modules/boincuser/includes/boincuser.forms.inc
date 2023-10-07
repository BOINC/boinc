<?php
// $Id$

/**
 * Form submission handlers and data processing functions are contained
 * herein to prevent littering of the main module file.
 */


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Login form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Form validation handler for login
 */
function boincuser_login_validate($form, &$form_state) {
  /* // SAMPLE: To try validation with Drupal first...
  global $user;
  if (!empty($user->uid)) {
    // Authentication with Drupal was successful
    return;
  }*/

  // Set name for logging purposes
  $form_state['values']['name'] = $form_state['values']['email'];

  // Bypass BOINC validation if passed the name of user 1
  $user_1 = user_load(1);
  if ($form_state['values']['email'] == $user_1->name) {
    user_authenticate($form_state['values']);
    return;
  }

  // Call our custom authentication function to check for an existing BOINC user
  if (!boincuser_login_authenticate($form_state['values'])) {
    // Authentication failed; set an error accordingly
    form_set_error('name', bts('Sorry, unrecognized email address or password.', array(), NULL, 'boinc:forgot-password') . ' ' . l(bts('Have you forgotten your password?', array(), NULL, 'boinc:forgot-password'), 'user/password'));
  }
}

/**
 * Custom authentication function to check BOINC account information
 */
function boincuser_login_authenticate($form_values) {
  global $boincuser_authenticated;
  $lower_email_addr = strtolower($form_values['email']);
  $passwd = $form_values['pass'];
  $passwd_hash = md5($passwd.$lower_email_addr);

  // Include BOINC user library
  require_boinc('boinc_db');
  require_boinc('user_util');

  // Get the BOINC user and check credentials
  $boinc_user = BoincUser::lookup_email_addr($lower_email_addr);
  if (!$boinc_user) return false;

  if (!check_passwd_hash($boinc_user, $passwd_hash)) {
    return false;
  }

  // BOINC authentication successful; log in and synchronize accounts
  boincuser_login_register($boinc_user);
  // Write session, update timestamp, run user 'login' hook.
  user_authenticate_finalize($form_values);
  // Use a global variable to save the fact that we did authentication.
  $boincuser_authenticated = true;

  return true;
}

/**
 * Allow pre-existing BOINC users to login and automatically create
 * Drupal accounts
 */
function boincuser_login_register($boinc_user) {
  global $user;
  require_boinc('boinc_db');
  // If a Drupal account already exists, log in
  $existing_user = user_load(array('mail' => $boinc_user->email_addr));
  if (!empty($existing_user->uid)) {
    $user = $existing_user;
  } else {
    // Create a Drupal user from the BOINC user
    if ($user = boincuser_register_make_drupal_user($boinc_user)) {
      watchdog('user', 'New external user: %email using module %module.', array('%email' => $email_addr, '%module' => $module), WATCHDOG_NOTICE, l(t('edit'), 'user/'. $user->uid .'/edit'));
    }
  }
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Registration form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * New user registration validation handler.
 */
function boincuser_register_validate($form, &$form_state) {
  // Include BOINC database objects library
  require_boinc('boinc_db');

  // Check terms of use agreement
  $termsofuse = variable_get('boinc_weboptions_termsofuse', '');
  if (!empty($termsofuse)) {
    if (!$form_state['values']['termsofuse']['agreeTOU']) {
      form_set_error('termsofuse', bts('ERROR: You must acknowledge our terms of use by clicking the checkbox before registering for an account.', NULL, 'boinc:register-new-user'));
      return false;
    }
  }

  // Lower-case the email address
  $lower_email_addr = strtolower($form_state['values']['mail']);
  $tmp_user = BoincUser::lookup_prev_email_addr($lower_email_addr);
  if ($tmp_user) {
    // User already exists
    rules_invoke_event('boincuser_change_email_error', $lower_email_addr, variable_get('boinc_admin_mailing_list_subject_tag', ''));
    form_set_error('mail',
      bts('An account already exists for @email. The @project administrators have been notified.',
        array(
          '@email' => $lower_email_addr,
          '@project' => variable_get('site_name', 'Drupal-BOINC')
        ), NULL, 'boinc:register-new-user')
    );
  }

  // Check for an existing BOINC user
  // This is somewhat redundent as Drupal will also check if the email
  // is a duplicate. However, in the case where there is no Drupal
  // account, but a BOINC account exists with this email, the check
  // will fail.
  $boinc_user = BoincUser::lookup_email_addr($lower_email_addr);
  if ($boinc_user) {
    // User already exists
    form_set_error('mail', bts('An account already exists for @email. Log in or request password assistance to access your @project account.', array('@email' => $lower_email_addr, '@project' => PROJECT), NULL, 'boinc:add-new-user'));
    return false;
  }

  // Check Drupal name is not a duplicate, and create a proper drupal
  // name from the boinc username.
  $form_state['values']['name'] = create_proper_drupalname($form_state['values']['boincuser_name']);

  return true;
}

/**
 * Create BOINC account
 */
function boincuser_register_make_user($params) {
  // Include BOINC user library
  require_boinc('user_util');
  // Create the BOINC user
  $boinc_user = make_user($params['email_addr'], $params['name'], $params['passwd_hash'], $params['country'], $params['postal_code']);
  return $boinc_user;
}

/**
 * Create Drupal account from existing BOINC account
 */
function boincuser_register_make_drupal_user($boinc_user) {
  $account = null;
  if (is_numeric($boinc_user)) {
    $boinc_user = boincuser_load($boinc_user);
  }
  if ($boinc_user) {
    $account = boincuser_create_drupal_user($boinc_user);
    if ($account) {
      boincuser_create_drupal_profile($account, $boinc_user);
    }
  }
  return $account;
}

function boincuser_create_drupal_user($boinc_user) {
  require_boinc('forum_db');
  BoincForumPrefs::lookup($boinc_user);
  $account = NULL;
  $module = 'boincuser';

  // Verify that this account has not already been imported
  $already_imported = db_result(db_query('
    SELECT COUNT(*) FROM {boincuser} WHERE boinc_id = %d',
    $boinc_user->id)
  );
  if ($already_imported) {
    return NULL;
  }

  // Email is limited to 64 characters...
  $boinc_email = substr($boinc_user->email_addr, 0, 64);

  // Email should be unique
  $already_imported = db_result(db_query("
    SELECT COUNT(*) FROM {users} WHERE mail = '%s'",
    $boinc_email)
  );
  if ($already_imported) {
    drupal_set_message(t('An account for @email already exists', array('@email' => $boinc_email)), 'warning');
    watchdog('boincimport', 'An account for @email already exists', array('@email' => $boinc_email), WATCHDOG_WARNING);
    return NULL;
  }

  // Make sure name is unique
  $unique_name = create_proper_drupalname($boinc_user->name);

  $userinfo = array(
    'name' => $unique_name,
    'pass' => $boinc_user->passwd_hash,
    'mail' => $boinc_email,
    'signature' => $boinc_user->prefs->signature,
    'created' => $boinc_user->create_time,
    'init' => $boinc_email,
    'status' => 1,
    "authname_{$module}" => $boinc_email,
    'access' => time()
  );

  $in_penalty = ($boinc_user->prefs->banished_until > time());

  // Add user to community member role (if no current penalty)
  if (!$in_penalty) {
    $community_role = array_search('community member', user_roles(true));
    $userinfo['roles'] = array(
      $community_role => ''
    );
  }

  // Mark account as imported so a BOINC account is not created on Insert (i.e. user_save)
  $_SESSION['importedUser'] = true;
  $account = user_save('', $userinfo);

  // Drupal overrides the original registration date, so update to the correct date
  $account = user_save($account, array('created' => $userinfo['created']));

  // Terminate if an error occured during user_save().
  if (!$account) {
    drupal_set_message(t("Error saving user account."), 'error');
    return NULL;
  }

  // Cross reference the Drupal and BOINC accounts
  boincuser_account_cross_reference($account->uid, $boinc_user->id);

  if ($in_penalty) {
    // Import the timestamp of when the penalty period will be over
    $boincuser_record = array(
      'uid' => $account->uid,
      'penalty_expiration' => $boinc_user->prefs->banished_until,
    );
    drupal_write_record('boincuser', $boincuser_record, 'uid');
  }

  return $account;
}

function boincuser_create_drupal_profile($account, $boinc_user) {

  require_boinc('forum_db');
  BoincForumPrefs::lookup($boinc_user);

  $image_dir = boinc_get_path('user_image');

  // Automatically create a content profile
  $now = time();
  $profile_background = null;
  $profile_opinions = null;
  $profile_image = null;
  $avatar_image = null;
  $profile_approved = NULL;

  // Load the BOINC profile, if available
  if ($boinc_user->has_profile) {
    $boinc_profile = BoincProfile::lookup("userid={$boinc_user->id}");
    $profile_background = $boinc_profile->response1;
    $profile_opinions = $boinc_profile->response2;
    if ($boinc_profile->has_picture) {
      // Load picture; load validators; determine where to store it in Drupal
      $image_path = "{$image_dir}/{$boinc_user->id}.jpg";
      $profile_image = get_cck_image_object($image_path, 'field_profile_image', 'profile', TRUE);
      // If the user does not have a different image as an avatar, use this one
      if (!$boinc_user->prefs->avatar) {
        $avatar_image = get_cck_image_object($image_path, 'field_image', 'profile', TRUE);
      }
    }
    if ($boinc_user->prefs->avatar) {
      $image_path = "{$image_dir}/{$boinc_user->id}_avatar.jpg";
      $avatar_image = get_cck_image_object($image_path, 'field_image', 'profile', TRUE);
    }
    $profile_approved = $boinc_profile->verification ? TRUE : FALSE;
  }
  $default_input_format = db_result(db_query("
    SELECT format FROM {filter_formats} WHERE name = '%s'", 'Rich text'));
  if (!$default_input_format) $default_input_format = 1;

  // Populate the profile object
  $profile_node = new stdClass();
  $profile_node->title = $boinc_user->name;
  $profile_node->uid = $account->uid;
  $profile_node->body = '';
  $profile_node->type = 'profile';
  $profile_node->created = $now;
  $profile_node->changed = $now;
  $profile_node->status = 1;
  $profile_node->promote = 0;
  $profile_node->sticky = 0;
  $profile_node->format = $default_input_format;
  $profile_node->name = $account->name; // Set to link profile to account uid!
  $profile_node->field_country[]['value'] = $boinc_user->country;
  $profile_node->field_zip[]['value'] = $boinc_user->postal_code;
  $profile_node->field_url[]['value'] = $boinc_user->url;
  $profile_node->field_background[]['value'] = $profile_background;
  $profile_node->field_opinions[]['value'] = $profile_opinions;
  $profile_node->field_image[] = $avatar_image;
  $profile_node->field_profile_image[] = $profile_image;

  // Save the profile object to the database
  // This will automatically set the author uid if node->name has been set
  $profile_node = node_submit($profile_node);
  node_save($profile_node);

  // Mark profile as approved if it has already been vetted somehow
  if ($profile_approved OR !$boinc_user->has_profile) {
    $profile_node->moderate = 0;
    node_save($profile_node);
  }

  return;
}

function boincuser_account_cross_reference($uid, $boinc_id) {
    // Cross reference Drupal account with BOINC
    $reference = db_query("INSERT INTO {boincuser} SET uid=%d, boinc_id=%d", $uid, $boinc_id);
    if (!$reference) {
      drupal_set_message(t("Error connecting BOINC account."), 'error');
      return false;
    }
    return true;
}

/**
 * Convert any BOINC forum thread subscriptions to Drupal flag subscriptions
 */
function boincuser_pull_subscriptions($account = NULL) {
  if (!$account) {
    global $user;
    $account = user_load($user->id);
  }
  elseif (is_numeric($account)) {
    $account = user_load($account);
  }

  // Get any subscriptions for this user
  db_set_active('boinc_rw');
  $subscriptions = db_query('
    SELECT threadid FROM {subscriptions}
    WHERE userid = %d', $account->boincuser_id);
  db_set_active('default');

  $flag = flag_get_flag('subscriptions') or die('no "subscriptions" flag defined');
  $count = 0;
  while ($thread_id = db_result($subscriptions)) {
    // For each BOINC thread ID subscribed, look up the corresponding node ID
    $nid = db_result(db_query('
      SELECT nid FROM {boincimport_temp_topic}
      WHERE topic_id = %d', $thread_id));
    // Flag this node for the user
    if ($nid > 0) {
      $flag->flag('flag', $nid, $account);
      $count++;
    }
  }
  return $count;
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Account form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * User account update validation handler.
 */
function boincuser_account_validate($edit, $account) {
  // Include BOINC database objects library, load BOINC account data
  require_boinc('boinc_db');
  $boinc_user = BoincUser::lookup_email_addr($account->mail);
  $changing_email = ($edit['mail'] AND $edit['mail'] != $account->mail) ? true : false;
  $changing_pass = ($edit['pass']) ? true : false;
  if ($changing_email) {
    // E-mail address is set to change; check for an existing BOINC user
    // Check previous email addresses as well, this user's current
    // email cannot be the same as another user's previous email
    // address.

    // set email address lower-case
    $edit['mail'] = strtolower($edit['mail']);

    $boinc_user_already_exists = ( BoincUser::lookup_email_addr($edit['mail']) || BoincUser::lookup_prev_email_addr($edit['mail']) );
    if ($boinc_user_already_exists) {
      form_set_error('mail', bts('A BOINC account already exists for @email.', array('@email' => $edit['mail']), NULL, 'boinc:add-new-user'));
    }

    // Check email has not been changed in last X days (default X=7).
    $duration = 86400 * 7;
    if ( (($boinc_user->email_addr_change_time + $duration) > time()) and (!(user_access('administer users'))) ) {
      form_set_error('email_addr_change_time',
        bts('Your email address was changed within the past seven (7) days. You must wait until !futuredate to change your email again. If you need to reverse this change, please look for an email sent to !prev_email_addr.',
        array(
          '!futuredate' => date('F j, Y \a\t H:i T', $boinc_user->email_addr_change_time + $duration),
          '!prev_email_addr' => $boinc_user->previous_email_addr,
        ),
        NULL, 'boinc:account-credentials-change')
      );
    }
  }

  // If user is changing email or password, require that the current
  // password has been given as well.
  if (($changing_email OR $changing_pass) AND !user_access('administer users')) {
    // If changing email or password, require current password
    // (except in cases where password is being reset)
    if (isset($_SESSION['reset_pass'])) {
      unset($_SESSION['reset_pass']);
    }
    else {
      $given_hash = md5($edit['current_pass'] . strtolower($account->mail));
      if (!$edit['current_pass']) {
        form_set_error('current_pass', bts('Authentication is required when changing E-mail address or setting new password.', array(), NULL, 'boinc:account-credentials-change'));
      }
      elseif ( (!password_verify($given_hash, $boinc_user->passwd_hash)) and ($given_hash != $boinc_user->passwd_hash) ) {
        form_set_error('current_pass', bts('Password entered is not valid. Please verify that it is correct.', array(), NULL, 'boinc:account-credentials-change'));
      }

    }
  }

  // If an admin tries to change the email and NOT the password, show
  // error message. BOINC requires both to be changed together.
  if ($changing_email AND !$changing_pass AND user_access('administer users')) {
    form_set_error('pass', bts('If changing a user\'s email, you must also change the password simultaneously.', array(), NULL, 'boinc:account-credentials-change'));
  }

  // Expansion required to allow account key in place of passwd...?

}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Request new password form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Password request validation handler.
 */
function boincuser_request_pass_validate($form, &$form_state) {
  $edit = $form_state['values'];
  // The Drupal submit function expects a "name" field, even though
  // it contains an email address and we will not allow a user name
  if (!$edit['name']) form_set_error('name', bts('Please enter your email address', array(), NULL, 'boinc:forgot-password'));
  elseif (!valid_email_address($edit['name'])) form_set_error('name', bts('@email is not a well formed email address, please verify', array('@email' => $edit['name']), NULL, 'boinc:forgot-password'));
  else {
    // set email addrress to lower case
    $lower_email_addr = strtolower($edit['name']);

    // First look for an existing Drupal account
    if ($account = user_load_by_mail($lower_email_addr)) return;
    // Otherwise, check the BOINC db for this email address
    require_boinc('boinc_db');
    $boinc_user = BoincUser::lookup_email_addr($lower_email_addr);
    if ($boinc_user) {
      // If the user is in BOINC but not Drupal, bring them over...
      boincuser_register_make_drupal_user($boinc_user);
    } else {
      // If there is no existing BOINC user either, show an error
      form_set_error('name', bts('No account exists for @email -- please create an account using a BOINC client -- !instructions',
      array(
          '@email' => $edit['name'],
          '!instructions' => l(bts('Instructions', array(), NULL, 'boinc:forgot-password'), 'join')
      ), NULL, 'boinc:forgot-password'));
    }
  }
}

/**
 * The definition of the authenticator login form.
 */
function boincuser_authloginform() {
  $headers = apache_request_headers();
  $project_name = variable_get('site_name', 'Drupal-BOINC');
  $project_domain = $headers['Host'];
  $form['heading'] = array(
    '#type' => 'markup',
    '#value' => '<h3>' . bts("If you forgot your account's email address, or you can't receive email there:", array(), NULL, 'boinc:authenticator-login-page') . '</h3>'
  );
  $form['instructions'] = array(
    '#type' => 'markup',
    '#value' => '' .
      '<p>' . bts("If you have run BOINC under the account, you can still access it. Here's how:", array(), NULL, 'boinc:authenticator-login-page') .
      '  <ul>' .
      '    <li>' . bts('Go to the BOINC data directory on your computer (see !boinc_wiki for help finding this).', array('!boinc_wiki' => l(bts('BOINC documentation', array(), NULL, 'boinc:authenticator-login-page'), 'http://boinc.berkeley.edu/wiki/BOINC_Data_directory')), NULL, 'boinc:authenticator-login-page') . '</li>' .
      '    <li>' . bts('Find your account file for this project; it will have a name like %file (where the project URL is %url).', array('%file' => "account_{$project_domain}.xml", '%url' => "http://{$project_domain}"), NULL, 'boinc:authenticator-login-page') . '</li>' .
      '    <li>' . bts("Open the file in a text editor like Notepad. You'll see something like:", array(), NULL, 'boinc:authenticator-login-page') .
      '      <pre>' .
      '&lt;account&gt;' . "\n" .
      "  &lt;master_url&gt;http://{$project_domain}/&lt;/master_url&gt;" . "\n" .
      '  &lt;authenticator&gt;8b8496fdd26df7dc0423ecd43c09a56b&lt;/authenticator&gt;' . "\n" .
      "  &lt;project_name&gt;{$project_name}&lt;/project_name&gt;" . "\n" .
      '  ...' . "\n" .
      '&lt;/account&gt;' .
      '      </pre>' .
      '    </li>' .
      '    <li>' . bts('Select and Copy the string between &lt;authenticator&gt; and &lt;/authenticator&gt; (%auth in the above example).', array('%auth' => '8b8496fdd26df7dc0423ecd43c09a56b'), NULL, 'boinc:authenticator-login-page') . '</li>' .
      '    <li>' . bts('Paste the string into the field below, and click OK.', array(), NULL, 'boinc:authenticator-login-page') . '</li>' .
      '    <li>' . bts('You will now be logged in to your account; update the email and password of your account.', array(), NULL, 'boinc:authenticator-login-page') . '</li>' .
      '  </ul>' .
      '</p>'
  );
  $form['authenticator'] = array(
    '#title' => bts('Log in with authenticator', array(), NULL, 'boinc:authenticator-login-page'),
    '#type' => 'textfield',
    '#size' => 60,
    '#maxlength' => 32,
    '#required' => TRUE,
    '#description' => null
  );

  // Form control
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">',
    '#weight' => 1001,
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => bts('OK', array(), NULL, 'boinc:form-ok'),
    '#suffix' => '</li>',
    '#weight' => 1002,
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), 'user/password') . '</li>',
    '#weight' => 1003,
  );
  $form['form control tabs suffix'] = array(
    '#value' => '</ul>',
    '#weight' => 1004,
  );
  return $form;
}

/**
 * The authenticator login validation handler
 */
function boincuser_authloginform_validate($form, &$form_state) {
  $authenticator = $form_state['values']['authenticator'];
  if (strlen($authenticator) != 32) {
    // We notify the form API that this field has failed validation.
    form_set_error('authenticator', bts('That authenticator is not valid.', array(), NULL, 'boinc:authenticator-login-page'));
  } else {
    require_boinc('boinc_db');
    $boinc_user = BoincUser::lookup("authenticator='".addslashes($authenticator)."'");
    if (!$boinc_user) form_set_error('authenticator', bts('There is no account with that authenticator.', array(), NULL, 'boinc:authenticator-login-page'));
  }
}

/**
 * The authenticator login submit handler
 */
function boincuser_authloginform_submit($form, &$form_state) {
  global $user;
  $authenticator = $form_state['values']['authenticator'];
  require_boinc('boinc_db');
  $boinc_user = BoincUser::lookup("authenticator='".addslashes($authenticator)."'");
  if (!$user = user_load(get_drupal_id($boinc_user->id))) drupal_set_message(t('An unresolved error occurred while logging into this account.'));
  else $form_state['redirect'] = 'account/info/edit';
}


/**
 * The definition of the moderator reject user profile form.
 */
function boincuser_moderate_profile_reject_form(&$form_state, $uid) {
  $form_state['storage']['reject_profile_uid'] = $uid;
  $form['reason'] = array(
    '#title' => bts('Reason for rejecting this profile', array(), NULL, 'boinc:moderate-user'),
    '#type' => 'textarea',
    '#description' => bts('This reason will be included in an email to the user. Please write a brief explanation of the problem and how to fix it.', array(), NULL, 'boinc:moderate-user'),
    '#default_value' => '',
  );

  // Form control
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">',
    '#weight' => 1001,
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => bts('Submit', array(), NULL, 'boinc:form-submit'),
    '#suffix' => '</li>',
    '#weight' => 1002,
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), "account/{$uid}") . '</li>',
    '#weight' => 1003,
  );
  $form['form control tabs suffix'] = array(
    '#value' => '</ul>',
    '#weight' => 1004,
  );
  return $form;
}

/**
 * The moderator reject user profile submit handler
 */
function boincuser_moderate_profile_reject_form_submit($form, &$form_state) {
  $uid = $form_state['storage']['reject_profile_uid'];
  $reason = $form_state['values']['reason'];
  boincuser_moderate_profile_reject($uid, $reason);
}


/**
 * The definition of the ban user form.
 */
function boincuser_moderate_user_ban_form(&$form_state, $uid) {
  $form_state['storage']['ban_user_uid'] = $uid;
  $form['reason'] = array(
    '#title' => bts('Reason for banning this user', array(), NULL, 'boinc:moderate-user'),
    '#type' => 'textarea',
    '#description' => bts('This reason will be included in an email to the user. Please write a brief explanation of why the user is being banned.', array(), NULL, 'boinc:moderate-user'),
    '#default_value' => '',
  );
  $form['duration'] = array(
    '#title' => bts('Duration of the ban', array(), NULL, 'boinc:moderate-user'),
    '#type' => 'textfield',
    '#description' => bts('The number of days until the ban expires. Set to 0 to ban permanently.', array(), NULL, 'boinc:moderate-user'),
    '#default_value' => '',
  );

  // Form control
  $form['form control tabs prefix'] = array(
    '#value' => '<ul class="form-control tab-list">',
    '#weight' => 1001,
  );
  $form['submit'] = array(
    '#prefix' => '<li class="first tab">',
    '#type' => 'submit',
    '#value' => bts('Submit', array(), NULL, 'boinc:form-submit'),
    '#suffix' => '</li>',
    '#weight' => 1002,
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), "account/{$uid}") . '</li>',
    '#weight' => 1003,
  );
  $form['form control tabs suffix'] = array(
    '#value' => '</ul>',
    '#weight' => 1004,
  );
  return $form;
}

/**
 * The ban user submit handler
 */
function boincuser_moderate_user_ban_form_submit($form, &$form_state) {
  $uid = $form_state['storage']['ban_user_uid'];
  $reason = $form_state['values']['reason'];
  $duration = $form_state['values']['duration'];
  if ($duration) $duration = $duration * 24*60*60;
  boincuser_moderate_user_ban($uid, $reason, $duration);
}


/**
 * Hack to fix submission of the flag friend unfriend form
 */
function boincuser_fix_unfriend_form_submit($form, &$form_state) {
  // Leaving action as "unfriend" causes problems
  if ($form_state['values']['action'] == 'unfriend') {
    $form_state['values']['action'] = 'unflag';
  }
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * The terms of use form
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

function boincuser_termsofuse_form() {
  global $user;

  // If user has already signed terms of use, and got to this form in error, send them to site home.
  if (boincuser_check_termsofuse($user)) {
    drupal_goto();
  }

  drupal_set_message( bts('WARNING: You have not agreed to our terms of use. Please agree to the terms of use before continuing.', array(), NULL, 'boinc:termsofuse-form'), 'warning' );

  $form = array();
  drupal_add_js(drupal_get_path('module', 'boincuser') . '/boincuser.js');
  $termsofuse = variable_get('boinc_weboptions_termsofuse', '');

  // Fieldset to hold all of the form as a container
  $form['termsofuse'] = array(
    '#type'   => 'fieldset',
    '#prefix' => '<div id="termsofuse-wrapper">', // This is our wrapper div.
    '#suffix' => '</div>',
    '#tree'   => TRUE,
  );

  $form['termsofuse']['title1'] = array(
    '#weight' => -12,
    '#value' => '<h2>' . bts( variable_get('boinc_weboptions_registrationtitle', 'Please read and acknowledge our terms of use'), array(), NULL, 'project:termsofuse-form' ) . '</h2>',
    '#prefix' => '<div id="register-title1">',
    '#suffix' => '</div>',
  );

  // Terms of use section
  $form['termsofuse']['body'] = array(
    '#weight' => -10,
    '#value' => bts($termsofuse, array(), NULL, 'project:termsofuse-form'),
    '#prefix' => '<div id="register-termsofuse">',
    '#suffix' => '</div>',
  );

  $form['termsofuse']['agreeTOU'] = array(
    '#type'   => 'checkbox',
    '#title'  => bts(variable_get('boinc_weboptions_agreequestion', 'Do you agree with the above terms of use?'), array(), NULL, 'project:termsofuse-form'),
    '#weight' => -8,
    '#prefix' => '<div id="register-checkbox">',
    '#suffix' => '</div>',
  );

  $form['termsofuse']['spacer'] = array(
    '#prefix' => '<div class="clearfix" id="register-title2">',
    '#value'  => '&nbsp;',
    '#suffix' => '</div>',
  );

  // Form Control
  $form['submit'] = array(
    '#prefix' => '<p><p><p><li class="first tab" id="register-submit">',
    '#type' => 'submit',
    '#value' => bts('Yes', array(), NULL, 'boinc:form-submit'),
    '#suffix' => '</li>',
  );
  $form['form control tabs'] = array(
    '#value' => '<li class="tab">' . l(bts('NO - LOGOUT', array(), NULL, 'boinc:form-cancel'), '/logout') . '</li>',
  );
  if (module_exists('boincuser_delete')) {
    $deletelink = '/user/' . $user->uid . '/delete';
    $form['deleteaccount'] = array(
      '#value' => '<li class="tab">' . l(bts('NO - DELETE ACCOUNT', array(), NULL, 'boinc:form-delete-user'), $deletelink) . '</li>',
    );
  }

  // Set form redirect
  $form['#redirect'] = $_REQUEST['destination'];

  // Add the current user's data into the form
  $form['#account'] = $user;

  return $form;
}

function boincuser_termsofuse_form_validate($form, &$form_state) {
  // Check TOU agreement
  if (!$form_state['values']['termsofuse']['agreeTOU']) {
    form_set_error('termsofuse', bts('ERROR: You must acknowledge our terms of use by clicking the checkbox before registering for an account.', array(), NULL, 'boinc:termsofuse-form'));
  }
}

function boincuser_termsofuse_form_submit($form, &$form_state) {
  $user = $form['#account'];
  if (!boincuser_consentto_termsofuse($user)) {
    form_set_error('termsofuse',
      bts('There was an error in agreeing to the terms of use. The @project administrators have been notified.',
        array('@project' => variable_get('site_name', 'Drupal-BOINC')), NULL, 'boinc:termsofuse-form')
    );
  }

  // Delete session messages
  if ($_SESSION['messages']['warning']) {
    unset($_SESSION['messages']['warning']);
  }
}

/**
 * Revert email to previous email address. The user must supply a
 * token given via e-mail. The token is active for 7 days, and is
 * removed after used.
 */
function boincuser_revertemail(&$form_state, $token) {
  require_boinc('token');
  require_boinc('util');

  global $user;
  $form = array();

  // drupal JS for dynamic password validation
  _user_password_dynamic_validation();

  // check BOINC user exists
  $account = user_load(array('uid' => $user->uid));
  $uid = $user->uid;
  $boincid = $account->boincuser_id;
  // check $token is valid
  if (!is_valid_token($boincid, $token, 'E')) {
    drupal_set_message(bts('ERROR: You have supplied an incorrect (most likely expired) token. Please obtain a new token by !link your email address.',
    array(
      '!link' => l(bts('changing', array(), NULL, 'boinc:revert-email-change'), "/account/info/edit"),
    ),
    NULL, 'boinc:revert-email-change'), 'error');
    drupal_goto();
  }

  // Attach account and token to this form.
  $form['_account'] = array('#type' => 'value', '#value' => $account);
  $form['_token'] = array('#type' => 'value', '#value' => $token);

  // Instructions
  $form['main']['instructions1'] = array(
    '#value' => '<p>'.
    bts('In order to change your email back to your previous email address, <strong>!prev_email</strong>, you must also change your password.',
      array(
        '!prev_email' => $account->boincuser_previous_email_addr,
      ),
      NULL, 'boinc:revert-email-change').
    '</p>',
  );

  $form['main']['pass'] = array(
    '#type' => 'password_confirm',
    '#description' => 'Enter a new password in both fields',
    '#size' => 17,
  );

  // Wrap action buttons for styling consistency
  $form['buttons']['form control tabs prefix'] = array(
      '#value' => '<ul class="form-control tab-list">',
      '#weight' => 1001,
  );
  $form['buttons']['submit']['#type'] = 'submit';
  $form['buttons']['submit']['#prefix'] = '<li class="first tab">';
  $form['buttons']['submit']['#value'] = bts('Submit', array(), NULL, 'boinc:form-submit');
  $form['buttons']['submit']['#suffix'] = '</li>';
  $form['buttons']['submit']['#weight'] = 1002;
  $form['buttons']['cancel'] = array(
      '#value' => '<li class="tab">' . l(bts('Cancel', array(), NULL, 'boinc:form-cancel'), 'user/info/edit') . '</li>',
      '#weight' => 1005,
  );
  $form['buttons']['form control tabs suffix'] = array(
      '#value' => '</ul>',
      '#weight' => 1010,
  );

  return $form;
}

/**
 * Validation handler for revertemail form
 */
function boincuser_revertemail_validate($form, &$form_state) {
  // Load account and boincuser
  $account = $form_state['values']['_account'];
  $boinc_user = BoincUser::lookup_id_nocache($account->boincuser_id);

  if (BoincUser::lookup_email_addr($boinc_user->previous_email_addr)) {
    rules_invoke_event('boincuser_revert_email_error', $boinc_user->previous_email_addr, variable_get('boinc_admin_mailing_list_subject_tag', ''));
    form_set_error('mail',
      bts('An account already exists for @email. Previous email address cannot be used because another account is using it as their email address. The @project administrators have been notified.',
        array(
          '@email' => $boinc_user->previous_email_addr,
          '@project' => variable_get('site_name', 'Drupal-BOINC'),
        ), NULL, 'boinc:add-new-user'));
  }

}

/**
 * Submit handler for revertemail form
 */
function boincuser_revertemail_submit($form, &$form_state) {
  require_boinc('password_compat/password');

  // Load account and boincuser
  $account = $form_state['values']['_account'];
  $boinc_user = BoincUser::lookup_id_nocache($account->boincuser_id);

  $pem = strtolower($boinc_user->previous_email_addr);

  // Set new password based on previous email address and entered
  // password.
  $new_passwd_hash = password_hash( md5($form_state['values']['pass'].$pem), PASSWORD_DEFAULT);

  $boinc_user->update("email_addr='${pem}', previous_email_addr='', email_addr_change_time=0, passwd_hash='${new_passwd_hash}'");

  // Set email in drupal database to previous email
  user_save($account, array('mail' => $pem));

  // delete the token
  $result = delete_token($account->boincuser_id, $form_state['values']['_token'], 'E');

  drupal_goto('account');
}
