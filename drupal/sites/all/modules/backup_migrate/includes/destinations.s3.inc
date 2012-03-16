<?php


/**
 * @file
 * Functions to handle the s3 backup destination.
 */

/**
 * A destination for sending database backups to an s3 server.
 *
 * @ingroup backup_migrate_destinations
 */
class backup_migrate_destination_s3 extends backup_migrate_destination_remote {
  var $supported_ops = array('scheduled backup', 'manual backup', 'restore', 'list files', 'configure', 'delete');
  var $s3 = NULL;
  var $cache_files = TRUE;


  /**
   * Save to to the s3 destination.
   */
  function _save_file($file, $settings) {
    if ($s3 = $this->s3_object()) {
      $path = $file->filename();
      if ($s3->putObject($s3->inputFile($file->filepath(), FALSE), $this->get_bucket(), $this->remote_path($file->filename()), S3::ACL_PRIVATE)) {
        return $file;
      }
    }
    return FALSE;
  }

  /**
   * Load from the s3 destination.
   */
  function load_file($file_id) {
    backup_migrate_include('files');
    $file = new backup_file(array('filename' => $file_id));
    if ($s3 = $this->s3_object()) {
      $data = $s3->getObject($this->get_bucket(), $this->remote_path($file_id), $file->filepath());
      if (!$data->error) {
        return $file;
      }
    }
    return NULL;
  }

  /**
   * Delete from the s3 destination.
   */
  function _delete_file($file_id) {
    if ($s3 = $this->s3_object()) {
      $s3->deleteObject($this->get_bucket(), $this->remote_path($file_id));
    }
  }

  /**
   * List all files from the s3 destination.
   */
  function _list_files() {
    backup_migrate_include('files');
    $files = array();
    if ($s3 = $this->s3_object()) {
      $s3_files = $s3->getBucket($this->get_bucket(), $this->get_subdir());
      foreach ((array)$s3_files as $id => $file) {
        $info = array(
          'filename' => $this->local_path($file['name']),
          'filesize' => $file['size'],
          'filetime' => $file['time'],
        );
        $files[$info['filename']] = new backup_file($info);
      }
    }
    return $files;
  }

  /**
   * Get the form for the settings for this filter.
   */
  function edit_form() {
    // Check for the library.
    $this->s3_object();

    $form = parent::edit_form();
    $form['scheme']['#type'] = 'value';
    $form['scheme']['#value'] = 'https';
    $form['host']['#type'] = 'value';
    $form['host']['#value'] = 's3.amazonaws.com';

    $form['path']['#title'] = 'S3 Bucket';
    $form['path']['#default_value'] = $this->get_bucket();
    $form['path']['#description'] = 'This bucket must already exist. It will not be created for you.';

    $form['user']['#title'] = 'Access Key ID';
    $form['pass']['#title'] = 'Secret Access Key';

    $form['subdir'] = array(
      '#type' => 'textfield',
      '#title' => t('Subdirectory'),
      '#default_value' => $this->get_subdir(),
      '#weight' => 25
    );
    $form['settings']['#weight'] = 50;

    return $form;
  }

  /**
   * Submit the form for the settings for the s3 destination.
   */
  function edit_form_submit($form, &$form_state) {
    // Append the subdir onto the path.

    if (!empty($form_state['values']['subdir'])) {
      $form_state['values']['path'] .= '/'. trim($form_state['values']['subdir'], '/');
    }
    parent::edit_form_submit($form, $form_state);
  }

  /**
   * Generate a filepath with the correct prefix.
   */
  function remote_path($path) {
    if ($subdir = $this->get_subdir()) {
      $path = $subdir .'/'. $path;
    }
    return $path;
  }

  /**
   * Generate a filepath with the correct prefix.
   */
  function local_path($path) {
    if ($subdir = $this->get_subdir()) {
      $path = str_replace($subdir .'/', '', $path);
    }
    return $path;
  }

  /**
   * Get the bucket which is the first part of the path.
   */
  function get_bucket() {
    $parts = explode('/', @$this->dest_url['path']);
    return $parts[0];
  }

  /**
   * Get the bucket which is the first part of the path.
   */
  function get_subdir() {
    // Support the older style of subdir saving.
    if ($subdir = $this->settings('subdir')) {
      return $subdir;
    }
    $parts = explode('/', @$this->dest_url['path']);
    array_shift($parts);
    return implode('/', array_filter($parts));
  }

  function s3_object() {
    // Try to use libraries module if available to find the path.
    if (function_exists('libraries_get_path')) {
      $library_paths[] = libraries_get_path('s3-php5-curl');
    }
    else {
      $library_paths[] = 'sites/all/libraries/s3-php5-curl';
    }
    $library_paths[] = drupal_get_path('module', 'backup_migrate') . '/includes/s3-php5-curl';
    $library_paths[] = drupal_get_path('module', 'backup_migrate') . '/includes';

    foreach($library_paths as $path) {
      if (file_exists($path . '/S3.php')) {
        require_once $path . '/S3.php';
        if (!$this->s3 && !empty($this->dest_url['user'])) {
          $this->s3 = new S3($this->dest_url['user'], $this->dest_url['pass']);
        }
        return $this->s3;
      }
    }
    drupal_set_message(t('Due to drupal.org code hosting policies, the S3 library needed to use an S3 destination is no longer distributed with this module. You must download the library from !link and place it in one of these locations: %locations.', array('%locations' => implode(', ', $library_paths), '!link' => l('http://undesigned.org.za/2007/10/22/amazon-s3-php-class', 'http://undesigned.org.za/2007/10/22/amazon-s3-php-class'))), 'error', FALSE);
    return NULL;
  }
}
