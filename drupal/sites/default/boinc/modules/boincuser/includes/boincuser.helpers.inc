<?php
// $Id$

/**
 * Functions that are shared amongst files and dependent modules go
 * here to keep the clutter down in the main module file.
 */ 

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Drupal 7 forward compatibility functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

if (!function_exists('user_load_by_mail')) {
  /**
   * user_load_by_mail will be broken out of user_load
   */
  function user_load_by_mail($mail) {
    return user_load(array('mail' => $mail));
  }
}
 

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * General utility functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Get an image object from a given file and cck field
 */ 
function get_cck_image_object($image_path, $field_name, $content_type, $ignore_resolution = FALSE) {
  $field = content_fields($field_name, $content_type); 
  $validators = array_merge(filefield_widget_upload_validators($field), imagefield_widget_upload_validators($field));
  if ($ignore_resolution) {
    unset($validators['filefield_validate_image_resolution']);
  }
  $target_path = filefield_widget_file_path($field);
  // Create the image object
  return field_file_save_file($image_path, $validators, $target_path, FILE_EXISTS_REPLACE);
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * General user functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * 
 */
function boincuser_check_credit_requirements() {
  global $user;
  if (!$user->uid) {
    return FALSE;
  }
  $account = user_load($user->uid);
  $min_credit_to_post = variable_get('boinc_comment_min_credit', 0);
  $community_role = array_search('community member', user_roles(true));
  $unrestricted_role = array_search('verified contributor', user_roles(true));
  
  // Set user roles based on current penalty status and total credit
  if ($account->boincuser_penalty_expiration > time()) {
    if (isset($account->roles[$community_role])) {
      // Remove from the community role, if not already
      unset($account->roles[$community_role]);
    }
    if (isset($account->roles[$unrestricted_role])) {
      // Likewise, revoke extra privileges
      unset($account->roles[$unrestricted_role]);
    }
    user_save($account, array('roles' => $account->roles));
  }
  elseif ($account->boincuser_total_credit >= $min_credit_to_post) {
    if (!isset($account->roles[$unrestricted_role])) {
      // This user is now above the credit threshold and is allowed full
      // privileges
      $account->roles[$unrestricted_role] = 'verified contributor';
      user_save($account, array('roles' => $account->roles));
    }
  }
  else {
    drupal_set_message(t('You must earn @more_credit to be able to post
      comments on this site.', array('@more_credit' => format_plural(
        $min_credit_to_post - $account->boincuser_total_credit,
        '1 more credit', '@count more credits'
      ))
    ), 'warning', FALSE);
    if (isset($account->roles[$unrestricted_role])) {
      // Either the threshold has been raised or credits have been revoked;
      // this user no longer qualifies for full privileges
      unset($account->roles[$unrestricted_role]);
      user_save($account, array('roles' => $account->roles));
    }
  }
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * General BOINC functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Require BOINC library
 */
function require_boinc($libraries) {
  if ($include_dir = boinc_get_path('html_inc')) {
    $working_dir = getcwd();
    chdir($include_dir);
    if (!is_array($libraries)) {
      $libraries = array($libraries);
    }
    foreach ($libraries as $library) {
      require_once("{$library}.inc");
    }
    chdir($working_dir);
  }
}

/**
 * Include BOINC code
 * The path from the BOINC root must be included (e.g. user/file.php)
 */
function include_boinc($file) {
  if ($include_dir = boinc_get_path()) {
    $workingDir = getcwd();
    chdir($include_dir);
    include($file);
    chdir($workingDir);
  }
}

/**
 * Get the BOINC include path
 */
function boinc_get_path($type = 'root') {
  $dir = "boinc_{$type}_dir";
  if ($include_dir = variable_get("boinc_{$type}_dir", '')) {
    return $include_dir;
  }
  else {
    // Don't show errors on blacklisted pages
    $page_blacklist = array(
      'admin/boinc/environment'
    );
    if (!in_array($_GET['q'], $page_blacklist)) {
      watchdog('boincuser', 'The BOINC environment is not configured. Please
          !configure_it', array('!configure_it' => l(t('configure it now'),
            'admin/boinc/environment')), WATCHDOG_WARNING);
      if (user_access('administer site configuration')) {
        drupal_set_message(t('The BOINC environment is not configured. Please
          !configure_it', array('!configure_it' => l(t('configure it now'),
            'admin/boinc/environment'))), 'warning', FALSE);
      }
      else {
        drupal_set_message(t('There is a problem with the site. Please contact
          the system administrator.'), 'error', FALSE);
      }
      // Redirect home to display the error message and avoid fatal errors
      // (unless on a blacklisted page)
      $redirect_blacklist = array(
        'admin/settings/performance',
        'admin/boinc/environment',
        'home',
        ''
      );
      if (!in_array($_GET['q'], $redirect_blacklist)) {
        drupal_goto('');
      }
    }
    else {
      // Clear the messages on the environment config page
      drupal_get_messages();
    }
  }
  return FALSE;
}

/**
 * Get the configured scheduler tags
 */
function boinc_get_scheduler_tags() {
  // Don't generate messages for blacklisted pages
  $page_blacklist = array(
    'admin/boinc/environment',
    'admin/boinc/scheduler'
  );
  if ($url_config = variable_get('boinc_scheduler_urls', '')) {
    return explode("\r\n", $url_config);
  }
  elseif (!in_array($_GET['q'], $page_blacklist)) {
    watchdog('boincuser', 'The BOINC scheduling server settings are not yet
        configured. Please !verify for the settings to become effective.',
          array('!verify' => l(t('verify the default values') . ' <strong>' . 
            t('and') . '</strong> ' . t('save the configuration'),
            'admin/boinc/scheduler', array('html' => TRUE))
          ), WATCHDOG_WARNING);
    if (user_access('administer site configuration')) {
      drupal_set_message(t('The BOINC scheduling server settings are not yet
          configured. Please !verify for the settings to become effective',
            array('!verify' => l(t('verify the default values') . ' <strong>' . 
              t('and') . '</strong> ' . t('save the configuration'),
              'admin/boinc/scheduler', array('html' => TRUE))
            )), 'warning', FALSE);
    }
  }
  return array();
}

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * BOINC user account functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Test if logged in user
 * Determine if a BOINC ID matches the logged in user
 */
function is_current_boinc_user($boinc_id) {
  global $user;
  if (!$user->uid) {
    return FALSE;
  }
  // boincuser_id is not stored in the global user, so load a new instance
  $drupuser = user_load($user->uid);
  return ($boinc_id == $drupuser->boincuser_id);
}

/**
 * Convert a BOINC ID to a Drupal ID
 */
function boincuser_lookup_uid($boinc_id) {
  $drupal_id = db_result(db_query("SELECT uid FROM {boincuser} WHERE boinc_id='%d'", $boinc_id));
  return $drupal_id;
}
function get_drupal_id($boinc_id) {
  return boincuser_lookup_uid($boinc_id);
}

/**
 * Get a BOINC user object
 */
function boincuser_load($user_id = NULL, $is_drupal_id = FALSE) {
  if (!$user_id) {
    global $user;
    $user_id = $user->uid;
    $is_drupal_id = TRUE;
  }
  if ($is_drupal_id) {
    $account = user_load($user_id);
    $user_id = $account->boincuser_id;
  }
  require_boinc('boinc_db');
  return BoincUser::lookup_id($user_id);
}


/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * BOINC function wrappers
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Wrapper for boinc_version() function
 */
function get_boinc_version($x) {
  require_boinc('host');
  return function_exists('boinc_version') ? boinc_version($x) : 'err!';
}
  

/*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *
 * Functions for use in displaying special case text in views
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
  * Generate the friend block header
  */
function boincuser_views_friends_block_header($context = null) {
  // Get the friend count for the user being viewed
  $view = views_get_current_view();
  $account_id = $view->args[0];
  $friend_count = flag_friend_get_friend_count($account_id);
  return '<h2 class="pane-title">' . t('Friends (@friend_count)', 
    array('@friend_count' => $friend_count)) . '</h2>';
}
