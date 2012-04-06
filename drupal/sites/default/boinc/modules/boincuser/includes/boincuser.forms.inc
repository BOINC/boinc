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
  // Call our custom authentication function to check for an existing BOINC user
  if (!boincuser_login_authenticate($form_state['values'])) {
    // Authentication failed; set an error accordingly
    form_set_error('name', t('Sorry, unrecognized email address or password. <a href="@password">Have you forgotten your password?</a>', array('@password' => url('user/password'))));
  }
}

/**
 * Custom authentication function to check BOINC account information
 */
function boincuser_login_authenticate($form_values) {
  global $boincuser_authenticated;
  $email_addr = $form_values['email'];       
  $passwd = $form_values['pass'];
  $passwd_hash = md5($passwd.$email_addr); 
  
  // Include BOINC user library
  require_boinc('user');
  
  // Get the BOINC user and check credentials
  $boinc_user = lookup_user_email_addr($email_addr);
  if (!$boinc_user) return false;
  if ($passwd_hash != $boinc_user->passwd_hash) return false;
  
  // BOINC authentication successful; log in and synchronize accounts
  boincuser_login_register($boinc_user);
  // Write session, update timestamp, run user 'login' hook.
  user_authenticate_finalize($form_state['values']);
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
function boincuser_register_validate($form_values) {            
  // Include BOINC user library
  require_boinc('user');
  // Check for an existing BOINC user
  $boinc_user = lookup_user_email_addr($form_values['mail']);
  if ($boinc_user) return false;
  return true;
}

/**
 * Create BOINC account
 */
function boincuser_register_make_user($params) {    
  // Include BOINC user library
  require_boinc('user');
  // Create the BOINC user
  $boinc_user = make_user($params['email_addr'], $params['name'], $params['passwd_hash'], $params['country'], $params['postal_code']);
  return $boinc_user;
}

/**
 * Create Drupal account from existing BOINC account
 */
function boincuser_register_make_drupal_user($boinc_user) {
  if (is_numeric($boinc_user)) {
    require_boinc('db');
    $boinc_user = BoincUser::lookup_id($boinc_user);
  }
  $account = boincuser_create_drupal_user($boinc_user);
  boincuser_create_drupal_profile($account, $boinc_user);
  return $account;
}

function boincuser_create_drupal_user($boinc_user) {
  require_boinc('forum_db');
  BoincForumPrefs::lookup($boinc_user);
  $account = null;
  $module = 'boincuser';
  
  // Make sure name is unique
  $unique_name = $boinc_user->name;
  $same_name_tally = 1;
  while (db_result(db_query("SELECT uid FROM users WHERE name = '{$unique_name}' LIMIT 1"))) {
    $same_name_tally++;
    $unique_name = "{$boinc_user->name} {$same_name_tally}";
  }
  
  // Email is limited to 64 characters...
  $boinc_email = substr($boinc_user->email_addr, 0, 64);
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
  
  // Mark account as imported so a BOINC account is not created on Insert (i.e. user_save)
  $_SESSION['importedUser'] = true;
  $account = user_save('', $userinfo);
  
  // Drupal overrides the original registration date, so update to the correct date
  $account = user_save($account, array('created' => $userinfo['created']));
  
  // Terminate if an error occured during user_save().
  if (!$account) {
    drupal_set_message(t("Error saving user account."), 'error');
    return null;
  }
  
  // Cross reference the Drupal and BOINC accounts
  boincuser_account_cross_reference($account->uid, $boinc_user->id);
  
  return $account;
}

function boincuser_create_drupal_profile($account, $boinc_user) {
  
  // Automatically create a content profile
  $now = time();
  $profile_background = null;
  $profile_opinions = null;
  $profile_image = null;
  
  // Load the BOINC profile, if available
  if ($boinc_user->has_profile) {
    $boinc_profile = BoincProfile::lookup("userid={$boinc_user->id}");
    $profile_background = $boinc_profile->response1;
    $profile_opinions = $boinc_profile->response2;
    if ($boinc_profile->has_picture) {
      // Load picture; load validators; determine where to store it in Drupal
      $profile_image_path = drupal_get_path('module', 'boincuser') . '/includes/boinclib/'.IMAGE_PATH.$boinc_user->id.'.jpg';
      $field = content_fields('field_image', 'profile'); 
      $validators = array_merge(filefield_widget_upload_validators($field), imagefield_widget_upload_validators($field));
      $target_path = filefield_widget_file_path($field);
      // Create the image object
      $profile_image = field_file_save_file($profile_image_path, $validators, $target_path, FILE_EXISTS_REPLACE);
    }
  }
  
  // Populate the profile object
  $profile_node = new stdClass();
  $profile_node->title = $boinc_user->name;
  $profile_node->body = '';
  $profile_node->type = 'profile'; 
  $profile_node->created = $now;
  $profile_node->changed = $now;
  $profile_node->status = 1;
  $profile_node->promote = 0;
  $profile_node->sticky = 0;
  $profile_node->format = 1;       // Filtered HTML
  $profile_node->uid = $account->uid;
  $profile_node->field_display_name[]['value'] = $boinc_user->name;
  $profile_node->field_country[]['value'] = $boinc_user->country;
  $profile_node->field_zip[]['value'] = $boinc_user->postal_code;
  $profile_node->field_url[]['value'] = $boinc_user->url;
  $profile_node->field_background[]['value'] = $profile_background;
  $profile_node->field_opinions[]['value'] = $profile_opinions;
  $profile_node->field_image[] = $profile_image;
  
  // Save the profile object to the database
  $profile_node = node_submit($profile_node);
  node_save($profile_node);
  
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

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Account form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * User account update validation handler.
 */
function boincuser_account_validate($form_values, $account, &$error) {            
  // Include BOINC user library
  require_boinc('user');
  if ($form_values['mail'] AND $form_values['mail'] != $account->mail) {
    // E-mail address is set to change; check for an existing BOINC user
    $boinc_user = lookup_user_email_addr($form_values['mail']);
    if ($boinc_user) $error = 101;
    else {
      // Ensure that a password was entered
      if (!$form_values['pass']['pass1']) $error = 102;
    }
  }
  if ($error) return false;
  return true;
  
  // Expansion required to allow account key in place of passwd...?
  
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Request new password form handlers and functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Password request validation handler.
 */
function boincuser_request_pass_validate($form, &$form_state) {
  $form_values = $form_state['values'];
  // The Drupal submit function expects a "name" field, even though
  // it contains an email address and we will not allow a user name
  if (!$form_values['name']) form_set_error('name', t('Please enter your email address'));
  elseif (!valid_email_address($form_values['name'])) form_set_error('name', t('@email is not a well formed email address, please verify', array('@email' => $form_values['name'])));
  else {
    // First look for an existing Drupal account
    if ($account = user_load_by_mail($form_values['name'])) return;
    // Otherwise, check the BOINC db for this email address
    require_boinc('db');
    $boinc_user = lookup_user_email_addr($form_values['name']);
    if ($boinc_user) {
      // If the user is in BOINC but not Drupal, bring them over...
      boincuser_register_make_drupal_user($boinc_user);
    } else {
      // If there is no existing BOINC user either, show an error
      form_set_error('name', t('No account exists for @email -- select &quot;Create new account&quot; to register', array('@email' => $form_values['name'])));
    }
  }
}

/**
 * The definition of the authenticator login form.
 */
function boincuser_authloginform() {
  $form['heading'] = array(
    '#type' => 'markup',
    '#value' => '<h3>' . t('2) If you forgot your account&apos;s email address, or you can&apos;t receive email there:') . '</h3>'
  );
  $form['instructions'] = array(
    '#type' => 'markup',
    '#value' => '' .
      '<p>' . t('If you have run BOINC under the account, you can still access it. Here&apos;s how:') .
      '  <ul>' .
      '    <li>' . t('Go to the BOINC data directory on your computer (on Windows this is usually @path1 or @path2.', array('@path1' => '<b>C:\Documents and Settings\All Users\Application Data\BOINC</b>', '@path2' => '<b>C:\Program Files\BOINC</b>')) . '</li>' .
      '    <li>' . t('Find your account file for this project; it will have a name like @file (where the project URL is @url).', array('@file' => '<b>account_lhcathome.cern.ch.xml</b>', '@url' => '<b>http://lhcathome.cern.ch</b>')) . '</li>' .
      '    <li>' . t('Open the file in a text editor like Notepad. You&apos;ll see something like') .
      '      <pre>' .
      '&lt;account&gt;' . "\n" .
      '  &lt;master_url&gt;http://lhcathome.cern.ch/&lt;/master_url&gt;' . "\n" .
      '  &lt;authenticator&gt;8b8496fdd26df7dc0423ecd43c09a56b&lt;/authenticator&gt;' . "\n" .
      '  &lt;project_name&gt;lhcathome&lt;/project_name&gt;' . "\n" .
      '  ...' . "\n" .
      '&lt;/account&gt;' .
      '      </pre>' .
      '    </li>' .
      '    <li>' . t('Select and Copy the string between <authenticator> and </authenticator> (@auth in the above example).', array('@auth' => '<b>8b8496fdd26df7dc0423ecd43c09a56b</b>')) . '</li>' .
      '    <li>' . t('Paste the string into the field below, and click OK.') . '</li>' .
      '    <li>' . t('You will now be logged in to your account; update the email and password of your account.') . '</li>' .
      '  </ul>' .
      '</p>'
  );
  $form['authenticator'] = array(
    '#title' => t('Log in with authenticator'),
    '#type' => 'textfield',
    '#size' => 60,
    '#maxlength' => 32,
    '#required' => TRUE,
    '#description' => null
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('OK')
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
    form_set_error('authenticator', t('That authenticator is not valid.'));
  } else {
    require_boinc('db');
    $boinc_user = lookup_user_auth($authenticator);
    if (!$boinc_user) form_set_error('authenticator', t('There is no account with that authenticator.'));
  }
}

/**
 * The authenticator login submit handler
 */
function boincuser_authloginform_submit($form, &$form_state) {
  global $user;
  require_boinc('db');
  $boinc_user = lookup_user_auth($form_state['values']['authenticator']);
  if (!$user = user_load(get_drupal_id($boinc_user->id))) drupal_set_message(t('An unresolved error occurred while logging into this account.'));
  else $form_state['redirect'] = "user/{$user->uid}/edit";
}
