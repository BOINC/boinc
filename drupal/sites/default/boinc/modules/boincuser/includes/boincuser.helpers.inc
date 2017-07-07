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
    drupal_set_message(bts(
      'You are banned from community participation until @date',
      array('@date' => format_date($account->boincuser_penalty_expiration))
    ), 'warning', FALSE);
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
    drupal_set_message(bts(
      'You must earn @count more credits to be able to post comments on this site.',
      array('@count' => $min_credit_to_post - $account->boincuser_total_credit)
    ), 'warning', FALSE);
    if (isset($account->roles[$unrestricted_role])) {
      // Either the threshold has been raised or credits have been revoked;
      // this user no longer qualifies for full privileges
      unset($account->roles[$unrestricted_role]);
      user_save($account, array('roles' => $account->roles));
    }
  }
}

/**
 * Get the cid of the first comment the user has not seen on a given node
 */
function boincuser_get_first_unread_comment_id($nid, $uid = NULL) {
  if (!$uid) {
    global $user;
    $uid = $user->uid;
  }
  return db_result(db_query("
    SELECT c.cid
    FROM {node} n
    INNER JOIN {comments} c ON c.nid = n.nid
    LEFT JOIN {history} h ON n.nid = h.nid AND h.uid = %d
    WHERE n.nid = %d
    AND n.status = 1
    AND c.timestamp > h.timestamp
    ORDER BY c.timestamp ASC
    LIMIT 1",
    $uid, $nid
  ));
}

/**
 * Choose and set the user of the day
 */
function boincuser_select_user_of_the_day() {
  // First get a list of users with recent credit
  db_set_active('boinc');
  $users_with_credit = db_query("
    SELECT
      id
    FROM {user} u
    JOIN {profile} p ON p.userid = u.id
    WHERE expavg_credit > 1
    ORDER BY uotd_time ASC, RAND()"
  );
  db_set_active('default');
  $active_users = array();
  while ($user_with_credit = db_fetch_object($users_with_credit)) {
    $active_users[] = $user_with_credit->id;
  }
  $active_users = implode(',', $active_users);
  // Limit to users who have never been user of the day, if there are any
  $never_been_picked = db_result(db_query("
    SELECT COUNT(*)
    FROM {boincuser} bu
    WHERE bu.uotd_time = 0
    " . ($active_users ? " AND bu.boinc_id IN ({$active_users}) " : '')
  ));
  $new_uotd_uid = 0;
  while (!$new_uotd_uid) {
    // Select a user of the day randomly from the pool
      $new_uotd_uid = db_result(db_query("
      SELECT
        n.uid
      FROM {node} n
      JOIN {boincuser} bu ON bu.uid = n.uid
      WHERE n.type = 'profile'
      AND n.status = 1
      AND n.moderate = 0
      " . ($never_been_picked ? " AND bu.uotd_time = 0 " : '') . "
      " . ($active_users ? " AND bu.boinc_id IN ({$active_users}) " : '') . "
      ORDER BY RAND()
      LIMIT 1"
    ));
    if (!$new_uotd_uid) {
      // Can't find a user with a profile; remove constraints on the pool
      if ($never_been_picked) {
        // Allow users who have been previously selected
        $never_been_picked = FALSE;
      }
      elseif ($active_users) {
        // Allow users who are not even active (getting desperate)
        $active_users = FALSE;
      }
      else {
        // Process failed...
        return FALSE;
      }
    }
  }
  $uotd = user_load($new_uotd_uid);
  if ($uotd->uid) {
    db_query("
      UPDATE {boincuser}
      SET uotd_time = '%d'
      WHERE uid = '%d'",
      time(), $uotd->uid
    );
  }
  return $uotd;
}

/**
 * Determine the first unique name from a given base. Also cleans the
 * username so that it will be valid for drupal. And truncates user
 * name to 50 characters.
 *
 */
function create_proper_drupalname($requested_name) {
  if (!$requested_name) {
    // If the name is empty, set it
    $requested_name = 'anonymous';
  }
  $same_name_tally = 1;

  // Remove extra spaces
  $name2 = preg_replace("/ {2,}/", " ", trim($requested_name));
  // Remove any non-valid characters
  $cleaned_name = preg_replace('/[^a-zA-Z0-9_ \.-]/s', '_', $name2);
  // Truncate name
  $name_length = strlen($cleaned_name);
  if ($name_length > 56) {
    // Name is limited to 60 characters, but we want to leave space to add a
    // tally if needed (for users with duplicate names); Limit to 56 chars and
    // replace the middle of the string with "..." if too long
    $cleaned_name = substr_replace($cleaned_name, '...', 28, ($name_length-56)+3);
  }

  // Determine if there are duplicate names, if so append a number to end.
  $unique_name = $cleaned_name;
  while (db_result(db_query("SELECT uid FROM {users} WHERE name = '{$unique_name}' LIMIT 1"))) {
    $same_name_tally++;
    $unique_name = "{$cleaned_name}_{$same_name_tally}";
  }
  return $unique_name;
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
 * The path from the BOINC html root must be included (e.g. user/file.php)
 */
function include_boinc($file) {
  if ($include_dir = boinc_get_path()) {
    $include_dir .= '/html';
    $workingDir = getcwd();
    $path = dirname($file);
    $file = basename($file);
    chdir("{$include_dir}/{$path}");
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
  return '<h2 class="pane-title">' . bts('Friends (@count)', 
    array('@count' => $friend_count)) . '</h2>';
}
