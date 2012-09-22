<?php


/**
 * @file
 * Functions to handle the FTP backup destination.
 */

/**
 * A destination for sending database backups to an FTP server.
 *
 * @ingroup backup_migrate_destinations
 */
class backup_migrate_destination_ftp extends backup_migrate_destination_remote {
  var $supported_ops = array('scheduled backup', 'manual backup', 'restore', 'list files', 'configure', 'delete');
  var $ftp = NULL;

  /**
   * Save to the ftp destination.
   */
  function _save_file($file, $settings) {
    $ftp = $this->ftp_object();
    if (drupal_ftp_file_to_ftp($file->filepath(), $file->filename(), '.', $ftp)) {
      return $file;
    }
    return FALSE;
  }

  /**
   * Load from the ftp destination.
   */
  function load_file($file_id) {
    backup_migrate_include('files');
    $file = new backup_file(array('filename' => $file_id));
    $this->ftp_object();
    if (drupal_ftp_ftp_to_file($file->filepath(), $file_id, '.', $this->ftp)) {
      return $file;
    }
    return FALSE;
  }

  /**
   * Delete from the ftp destination.
   */
  function _delete_file($file_id) {
    $this->ftp_object();
    drupal_ftp_delete_file($file_id, $this->ftp);
  }

  function _list_files() {
    backup_migrate_include('files');
    $files = array();
    $this->ftp_object();
    $ftp_files = drupal_ftp_file_list('.', $this->ftp);
    foreach ($ftp_files as $file) {
      $files[$file['filename']] = new backup_file($file);
    }
    return $files;
  }

  /**
   * Get the form for the settings for this filter.
   */
  function edit_form() {
    $form = parent::edit_form();
    $form['scheme']['#type'] = 'value';
    $form['scheme']['#value'] = 'ftp';
    $form['port'] = array(
      "#type" => "textfield",
      "#title" => t("Port"),
      "#default_value" => @$this->dest_url['port'] ? $this->dest_url['port'] : '21',
      "#weight" => 15,
    );
    $form['pasv'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use PASV transfers'),
      '#default_value' => $this->get_pasv(),
      '#weight' => 50,
    );
    return $form;
  }

  function set_pasv($value) {
    $this->settings['pasv'] = (bool)$value;
  }

  function get_pasv() {
    return isset($this->settings['pasv']) ? $this->settings['pasv'] : FALSE;
  }

  function ftp_object() {
    if (!$this->ftp) {
      $this->dest_url['port'] = empty($this->dest_url['port']) ? '21' : $this->dest_url['port'];
      $this->dest_url['pasv'] = $this->get_pasv();
      $this->ftp = drupal_ftp_ftp_object($this->dest_url['host'], $this->dest_url['port'], $this->dest_url['user'], $this->dest_url['pass'], $this->dest_url['path'], $this->dest_url['pasv']);
    }
    return $this->ftp;
  }
}

// The FTP code below was taken from the ftp module by Aaron Winborn.

// Inspired by http://www.devarticles.com/c/a/PHP/My-FTP-Wrapper-Class-for-PHP/
// It's been drupalized, however, and most of the bugs from that example have been fixed.
// - winborn 2007-06-22 - 2007-06-28

define('DRUPAL_FTP_FT_DIRECTORY', 0);
define('DRUPAL_FTP_FT_FILE', 1);

/**
 *  creates a new ftp object. if any elements of ftp_map are missing, they'll be filled with the server defaults.
 */
function drupal_ftp_ftp_object($server, $port, $user, $pass, $dir, $pasv) {
  $ftp = new stdClass();

  $ftp->__server = $server;
  $ftp->__port = $port;
  $ftp->__user = $user;
  $ftp->__password = $pass;
  $ftp->__directory = $dir;
  $ftp->__pasv = $pasv;

  return $ftp;
}

/**
 *  The drupal_ftp_connect function
 *  This function connects to an FTP server and attempts to change into the directory specified by
 *  the fourth parameter, $directory.
 */
function drupal_ftp_connect(&$ftp) {
  if (is_NULL($ftp)) {
    $ftp = drupal_ftp_ftp_object();
  }

  if (empty($ftp->__conn) && !drupal_ftp_connected($ftp)) {
    // Attempt to connect to the remote server
    $ftp->__conn = @ftp_connect($ftp->__server, $ftp->__port);

    if (!$ftp->__conn) {
      _backup_migrate_message('FTP Error: Couldn\'t connect to server @server', array('@server' => $ftp->__server), 'error');
      return FALSE;
    }

    // Attempt to login to the remote server
    $ftp->__login = @ftp_login($ftp->__conn, $ftp->__user, $ftp->__password);

    if (!$ftp->__login) {
      _backup_migrate_message('FTP Error: Couldn\'t login as user @ftp_user to @server', array('@ftp_user' => $ftp->__user, '@server' => $ftp->__server), 'error');
      return FALSE;
    }

    // Attempt to change into the working directory
    $chdir = @ftp_chdir($ftp->__conn, $ftp->__directory);

    if (!$chdir) {
      _backup_migrate_message('FTP Error: Couldn\'t change into the @directory directory', array('@directory' => $ftp->__directory), 'error');
      return FALSE;
    }

    // Set PASV - if needed
    if ($ftp->__pasv) {
      $pasv = @ftp_pasv($ftp->__conn, TRUE);
      if (!$pasv) {
        _backup_migrate_message('FTP Error: Couldn\'t set PASV mode', array(), 'error');
        return FALSE;
      }
    }
  }

  // Everything worked OK, return TRUE
  return TRUE;
}

/**
 * The drupal_ftp_connected function
 * This function queries the FTP server with the ftp_systype command to check if the connection is still alive.
 * It returns TRUE on success or FALSE on disconnection.
 */
function drupal_ftp_connected(&$ftp) {
  // Attempt to call the ftp_systype to see if the connect
  // to the FTP server is still alive and kicking

  if (is_NULL($ftp)) {
    $ftp = drupal_ftp_ftp_object();
    return FALSE;
  }

  if (!@ftp_systype($ftp->__conn)) {
    // The connection is dead
    return FALSE;
  }
  else {
    // The connection is still alive
    return TRUE;
  }
}

/**
 *  This function tries to retrieve the contents of a file from the FTP server.
 *  Firstly it changes into the $directory directory, and then attempts to download the file $filename.
 *  The file is saved locally and its contents are returned to the caller of the function.
 */
function drupal_ftp_ftp_to_file($file, $filename, $directory, &$ftp) {
  // Change into the remote directory and retrieve the content
  // of a file. Once retrieve, return this value to the caller

  if (!@drupal_ftp_connect($ftp)) {
    return FALSE;
  }

  // We are now connected, so let's retrieve the file contents.
  // Firstly, we change into the directory
  $chdir = @ftp_chdir($ftp->__conn, $directory);

  if (!$chdir) {
    _backup_migrate_message('FTP Error: Couldn\'t change into directory: @directory', array('@directory' => $directory), 'error');
    return FALSE;
  }

  // We have changed into the directory, let's attempt to get the file
  $fp = @fopen($file, 'wb');
  $get_file = @ftp_fget($ftp->__conn, $fp, $filename, FTP_BINARY);
  fclose($fp);

  $fp = NULL;

  if (!$get_file) {
    _backup_migrate_message('FTP Error: Unable to download file: @filename from @directory', array('@filename' => $filename, '@directory' => $directory), 'error');
    return FALSE;
  }
  
  return TRUE;
}

/**
 * Send a file to an FTP server.
 */
function drupal_ftp_file_to_ftp($file, $ftp_filename, $ftp_directory, &$ftp) {
  if (!@drupal_ftp_connect($ftp)) {
    return FALSE;
  }

  if ($source = file_create_path($file)) {
    // Now we can try to write to the remote file
    $complete_filename = $ftp_directory .'/'. $ftp_filename;
    $put_file = @ftp_put($ftp->__conn, $complete_filename, $source, FTP_BINARY);
    if (!$put_file) {
      _backup_migrate_message('FTP Error: Couldn\'t write to @complete_filename when trying to save file on the ftp server.', array('@complete_filename' => $complete_filename), 'error');
      return FALSE;
    }

    // Everything worked OK
    return TRUE;
  }
  else {
    _backup_migrate_message('FTP Error: Couldn\'t find @file.', array('@file'), 'error');
    return FALSE;
  }
}

/**
 *  The drupal_ftp_change_directory Function
 *  This function simply changes into the $directory folder on the FTP server.
 *  If a connection or permission error occurs then _backup_migrate_message() will contain the error message.
 */
function drupal_ftp_change_directory($directory, &$ftp) {
  // Switch to another directory on the web server. If we don't
  // have permissions then an error will occur

  if (!@drupal_ftp_connect($ftp)) {
    return FALSE;
  }

  // Try and change into another directory
  $chdir = ftp_chdir($ftp->__conn, $directory);

  if (!$chdir) {
    _backup_migrate_message('FTP Error: Couldn\'t change into directory: @directory', array('@directory', $directory), 'error');
    return FALSE;
  }
  else {
    // Changing directories worked OK
    return TRUE;
  }
}

/**
 *  The drupal_ftp_file_list Function
 *  This function will change into the $directory folder and get a list of files and directories contained in that folder.
 *  This function still needs a lot of work, but should work in most cases.
 */
function drupal_ftp_file_list($directory, &$ftp) {
  // This function will attempt to change into the specified
  // directory and retrieve a list of files as an associative
  // array. This list will include file name, size and date last modified

  $file_array = array();

  // Can we switch to the desired directory?
  if (!drupal_ftp_change_directory($directory, $ftp)) {
    return FALSE;
  }

  // We are in the directory, let's retrieve a list of files
  // This is slower than parsing the raw return values, but it is faster.
  $file_list = ftp_nlist($ftp->__conn, $directory);

  // Save the list of files
  if (@is_array($file_list)) {
    // Interate through the array
    foreach ($file_list as $file) {
      $file_array[] = array(
        'filename' => $file,
        'filesize' => ftp_size($ftp->__conn, $directory ."/". $file),
        'filetime' => ftp_mdtm($ftp->__conn, $directory ."/". $file),
      );
    }
  }
  sort($file_array);
  return $file_array;
}

/**
 *  The drupal_ftp_create_directory Function
 *  This function tries to make a new directory called $folder_name on the FTP server.
 *  If it can create the folder, then the folder is given appropriate rights with the CHMOD command.
 */
function drupal_ftp_create_directory($folder_name, &$ftp) {
  // Makes a new folder on the web server via FTP

  if (!@drupal_ftp_connect($ftp)) {
    return FALSE;
  }

  $create_result = @ftp_mkdir($ftp->__conn, $folder_name);

  if ($create_result == TRUE) {
    // Can we change the files permissions?
    $exec_result = @ftp_site($ftp->__conn, 'chmod 0777 '. $folder_name .'/');

    if ($exec_result == TRUE) {
      return TRUE;
    }
    else {
      _backup_migrate_message('FTP Error: Couldn\'t set owner permissions on @folder.', array('@folder', $folder_name), 'error');
      return FALSE;
    }
  }
  else {
    _backup_migrate_message('FTP Error: Couldn\'t create new folder @folder.', array('@folder', $folder_name), 'error');
    return FALSE;
  }
}

/**
 *  The drupal_ftp_delete_file Function
 *  This function attempts to delete a file called $filename from the FTP server.
 */
function drupal_ftp_delete_file($filename, &$ftp) {
  // Remove the specified file from the FTP server
  if (!@drupal_ftp_connect($ftp)) {
    return FALSE;
  }

  $delete_result = @ftp_delete($ftp->__conn, $filename);

  if ($delete_result == TRUE) {
    // The file/folder was renamed successfully
    return TRUE;
  }
  else {
    // Couldn't delete the selected file
    _backup_migrate_message('FTP Error: Couldn\'t delete the selected file: @filename', array('@filename' => $filename), 'error');
    return FALSE;
  }
}

/**
 *  The drupal_ftp_delete_folder Function
 *  This function was one of the hardest to write. It recursively deletes all files and folders from a directory called $folder_name.
 */
function drupal_ftp_delete_folder($folder_name, &$ftp) {
  if (!@drupal_ftp_connect($ftp)) {
    return FALSE;
  }

  @ftp_chdir($ftp->__conn, $folder_name);
  $location = @ftp_pwd($ftp->__conn);

  $directories = array();
  $files = array();
  $dir_counter = 0;
  $file_counter = 0;
  $content = @ftp_nlist($ftp->__conn, ".");

  for ($i = 0; $i < sizeof($content); $i++) {
    // If we can change into this then it's a directory.
    // If not, it's a file
    if ($content[$i] != "." && $content[$i] != "..") {
      if (@ftp_chdir($ftp->__conn, $content[$i])) {
        // We have a directory
        $directories[] = $content[$i];
        $dir_counter++;
        @ftp_cdup($ftp->__conn);
      }
      else {
        // We have a file
        $files[] = $content[$i];
        $file_counter++;
      }
    }
  }

  for ($j = 0; $j < $file_counter; $j++) {
    @ftp_delete($ftp->__conn, $files[$j]);
  }

  for ($j = 0; $j < $dir_counter; $j++) {
    if ($directories[$j] != "." OR $directories[$j] != "..") {
      $location = ftp_pwd($ftp->__conn);
      drupal_ftp_delete_folder($directories[$j], $ftp);
      @ftp_cdup ($ftp->__conn);
      @ftp_rmdir($ftp->__conn, $directories[$j]);
    }
  }

  // Lastly, we change into the directory that we want to delete and see
  // if we can cdup. If we can, we delete it.
  @ftp_chdir($ftp->__conn, $folder_name);
  @ftp_cdup($ftp->__conn);
  @ftp_rmdir($ftp->__conn, $folder_name);

  // Did the recursive folder/file deletion work?
  return TRUE;
}
