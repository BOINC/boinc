<?php
// $Id: field_file.inc,v 1.37 2010/06/29 19:14:16 quicksketch Exp $

/**
 * @file
 * Common functionality for file handling CCK field modules.
 */

/**
 * Load a file from the database.
 *
 * @param $fid
 *   A numeric file id or string containing the file path.
 * @param $reset
 *   Whether to reset the internal file_load cache.
 * @return
 *   A file array.
 */
function field_file_load($fid, $reset = NULL) {
  // Reset internal cache.
  if (isset($reset)) {
    _field_file_cache(NULL, TRUE);
  }

  if (empty($fid)) {
    return array('fid' => 0, 'filepath' => '', 'filename' => '', 'filemime' => '', 'filesize' => 0);
  }

  $files = _field_file_cache();

  // Serve file from internal cache if available.
  if (empty($files[$fid])) {
    if (is_numeric($fid)) {
      $file = db_fetch_object(db_query('SELECT f.* FROM {files} f WHERE f.fid = %d', $fid));
    }
    else {
      $file = db_fetch_object(db_query("SELECT f.* FROM {files} f WHERE f.filepath = '%s'", $fid));
    }

    if (!$file) {
      $file = (object) array('fid' => 0, 'filepath' => '', 'filename' => '', 'filemime' => '', 'filesize' => 0);
    }

    foreach (module_implements('file_load') as $module) {
      $function = $module .'_file_load';
      $function($file);
    }

    // Cache the fully loaded file for later use.
    $files = _field_file_cache($file);
  }

  // Cast to an array for the field storage.
  // Contrary to fields, hook_file() and core file functions expect objects.
  return isset($files[$fid]) ? (array) $files[$fid] : FALSE;
}

/**
 * Save a file upload to a new location.
 * The source file is validated as a proper upload and handled as such. By
 * implementing hook_file($op = 'insert'), modules are able to act on the file
 * upload and to add their own properties to the file.
 *
 * The file will be added to the files table as a temporary file. Temporary
 * files are periodically cleaned. To make the file permanent file call
 * file_set_status() to change its status.
 *
 * @param $source
 *   A string specifying the name of the upload field to save.
 * @param $validators
 *   An optional, associative array of callback functions used to validate the
 *   file. The keys are function names and the values arrays of callback
 *   parameters which will be passed in after the user and file objects. The
 *   functions should return an array of error messages, an empty array
 *   indicates that the file passed validation. The functions will be called in
 *   the order specified.
 * @param $dest
 *   A string containing the directory $source should be copied to. If this is
 *   not provided or is not writable, the temporary directory will be used.
 * @return
 *   An array containing the file information, or 0 in the event of an error.
 */
function field_file_save_upload($source, $validators = array(), $dest = FALSE) {
  if (!$file = file_save_upload($source, $validators, $dest, FILE_EXISTS_RENAME)) {
    return 0;
  }
  if (!@chmod($file->filepath, 0664)) {
    watchdog('filefield', 'Could not set permissions on destination file: %file', array('%file' => $file->filepath));
  }

  // Let modules add additional properties to the yet barebone file object.
  foreach (module_implements('file_insert') as $module) {
    $function =  $module .'_file_insert';
    $function($file);
  }
  _field_file_cache($file); // cache the file in order to minimize load queries
  return (array)$file;
}

/**
 * Save a file into a file node after running all the associated validators.
 *
 * This function is usually used to move a file from the temporary file
 * directory to a permanent location. It may be used by import scripts or other
 * modules that want to save an existing file into the database.
 *
 * @param $filepath
 *   The local file path of the file to be saved.
 * @param $validators
 *   An optional, associative array of callback functions used to validate the
 *   file. The keys are function names and the values arrays of callback
 *   parameters which will be passed in after the user and file objects. The
 *   functions should return an array of error messages, an empty array
 *   indicates that the file passed validation. The functions will be called in
 *   the order specified.
 * @param $dest
 *   A string containing the directory $source should be copied to. If this is
 *   not provided or is not writable, the temporary directory will be used.
 * @param $account
 *   The user account object that should associated with the uploaded file.
 * @return
 *   An array containing the file information, or 0 in the event of an error.
 */
function field_file_save_file($filepath, $validators = array(), $dest = FALSE, $account = NULL) {
  if (!isset($account)) {
    $account = $GLOBALS['user'];
  }

  // Add in our check of the the file name length.
  $validators['file_validate_name_length'] = array();

  // Begin building file object.
  $file = new stdClass();
  $file->uid = $account->uid;
  $file->filename = basename($filepath);
  $file->filepath = $filepath;
  $file->filemime = module_exists('mimedetect') ? mimedetect_mime($file) : file_get_mimetype($file->filename);

  // Rename potentially executable files, to help prevent exploits.
  if (preg_match('/\.(php|pl|py|cgi|asp|js)$/i', $file->filename) && (substr($file->filename, -4) != '.txt')) {
    $file->filemime = 'text/plain';
    $file->filepath .= '.txt';
    $file->filename .= '.txt';
  }

  // If the destination is not provided, or is not writable, then use the
  // temporary directory.
  if (empty($dest) || file_check_path($dest) === FALSE) {
    $dest = file_directory_temp();
  }

  $file->source = 'field_file_save_file';
  $file->destination = file_destination(file_create_path($dest .'/'. $file->filename), FILE_EXISTS_RENAME);
  $file->filesize = filesize($filepath);

  // Call the validation functions.
  $errors = array();
  foreach ($validators as $function => $args) {
    array_unshift($args, $file);
    $errors = array_merge($errors, call_user_func_array($function, $args));
  }

  // Check for validation errors.
  if (!empty($errors)) {
    $message = t('The selected file %name could not be saved.', array('%name' => $file->filename));
    if (count($errors) > 1) {
      $message .= '<ul><li>'. implode('</li><li>', $errors) .'</li></ul>';
    }
    else {
      $message .= ' '. array_pop($errors);
    }
    form_set_error($file->source, $message);
    return 0;
  }

  if (!file_copy($file, $file->destination, FILE_EXISTS_RENAME)) {
    form_set_error($file->source, t('File upload error. Could not move uploaded file.'));
    watchdog('file', 'Upload error. Could not move file %file to destination %destination.', array('%file' => $file->filename, '%destination' => $file->destination));
    return 0;
  }

  // If we made it this far it's safe to record this file in the database.
  $file->status = FILE_STATUS_TEMPORARY;
  $file->timestamp = time();
  // Insert new record to the database.
  drupal_write_record('files', $file);

  // Let modules add additional properties to the yet barebone file object.
  foreach (module_implements('file_insert') as $module) {
    $function =  $module .'_file_insert';
    $function($file);
  }
  _field_file_cache($file); // cache the file in order to minimize load queries
  return (array)$file;
}

/**
 * Save a node file. Delete items if necessary and set new items as permanent.
 *
 * @param $node
 *    Node object this file is be associated with.
 * @param $file
 *    File to be inserted, passed by reference since fid should be attached.
 * @return array
 */
function field_file_save($node, &$file) {
  // If this item is marked for deletion.
  if (!empty($file['delete']) || !empty($file['_remove'])) {
    // If we're creating a new revision, return an empty array so CCK will
    // remove the item.
    if (!empty($node->old_vid)) {
      return array();
    }
    // Otherwise delete the file and return an empty array.
    if (field_file_delete($file)) {
      return array();
    }
  }

  // Cast to object since core functions use objects.
  $file = (object)$file;

  // Set permanent status on files if unset.
  if (empty($file->status)) {
    file_set_status($file, FILE_STATUS_PERMANENT);
  }

  // Let modules update their additional file properties too.
  foreach (module_implements('file_update') as $module) {
    $function =  $module .'_file_update';
    $function($file);
  }
  _field_file_cache($file); // update the cache, in case the file has changed

  $file = (array)$file;
  return $file;
}

/**
 * Delete a field file and its database record.
 *
 * @param $path
 *   A file object.
 * @param $force
 *   Force File Deletion ignoring reference counting.
 * @return mixed
 *   TRUE for success, Array for reference count block, or FALSE in the event of an error.
 */
function field_file_delete($file, $force = FALSE) {
  $file = (object)$file;
  // If any module returns a value from the reference hook, the
  // file will not be deleted from Drupal, but file_delete will
  // return a populated array that tests as TRUE.
  if (!$force && $references = module_invoke_all('file_references', $file)) {
    $references = array_filter($references); // only keep positive values
    if (!empty($references)) {
      return $references;
    }
  }

  // Let other modules clean up on delete.
  module_invoke_all('file_delete', $file);

  // Make sure the file is deleted before removing its row from the
  // database, so UIs can still find the file in the database.
  if (file_delete($file->filepath)) {
    db_query('DELETE FROM {files} WHERE fid = %d', $file->fid);
    _field_file_cache(NULL, $file); // delete the file from the cache
    return TRUE;
  }
  return FALSE;
}

/**
 * Internal cache, in order to minimize database queries for loading files.
 */
function _field_file_cache($file = NULL, $reset = FALSE) {
  static $files = array();

  // Reset internal cache.
  if (is_object($reset)) { // file object, uncache just that one
    unset($files[$reset->fid]);
    unset($files[$reset->filepath]);
  }
  else if ($reset) { // TRUE, delete the whole cache
    $files = array();
  }

  // Cache the file by both fid and filepath.
  // Use non-copying objects to save memory.
  if (!empty($file->fid)) {
    $files[$file->fid] = $file;
    $files[$file->filepath] = $file;
  }
  return $files;
}

/**
 * A silent version of file.inc's file_check_directory().
 *
 * This function differs from file_check_directory in that it checks for
 * files when doing the directory check and it does not use drupal_set_message()
 * when creating directories. This function may be removed in Drupal 7.
 *
 * Check that the directory exists and is writable. Directories need to
 * have execute permissions to be considered a directory by FTP servers, etc.
 *
 * @param $directory A string containing the name of a directory path.
 * @param $mode A Boolean value to indicate if the directory should be created
 *   if it does not exist or made writable if it is read-only.
 * @param $form_item An optional string containing the name of a form item that
 *   any errors will be attached to. This is useful for settings forms that
 *   require the user to specify a writable directory. If it can't be made to
 *   work, a form error will be set preventing them from saving the settings.
 * @return FALSE when directory not found, or TRUE when directory exists.
 */
function field_file_check_directory(&$directory, $mode = 0, $form_item = NULL) {
  $directory = rtrim($directory, '/\\');

  // Error if the directory is a file.
  if (is_file($directory)) {
    watchdog('file system', 'The path %directory was checked as a directory, but it is a file.',  array('%directory' => $directory), WATCHDOG_ERROR);
    if ($form_item) {
      form_set_error($form_item, t('The directory %directory is a file and cannot be overwritten.', array('%directory' => $directory)));
    }
    return FALSE;
  }

  // Create the directory if it is missing.
  if (!is_dir($directory) && $mode & FILE_CREATE_DIRECTORY && !@mkdir($directory, 0775, TRUE)) {
    watchdog('file system', 'The directory %directory does not exist.', array('%directory' => $directory), WATCHDOG_ERROR);
    if ($form_item) {
      form_set_error($form_item, t('The directory %directory does not exist.', array('%directory' => $directory)));
    }
    return FALSE;
  }

  // Check to see if the directory is writable.
  if (!is_writable($directory) && $mode & FILE_MODIFY_PERMISSIONS && !@chmod($directory, 0775)) {
    watchdog('file system', 'The directory %directory is not writable, because it does not have the correct permissions set.', array('%directory' => $directory), WATCHDOG_ERROR);
    if ($form_item) {
      form_set_error($form_item, t('The directory %directory is not writable', array('%directory' => $directory)));
    }
    return FALSE;
  }

  if ((file_directory_path() == $directory || file_directory_temp() == $directory) && !is_file("$directory/.htaccess")) {
    $htaccess_lines = "SetHandler Drupal_Security_Do_Not_Remove_See_SA_2006_006\nOptions None\nOptions +FollowSymLinks";
    if (($fp = fopen("$directory/.htaccess", 'w')) && fputs($fp, $htaccess_lines)) {
      fclose($fp);
      chmod($directory .'/.htaccess', 0664);
    }
    else {
      $repl = array('%directory' => $directory, '!htaccess' => nl2br(check_plain($htaccess_lines)));
      form_set_error($form_item, t("Security warning: Couldn't write .htaccess file. Please create a .htaccess file in your %directory directory which contains the following lines:<br /><code>!htaccess</code>", $repl));
      watchdog('security', "Security warning: Couldn't write .htaccess file. Please create a .htaccess file in your %directory directory which contains the following lines:<br /><code>!htaccess</code>", $repl, WATCHDOG_ERROR);
    }
  }

  return TRUE;
}

/**
 * Remove a possible leading file directory path from the given path.
 */
function field_file_strip_path($path) {
  $dirpath = file_directory_path();
  $dirlen = drupal_strlen($dirpath);
  if (drupal_substr($path, 0, $dirlen + 1) == $dirpath .'/') {
    $path = drupal_substr($path, $dirlen + 1);
  }
  return $path;
}

/**
 * Return a count of the references to a file by all modules.
 */
function field_file_references($file) {
  $references = (array) module_invoke_all('file_references', $file);
  $reference_count = 0;
  foreach ($references as $module => $count) {
    $reference_count += $count;
  }
  return $reference_count;
}
