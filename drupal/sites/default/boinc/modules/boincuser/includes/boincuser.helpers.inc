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
 * General BOINC functions
 *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  */

/**
 * Require BOINC library
 */
function require_boinc($libraries) {
  $workingDir = getcwd();
  chdir('./' . drupal_get_path('module', 'boincuser') . '/includes/boinclib');
  if (!is_array($libraries)) $libraries = array($libraries);
  foreach ($libraries as $library) require_once("{$library}.inc");
  chdir($workingDir);
}

/**
 * Include BOINC code
 * The path from the BOINC root must be included (e.g. user/file.php)
 */
function include_boinc($file) {
  $workingDir = getcwd();
  chdir('./' . drupal_get_path('module', 'boincuser') . '/includes/boinclib');
  include("../{$file}");
  chdir($workingDir);
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
  // boincuser_id is not stored in the global user, so load a new instance
  $drupuser = user_load($user->uid);
  return ($boinc_id == $drupuser->boincuser_id);
}

/**
 * Convert a BOINC ID to a Drupal ID
 */
function get_drupal_id($boinc_id) {
  $drupal_id = db_result(db_query("SELECT uid FROM {boincuser} WHERE boinc_id='%d'", $boinc_id));
  return $drupal_id;
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
