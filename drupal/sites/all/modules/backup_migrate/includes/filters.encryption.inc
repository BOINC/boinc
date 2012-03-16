<?php


/**
 * @file
 * A filter for encrypting bckups with AES.
 */

/**
 * A filter for encrypting backup files.
 *
 * @ingroup backup_migrate_filters
 */
class backup_migrate_filter_encryption extends backup_migrate_filter {
  var $op_weights = array('backup' => 170, 'restore' => -170);

  /**
   * This function is called on a backup file after the backup has been completed.
   */
  function backup($file, $settings) {
    return $this->file_encrypt($file, $settings);
  }

  /**
   * This function is called on a backup file before importing it.
   */
  function restore($file, &$settings) {
    return $this->file_decrypt($file);
  }

  /**
   * Get the form for the settings for this filter.
   */
  function backup_settings_default() {
    return array('encryption' => 'none');
  }

  /**
   * Get the form for the settings for this filter.
   */
  function backup_settings_form($settings) {
    $form = array();
    $options = $this->_backup_migrate_get_encryption_form_item_options();
    if (count($options) > 1) {
      $form['file']['encryption'] = array(
        "#type" => "select",
        "#title" => t("File Encryption (EXPERIMENTAL)"),
        "#options" => $options,
        "#default_value" => @$settings['encryption'],
        '#description' => t('Encrypted files can only be restored by Backup and Migrate and only on sites with the same encryption key. <strong>This functionality is experimental, and should only be used for testing.</strong>'),
      );
    }
    else {
      $form['file']['encryption'] = array(
        "#type" => 'item',
        "#title" => t("File Encryption"),
        "#value" => t('Install the !link to enable backup file encryption.', array('!link' => l(t('AES Encryption Module'), 'http://drupal.org/project/aes'))),
      );
    }

    return $form;
  }

  /**
   * Return a list of backup filetypes.
   */
  function file_types() {
    return array(
      "aes" => array(
        "extension" => "aes",
        "filemime" => "application/octet-stream",
        "backup" => TRUE,
        "restore" => TRUE,
      ),
    );
  }

  /**
   * Get the compression options as an options array for a form item.
   */
  function _backup_migrate_get_encryption_form_item_options() {
    $options = array();
    $options = array('' => t('No Encryption'));
    if (@function_exists("aes_encrypt")) {
      $options['aes'] = t("AES Encryption");
    }
    return $options;
  }

  /**
   * AES encrypt a file.
   */
  function aes_encrypt($source, $dest) {
    $success = FALSE;
    if (function_exists('aes_encrypt')) {
      if ($data = $source->get_contents()) {
        // Add a marker to the end of the data so we can trim the padding on decrpypt.
        $data = pack("a*H2", $data, "80");
        if ($data = aes_encrypt($data, FALSE)) {
          $dest->put_contents($data);
          $success = TRUE;
        }
      }
    }
    return $success;
  }

  /**
   * Gzip decode a file.
   */
  function aes_decrypt($source, $dest) {
    $success = FALSE;
    if (function_exists('aes_decrypt')) {
      if ($data = $source->get_contents()) {
        if ($data = aes_decrypt($data, FALSE)) {
          // Trim all the padding zeros plus our non-zero marker.
          $data = substr(rtrim($data, "\0"), 0, -1);
          $dest->put_contents($data);
          $success = TRUE;
        }
      }
    }
    return $success;
  }

  /**
   * Compress a file with the given settings.
   *  Also updates settings to reflect new file mime and file extension.
   */
  function file_encrypt($file, $settings) {
    if (!empty($settings->filters['encryption'])) {
      switch ($settings->filters['encryption']) {
        case "aes":
          $from = $file->push_type('aes');
          $from = new backup_file(array('filepath' => $from));
          if (!$success = $this->aes_encrypt($from, $file)) {
            $file = NULL;
          }
          break;
      }
      if (!$file) {
        _backup_migrate_message("Could not encrypt backup file. Try backing up without encryption.", array(), 'error');
      }
    }
    return $file;
  }

  /**
   * Decompress a file with the given settings.
   *  Also updates settings to reflect new file mime and file extension.
   */
  function file_decrypt($file) {
    $success = FALSE;
    if ($file) {
      switch ($file->type_id()) {
        case "aes":
          $from = $file->pop_type();
          $success = $this->aes_decrypt($from, $file);
          break;
        default:
          return $file;
        break;
    }
  
      if (!$success) {
        if (function_exists('aes_decrypt')) {
          _backup_migrate_message("Could not decrpyt backup file. Please check that the file is valid and that the encryption key of the server matches the server that created the backup.", array(), 'error');
        }
        else {
          _backup_migrate_message('You must install the !link to restore encrypted backkups.', array('!link' => l(t('AES Encryption Module'), 'http://drupal.org/project/aes')), 'error');
        }
      }
    }
    return $success ? $file : NULL;
  }
}

