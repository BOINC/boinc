<?php


/**
 * @file
 * All of the destination handling code needed for Backup and Migrate.
 */

backup_migrate_include('crud');

/**
 * Get the available destination types.
 */
function backup_migrate_get_destination_types() {
  return module_invoke_all('backup_migrate_destination_types');
}

/**
 * Implementation of hook_backup_migrate_destination_types().
 *
 * Get the built in Backup and Migrate destination types.
 */
function backup_migrate_backup_migrate_destination_types() {
  $out = array();
  if (variable_get('backup_migrate_allow_backup_to_file', TRUE)) {
    $out += array(
      'file' => array(
        'description' => t('Save the backup files to any directory on the server which the web-server can write to.'),
        'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.file.inc',
        'class' => 'backup_migrate_destination_files',
        'type_name' => t('Server Directory'),
        'can_create' => TRUE,
      ),
      'file_manual' => array(
        'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.file.inc',
        'type_name' => t('Server Directory'),
        'class' => 'backup_migrate_destination_files_manual',
      ),
      'file_scheduled' => array(
        'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.file.inc',
        'type_name' => t('Server Directory'),
        'class' => 'backup_migrate_destination_files_scheduled',
      ),
    );
  }  
  $out += array(
    'browser_download' => array(
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.browser.inc',
      'class' => 'backup_migrate_destination_browser_download',
    ),
    'browser_upload' => array(
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.browser.inc',
      'class' => 'backup_migrate_destination_browser_upload',
    ),
    'db' => array(
      'type_name' => t('Database'),
      'description' => t('Import the backup directly into another database. Database destinations can also be used as a source to backup from.'),
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.db.inc',
      'class' => 'backup_migrate_destination_db',
      'can_create' => FALSE,
    ),
    'mysql' => array(
      'type_name' => t('MySQL Database'),
      'description' => t('Import the backup directly into another MySQL database. Database destinations can also be used as a source to backup from.'),
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.db.mysql.inc',
      'class' => 'backup_migrate_destination_db_mysql',
      'can_create' => TRUE,
    ),
    'ftp' => array(
      'description' => t('Save the backup files to any a directory on an FTP server.'),
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.ftp.inc',
      'class' => 'backup_migrate_destination_ftp',
      'type_name' => t('FTP Directory'),
      'can_create' => TRUE,
    ),
    's3' => array(
      'description' => t('Save the backup files to a bucket on your !link.', array('!link' => l(t('Amazon S3 account'), 'http://aws.amazon.com/s3/'))),
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.s3.inc',
      'class' => 'backup_migrate_destination_s3',
      'type_name' => t('Amazon S3 Bucket'),
      'can_create' => TRUE,
    ),
    'email' => array(
      'type_name' => t('Email'),
      'description' => t('Send the backup as an email attachment to the specified email address.'),
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/destinations.email.inc',
      'class' => 'backup_migrate_destination_email',
      'can_create' => TRUE,
    ),
  );

  return $out;
}

/**
 * Implementation of hook_backup_migrate_destinations().
 *
 * Get the built in backup destinations and those in the db.
 */
function backup_migrate_backup_migrate_destinations() {
  $out              = array();

  // Add the default, out of the box destinations for new users.
  if (variable_get('backup_migrate_allow_backup_to_file', TRUE)) {
    $out['manual']    = backup_migrate_create_destination('file_manual', array('destination_id' => 'manual'));
    $out['scheduled'] = backup_migrate_create_destination('file_scheduled', array('destination_id' => 'scheduled'));
  }
  // Add the browser destination for downloading to the desktop.
  if (variable_get('backup_migrate_allow_backup_to_download', TRUE)) {
    $out['download']  = backup_migrate_create_destination('browser_download');
  }
  $out['upload']    = backup_migrate_create_destination('browser_upload');

  // Expose the configured databases as sources.
  backup_migrate_include('filters');
  $out              += backup_migrate_filters_invoke_all('destinations');
/*
  global $db_url;
  $urls = is_array($db_url) ? $db_url : array('default' => $db_url);
  foreach ((array)$urls as $key => $url) {
    // Only mysql is currently supported. If more DBMs's are implemented, theis will need to be abstacted.
    if (strpos($url, 'mysql') === 0) {
      if ($destination = backup_migrate_create_destination('mysql', array('url' => $url))) {
        // Treat the default database differently because it is probably the only one available.
        if ($key == 'default') {
          $destination->set_id('db');
          $destination->set_name(t('Default Database'));
          // Dissalow backing up to the default database because that's confusing and potentially dangerous.
          unset($destination->supported_ops['scheduled backup']);
          unset($destination->supported_ops['manual backup']);
        }
        else {
          $destination->set_id('db:'. $key);
          $destination->set_name($key .": ". $destination->get_display_location());
        }
        $out[$destination->get_id()] = $destination;
      }
    }
  }
*/
  return $out;
}

/**
 * Get all the available backup destination.
 *
 * @param $op
 *  The operation which will be performed on the destination. Hooks can use this
 *  to return only those destinations appropriate for the given op.
 *  Options include:
 *    'manual backup' - destinations available for manual backup
 *    'scheduled backup' - destinations available for schedules backup
 *    'list files' - destinations whose backup files can be listed
 *    'restore' - destinations whose files can be restored from
 *    'all' - all available destinations should be returned
 */
function backup_migrate_get_destinations($op = 'all') {
  static $destinations = NULL;

  // Get the list of destinations and cache them locally.
  if ($destinations === NULL) {
    $destinations = backup_migrate_crud_get_items('destination');
  }

  // Return all if that's what was asked for.
  if ($op == 'all') {
    return $destinations;
  }

  // Return only those destinations which support the given op.
  $out = array();
  foreach ($destinations as $key => $destination) {
    if ($destination->op($op)) {
      $out[$key] = $destination;
    }
  }
  return $out;
}

/**
 * Get the destination of the given id.
 */
function backup_migrate_get_destination($id) {
  $destinations = backup_migrate_get_destinations('all');
  return empty($destinations[$id]) ? NULL : $destinations[$id];
}

/**
 * Create a destination object of the given type with the given params.
 */
function backup_migrate_create_destination($destination_type, $params = array()) {
  $params['type'] = $destination_type;
  return backup_migrate_destination::create($params);
}

/**
 * Load a file from a destination and return the file info.
 */
function backup_migrate_destination_get_file($destination_id, $file_id) {
  if ($destination = backup_migrate_get_destination($destination_id)) {
    return $destination->load_file($file_id);
  }
  return NULL;
}

/**
 * Check if a file exists in the given destination.
 */
function backup_migrate_destination_file_exists($destination_id, $file_id) {
  if ($destination = backup_migrate_get_destination($destination_id)) {
    return $destination->file_exists($file_id);
  }
  return NULL;
}

/**
 * Send a file to the destination specified by the settings array.
 */
function backup_migrate_destination_save_file($file, &$settings) {
  if ($destination = $settings->get_destination()) {
    $file = $destination->save_file($file, $settings);
    return $file;
  }
  return NULL;
}

/**
 * Delete a file in the given destination.
 */
function backup_migrate_destination_delete_file($destination_id, $file_id) {
  if ($destination = backup_migrate_get_destination($destination_id)) {
    return $destination->delete_file($file_id);
  }
}

/**
 * Get the action links for a file on a given destination.
 */
function _backup_migrate_destination_get_file_links($destination_id, $file_id) {
  $out = array();
  if ($destination = backup_migrate_get_destination($destination_id)) {
    $out = $destination->get_file_links($file_id);
  }
  return $out;
}

/* UI Menu Callbacks */

/**
 * List the backup files in the given destination.
 */
function backup_migrate_ui_destination_display_files($destination_id = NULL) {
  $rows = $sort = array();
  if ($destination = backup_migrate_get_destination($destination_id)) {
    // Refresh the file listing cache if requested.
    if (isset($_GET['refresh'])) {
      $destination->file_cache_clear();
      drupal_goto($_GET['q']);
    }


    drupal_set_title(t('%title Files', array('%title' => $destination->get_name())));
    $headers = array(
      array('data' => 'Filename', 'field' => 'filename'),
      array('data' => 'Date', 'field' => 'filetime'),
      array('data' => 'Age',  'field' => 'filetime', 'sort' => 'desc'),
      array('data' => 'Size', 'field' => 'filesize'),
      t('Operations'),
    );

    $sort_order = tablesort_get_order($headers);
    $sort_key   = $sort_order['sql'] ? $sort_order['sql'] : 'filetime';
    $sort_dir   = tablesort_get_sort($headers) == 'desc' ? SORT_DESC : SORT_ASC;

    $files      = $destination->list_files();

    $i          = 0;
    foreach ((array)$files as $file) {
      $info = $file->info();
      // Show only files that can be restored from.
      if ($file->is_recognized_type()) {
        $sort[] = $info[$sort_key];
        $rows[] = array(
          check_plain($info['filename']),
          format_date($info['filetime'], 'small'),
          format_interval(time() - $info['filetime'], 1),
          format_size($info['filesize']),
          implode(" | ", $destination->get_file_links($file->file_id())),
        );
      }
    }

    array_multisort($sort, $sort_dir, $rows);

    if ($files) {
      $out = theme("table", $headers, $rows);
    }
    else {
      $out = t('There are no backup files to display.');
    }
    if ($destination->cache_files && $destination->fetch_time) {
      $out .= '<div class="description">'. t('This listing was fetched !time ago. !refresh', array('!time' => format_interval(time() - $destination->fetch_time, 1), '!refresh' => l(t('fetch now'), $_GET['q'], array('query' => 'refresh=true')))) .'</div>';
    }
    return $out;
  }
  drupal_goto(BACKUP_MIGRATE_MENU_PATH . "/destination");
}

/**
 * Download a file to the browser.
 */
function backup_migrate_ui_destination_download_file($destination_id = NULL, $file_id = NULL) {
  if ($file = backup_migrate_destination_get_file($destination_id, $file_id)) {
    backup_migrate_include('files');
    $file->transfer();
  }
  drupal_goto(BACKUP_MIGRATE_MENU_PATH);
}

/**
 * Restore a backup file from a destination.
 */
function backup_migrate_ui_destination_restore_file($destination_id = NULL, $file_id = NULL) {
  if (backup_migrate_destination_file_exists($destination_id, $file_id)) {
    return drupal_get_form('backup_migrate_ui_destination_restore_file_confirm', $destination_id, $file_id);
  }
  _backup_migrate_message('Cannot restore from the the file: %file_id because it does not exist.', array('%file_id' => $file_id), 'error');
  if ($destination_id && user_access('access backup files')) {
    drupal_goto(BACKUP_MIGRATE_MENU_PATH . '/destination/list/files/' . $destination_id);
  }
  drupal_goto(BACKUP_MIGRATE_MENU_PATH);
}

/**
 * Ask confirmation for file restore.
 */
function backup_migrate_ui_destination_restore_file_confirm(&$form_state, $destination_id, $file_id) {
  $sources = _backup_migrate_get_destination_form_item_options('source');
  if (count($sources) > 1) {
    $form['source_id'] = array(
      "#type" => "select",
      "#title" => t("Database"),
      "#options" => _backup_migrate_get_destination_form_item_options('source'),
      "#description" => t("Choose the database to restore to. Any database destinations you have created and any databases specified in your settings.php can be restored to."),
      "#default_value" => 'db',
    );
  }
  else {
    $form['source_id'] = array(
      "#type" => "value",
      "#value" => 'db',
    );
  }

  $form['destination_id'] = array('#type' => 'value', '#value' => $destination_id);
  $form['file_id'] = array('#type' => 'value', '#value' => $file_id);
  $form = confirm_form($form, t('Are you sure you want to restore the database?'), BACKUP_MIGRATE_MENU_PATH . "/destination/list/files/". $destination_id, t('Are you sure you want to restore the database from the backup file %file_id? This will delete some or all of your data and cannot be undone. <strong>Always test your backups on a non-production server!</strong>', array('%file_id' => $file_id)), t('Restore'), t('Cancel'));
  drupal_set_message(t('Restoring will delete some or all of your data and cannot be undone. <strong>Always test your backups on a non-production server!</strong>'), 'warning', FALSE);
  $form = array_merge_recursive($form, backup_migrate_filters_settings_form(backup_migrate_filters_settings_default('restore'), 'restore'));
  $form['actions']['#weight'] = 100;

  // Add the advanced fieldset if there are any fields in it.
  if (@$form['advanced']) {
    $form['advanced']['#type'] = 'fieldset';
    $form['advanced']['#title'] = t('Advanced Options');
    $form['advanced']['#collapsed'] = true;
    $form['advanced']['#collapsible'] = true;
  }

  return $form;
}

/**
 * Do the file restore.
 */
function backup_migrate_ui_destination_restore_file_confirm_submit($form, &$form_state) {
  $destination_id = $form_state['values']['destination_id'];
  $file_id = $form_state['values']['file_id'];
  if ($destination_id && $file_id) {
    backup_migrate_perform_restore($destination_id, $file_id, $form_state['values']);
  }
  $redir = user_access('access backup files') ? BACKUP_MIGRATE_MENU_PATH . "/destination/list/files/". $destination_id : BACKUP_MIGRATE_MENU_PATH;
  $form_state['redirect'] = $redir;
}

/**
 * Menu callback to delete a file from a destination.
 */
function backup_migrate_ui_destination_delete_file($destination_id = NULL, $file_id = NULL) {
  if (backup_migrate_destination_file_exists($destination_id, $file_id)) {
    return drupal_get_form('backup_migrate_ui_destination_delete_file_confirm', $destination_id, $file_id);
  }
  _backup_migrate_message('Cannot delete the file: %file_id because it does not exist.', array('%file_id' => $file_id), 'error');
  if ($destination_id && user_access('access backup files')) {
    drupal_goto(BACKUP_MIGRATE_MENU_PATH .'/destination/list/files/' . $destination_id);
  }
  drupal_goto(BACKUP_MIGRATE_MENU_PATH);
}

/**
 * Ask confirmation for file deletion.
 */
function backup_migrate_ui_destination_delete_file_confirm(&$form_state, $destination_id, $file_id) {
  $form['destination_id'] = array('#type' => 'value', '#value' => $destination_id);
  $form['file_id'] = array('#type' => 'value', '#value' => $file_id);
  return confirm_form($form, t('Are you sure you want to delete the backup file?'), BACKUP_MIGRATE_MENU_PATH . '/destination/list/files/'. $destination_id, t('Are you sure you want to delete the backup file %file_id? <strong>This action cannot be undone.</strong>', array('%file_id' => $file_id)), t('Delete'), t('Cancel'));
}

/**
 * Delete confirmed, perform the delete.
 */
function backup_migrate_ui_destination_delete_file_confirm_submit($form, &$form_state) {
  if (user_access('delete backup files')) {
    $destination_id = $form_state['values']['destination_id'];
    $file_id = $form_state['values']['file_id'];
    backup_migrate_destination_delete_file($destination_id, $file_id);
    _backup_migrate_message('Database backup file deleted: %file_id', array('%file_id' => $file_id));
  }
  $form_state['redirect'] = user_access('access backup files') ? BACKUP_MIGRATE_MENU_PATH . "/destination/list/files/". $destination_id : BACKUP_MIGRATE_MENU_PATH;
}

/* Utilities */

/**
 * Get the source options as a form element.
 */
function _backup_migrate_get_source_form($source_id = 'db') {
  backup_migrate_include('destinations');

  $form = array();
  $sources = _backup_migrate_get_source_pulldown($source_id);
  if (count($sources['#options']) > 1) {
    $form['source'] = array(
      "#type" => "fieldset",
      "#title" => t("Backup Source"),
      "#collapsible" => TRUE,
      "#collapsed" => FALSE,
      "#tree" => FALSE,
    );
    $sources['#description'] = t("Choose the database to backup. Any database destinations you have created and any databases specified in your settings.php can be backed up.");

    $form['source']['source_id'] = $sources;
  }
  else {
    $form = array();
    $form['source']['source_id'] = array(
      "#type" => "value",
      "#value" => $source_id,
    );
  }
  return $form;
}

/**
 * Get pulldown to select existing source options.
 */
function _backup_migrate_get_source_pulldown($source_id = NULL) {
  backup_migrate_include('destinations');
  $sources = _backup_migrate_get_destination_form_item_options('source');
  $form = array(
    "#type" => "select",
    "#title" => t("Backup Source"),
    "#options" => _backup_migrate_get_destination_form_item_options('source'),
    "#default_value" => $source_id,
  );
  return $form;
}

/**
 * Get the destination options as an options array for a form item.
 */
function _backup_migrate_get_destination_form_item_options($op) {
  $out = array();
  foreach (backup_migrate_get_destinations($op) as $key => $destination) {
    $out[$key] = $destination->get_name();
  }
  return $out;
}

/**
 * A base class for creating destinations.
 */
class backup_migrate_destination extends backup_migrate_item {
  var $db_table = "backup_migrate_destinations";
  var $type_name = "destination";
  var $default_values = array('settings' => array());
  var $singular = 'destination';
  var $plural = 'destinations';
  var $cache_files = FALSE;
  var $fetch_time = NULL;
  var $cache_expire = 86400; // 24 hours

  var $destination_type = "";
  var $supported_ops = array();

  /**
   * This function is not supposed to be called. It is just here to help the po extractor out.
   */
  function strings() {
    // Help the pot extractor find these strings.
    t('Destination');
    t('Destinations');
    t('destinations');
    t('destination');
  }

  function ops() {
    return $this->supported_ops;
  }

  /**
   * Does this destination support the given operation.
   */
  function op($op) {
    $ops = (array)$this->ops();
    return in_array($op, $ops);
  }

  /**
   * Remove the given op from the support list.
   */
  function remove_op($op) {
    $key = array_search($op, $this->supported_ops);
    if ($key !== FALSE) {
      unset($this->supported_ops[$key]);
    }
  }

  function get_name() {
    return @$this->name;
  }

  function set_name($name) {
    return $this->name = $name;
  }

  function set_type($type) {
    $this->type = $type;
  }

  function set_location($location) {
    $this->location = $location;
  }

  function get_location() {
    return @$this->location;
  }

  function get_display_location() {
    return $this->get_location();
  }

  function file_types() {
    return array();
  }

  function settings($key = NULL) {
    $out = $this->settings;
    if ($key) {
      $out = isset($out[$key]) ? $out[$key] : NULL;
    }
    return $out;
  }

  /**
   * Get the type name of this destination for display to the user.
   */
  function get_destination_type_name() {
    if ($type = $this->destination_type) {
      $types = backup_migrate_get_destination_types();
      return isset($types[$type]['type_name']) ? $types[$type]['type_name'] : $type;
    }
  }

  /**
   * Save the given file to the destination.
   */
  function save_file($file, $settings) {
    // This must be overriden.
    $this->file_cache_clear();
    return $this->_save_file($file, $settings);
  }

  /**
   * Save the given file to the destination.
   */
  function _save_file($file, $settings) {
    // This must be overriden.
    return $file;
  }

  /**
   * Load the file with the given destination specific id and return as a backup_file object.
   */
  function load_file($file_id) {
    // This must be overriden.
    return NULL;
  }

  /**
   * Check if a file exists in the given destination.
   */
  function file_exists($file_id) {
    // Check if the file exists in the list of available files. Actual destination types may have more efficient ways of doing this.
    $files = $this->list_files();
    return isset($files[$file_id]);
  }

  /**
   * List all the available files in the given destination with their destination specific id.
   */
  function list_files() {
    $out = NULL;
    if ($this->cache_files) {
      $out = $this->file_cache_get();
    }
    if ($out === NULL) {
      $out = $this->_list_files();
      if ($this->cache_files) {
        $this->file_cache_set($out);
      }
    }
    return $out;
  }

  /**
   * List all the available files in the given destination with their destination specific id.
   */
  function _list_files() {
    return array();
  }

  /**
   * Cache the file list.
   */
  function file_cache_set($files) {
    cache_set('backup_migrate_file_list:'. $this->get_id(), $files, 'cache', time() + $this->cache_expire);
  }

  /**
   * Retrieve the file list.
   */
  function file_cache_get() {
    backup_migrate_include('files');
    $cache = cache_get('backup_migrate_file_list:'. $this->get_id());
    if (!empty($cache->data) && $cache->created > (time() - $this->cache_expire)) {
      $this->fetch_time = $cache->created;
      return $cache->data;
    }
    $this->fetch_time = 0;
    return NULL;
  }

  /**
   * Retrieve the file list.
   */
  function file_cache_clear() {
    if ($this->cache_files) {
      $this->file_cache_set(NULL);
    }
  }

  /**
   * Delete the file with the given destination specific id.
   */
  function delete_file($file_id) {
    $this->file_cache_clear();
    $this->_delete_file($file_id);
  }

  /**
   * Delete the file with the given destination specific id.
   */
  function _delete_file($file_id) {
    // This must be overriden.
  }

  /**
   * Get the edit form for the item.
   */
  function edit_form() {
    if (get_class($this) !== 'backup_migrate_destination') {
      $form = parent::edit_form();
      $form['name'] = array(
        "#type" => "textfield",
        "#title" => t("Destination name"),
        "#default_value" => $this->get_name(),
        "#required" => TRUE,
      );
      $form['type'] = array(
        "#type" => "value",
        "#default_value" => $this->destination_type,
      );
    }
    else {
      $types = backup_migrate_get_destination_types();
      $items = array();
      // If no (valid) node type has been provided, display a node type overview.
      foreach ($types as $key => $type) {
        if (@$type['can_create']) {
          $type_url_str = str_replace('_', '-', $key);
          $out = '<dt>'. l($type['type_name'], BACKUP_MIGRATE_MENU_PATH . "/destination/list/add/$type_url_str", array('attributes' => array('title' => t('Add a new @s destination.', array('@s' => $type['type_name']))))) .'</dt>';
          $out .= '<dd>'. filter_xss_admin($type['description']) .'</dd>';
          $items[] = $out;
        }
      }
      if (count($items)) {
        $output = t('Choose the type of destination you would like to create:') .'<dl>'. implode('', $items) .'</dl>';
      }
      else {
        $output = t('No destination types available.');
      }
      $form['select_type'] = array(
        '#type' => 'markup',
        '#value' => $output,
      );
    }
    return $form;
  }

  /**
   * Get the message to send to the user when confirming the deletion of the item.
   */
  function delete_confirm_message() {
    return t('Are you sure you want to delete the destination %name? Backup files already saved to this destination will not be deleted.', array('%name' => $this->get_name()));
  }


  /**
   * Get the columns needed to list the type.
   */  
  function get_list_column_info() {
    $out = parent::get_list_column_info();
    $out = array(
      'name'                  => array('title' => t('Name')),
      'destination_type_name' => array('title' => t('Type')),
      'display_location'      => array('title' => t('Location')),
    ) + $out;
    return $out;
  }

  /**
   * Get a row of data to be used in a list of items of this type.
   */  
  function get_list_row() {
    $out = parent::get_list_row();

    // Supress destinations with no actions as there's no value in showing them (and they may confuse new users).
    if (empty($out['actions'])) {
      return NULL;
    }
    return $out;
  }

  /**
   * Get the action links for a destination.
   */
  function get_action_links() {
    $out = parent::get_action_links();
    $item_id = $this->get_id();

    // Don't display the download/delete/restore ops if they are not available for this destination.
    if ($this->op('list files') && user_access("access backup files")) {
      $out = array('list files' => l(t("list files"), BACKUP_MIGRATE_MENU_PATH . "/$this->type_name/list/files/". $item_id)) + $out;
    }
    if (!$this->op('configure') || !user_access('administer backup and migrate')) {
      unset($out['edit']);
    }
    return $out;
  }

  /**
   * Get the action links for a file on a given destination.
   */
  function get_file_links($file_id) {
    $out = array();

    // Don't display the download/delete/restore ops if they are not available for this destination.
    $can_read       = $this->can_read_file($file_id);
    $can_delete     = $this->can_delete_file($file_id);

    $destination_id = $this->get_id();
    if ($can_read && user_access("access backup files")) {
      $out[] = l(t("download"), BACKUP_MIGRATE_MENU_PATH . "/destination/downloadfile/". $destination_id .'/'. $file_id);
    }
    if ($can_read && user_access("restore from backup")) {
      $out[] = l(t("restore"), BACKUP_MIGRATE_MENU_PATH . "/destination/list/restorefile/". $destination_id .'/'. $file_id);
    }
    if ($can_delete && user_access("delete backup files")) {
      $out[] = l(t("delete"), BACKUP_MIGRATE_MENU_PATH . "/destination/list/deletefile/". $destination_id .'/'. $file_id);
    }
    return $out;
  }

  /**
   * Determine if we can read the given file.
   */
  function can_read_file($file_id) {
    return $this->op('restore');
  }

  /**
   * Determine if we can read the given file.
   */
  function can_delete_file($file_id) {
    return $this->op('delete');
  }

  /**
   * Get the form for the settings for this destination type.
   */
  function settings_default() {
    return array();
  }

  /**
   * Get the form for the settings for this destination.
   */
  function settings_form($form) {
    return $form;
  }

  /**
   * Validate the form for the settings for this destination.
   */
  function settings_form_validate($form_values) {
  }

  /**
   * Submit the settings form. Any values returned will be saved.
   */
  function settings_form_submit($form_values) {
    return $form_values;
  }
  

  /**
   * Create a new destination of the correct type.
   */
  function create($params = array()) {
    $out = NULL;
    $types = backup_migrate_get_destination_types();

    // Get the type passed in in the params, or if none, check the url for a valid type name.
    // This is to allow new destination type to be specified in the path.
    $destination_type = !empty($params['type']) ? $params['type'] : arg(BACKUP_MIGRATE_MENU_DEPTH + 3);

    if ($destination_type && ($type = @$types[$destination_type])) {
      // Include the necessary file if specified by the type.
      if (!empty($type['file'])) {
        require_once './'. $type['file'];
      }
      $out = new $type['class']($params + array('destination_type' => $destination_type));
    }
    if (empty($out)) {
      $out = new backup_migrate_destination();
    }
    return $out;
  }

  /**
   * Add the menu items specific to the destination type.
   */
  function get_menu_items() {
    $items = parent::get_menu_items();
    $items[BACKUP_MIGRATE_MENU_PATH . '/destination/list/files'] = array(
      'title' => 'Destination Files',
      'page callback' => 'backup_migrate_menu_callback',
      'page arguments' => array('destinations', 'backup_migrate_ui_destination_display_files', TRUE),
      'access arguments' => array('access backup files'),
      'type' => MENU_LOCAL_TASK,
    );
    $items[BACKUP_MIGRATE_MENU_PATH . '/destination/list/deletefile'] = array(
      'title' => 'Delete File',
      'description' => 'Delete a backup file',
      'page callback' => 'backup_migrate_menu_callback',
      'page arguments' => array('destinations', 'backup_migrate_ui_destination_delete_file', TRUE),
      'access arguments' => array('delete backup files'),
      'type' => MENU_LOCAL_TASK,
    );
    $items['admin/content/backup_migrate/destination/list/restorefile'] = array(
      'title' => 'Restore from backup',
      'description' => 'Restore database from a backup file on the server',
      'page callback' => 'backup_migrate_menu_callback',
      'page arguments' => array('destinations', 'backup_migrate_ui_destination_restore_file', TRUE),
      'access arguments' => array('restore from backup'),
      'type' => MENU_LOCAL_TASK,
    );
    $items[BACKUP_MIGRATE_MENU_PATH . '/destination/downloadfile'] = array(
      'title' => 'Download File',
      'description' => 'Download a backup file',
      'page callback' => 'backup_migrate_menu_callback',
      'page arguments' => array('destinations', 'backup_migrate_ui_destination_download_file', TRUE),
      'access arguments' => array('access backup files'),
      'type' => MENU_CALLBACK,
    );
    return $items;
  }

  /**
   * Get the form for the settings for this filter.
   */
  function backup_settings_default() {
    return array();
  }

  /**
   * Get the form for the settings for this filter.
   */
  function backup_settings_form($settings) {
    return array();
  }

  /**
   * Get the form for the settings for this filter.
   */
  function backup_settings_form_validate($form, &$form_state) {
  }

  /**
   * Submit the settings form. Any values returned will be saved.
   */
  function backup_settings_form_submit($form, &$form_state) {
  }

  /**
   * Get the form for the settings for this filter.
   */
  function restore_settings_default() {
    return array();
  }

  /**
   * Get the form for the settings for this filter.
   */
  function restore_settings_form($settings) {
    return array();
  }

  /**
   * Get the form for the settings for this filter.
   */
  function restore_settings_form_validate($form_values) {
  }

  /**
   * Submit the settings form. Any values returned will be saved.
   */
  function restore_settings_form_submit($form_values) {
    return $form_values;
  }
}

/**
 * A base class for creating destinations.
 */
class backup_migrate_destination_remote extends backup_migrate_destination {
  /**
   * The location is a URI so parse it and store the parts.
   */
  function get_location() {
    return $this->url(FALSE);
  }

  /**
   * The location to display is the url without the password.
   */
  function get_display_location() {
    return $this->url(TRUE);
  }

  /**
   * Return the location with the password.
   */
  function set_location($location) {
    $this->location = $location;
    $this->set_url($location);
  }

  /**
   * Get a url from the parts.
   */
  function url($hide_password = TRUE) {
    return $this->glue_url($this->dest_url, $hide_password);
  }

  /**
   * Glue a URLs component parts back into a URL.
   */
  function glue_url($parts, $hide_password = TRUE) {
    // Obscure the password if we need to.
    $parts['pass'] = $hide_password ? "" : $parts['pass'];

    // Assemble the URL.
    $out = "";
    $out .= $parts['scheme'] .'://';
    $out .= $parts['user'] ? urlencode($parts['user']) : '';
    $out .= ($parts['user'] && $parts['pass']) ? ":". urlencode($parts['pass']) : '';
    $out .= ($parts['user'] || $parts['pass']) ? "@" : "";
    $out .= $parts['host'];
    $out .= !empty($parts['port']) ? ':'. $parts['port'] : '';
    $out .= "/". $parts['path'];
    return $out;
  }

  /**
   * Break a URL into it's component parts.
   */
  function set_url($url) {
    $parts          = (array)parse_url($url);
    $parts['user'] = urldecode(@$parts['user']);
    $parts['pass'] = urldecode(@$parts['pass']);
    $parts['path'] = urldecode(@$parts['path']);
    $parts['path']  = ltrim(@$parts['path'], "/");
    $this->dest_url = $parts;
  }

  /**
   * Destination configuration callback.
   */
  function edit_form() {
    $form = parent::edit_form();
    $form['scheme'] = array(
      "#type" => "select",
      "#title" => t("Scheme"),
      "#default_value" => @$this->dest_url['scheme'] ? $this->dest_url['scheme'] : 'mysql',
      "#required" => TRUE,
      "#options" => array($GLOBALS['db_type'] => $GLOBALS['db_type']),
      "#weight" => 0,
    );
    $form['host'] = array(
      "#type" => "textfield",
      "#title" => t("Host"),
      "#default_value" => @$this->dest_url['host'] ? $this->dest_url['host'] : 'localhost',
      "#required" => TRUE,
      "#weight" => 10,
    );
    $form['path'] = array(
      "#type" => "textfield",
      "#title" => t("Path"),
      "#default_value" => @$this->dest_url['path'],
      "#required" => TRUE,
      "#weight" => 20,
    );
    $form['user'] = array(
      "#type" => "textfield",
      "#title" => t("Username"),
      "#default_value" => @$this->dest_url['user'],
      "#required" => TRUE,
      "#weight" => 30,
    );
    $form['pass'] = array(
      "#type" => "password",
      "#title" => t("Password"),
      "#default_value" => @$this->dest_url['pass'],
      '#description' => '',
      "#weight" => 40,
    );
    if (@$this->dest_url['pass']) {
      $form['old_password'] = array(
        "#type" => "value",
        "#value" => @$this->dest_url['pass'],
      );
      $form['pass']["#description"] .= t(' You do not need to enter a password unless you wish to change the currently saved password.');
    }
    return $form;
  }

  /**
   * Submit the configuration form. Glue the url together and add the old password back if a new one was not specified.
   */
  function edit_form_submit($form, &$form_state) {
    $form_state['values']['pass'] = $form_state['values']['pass'] ? $form_state['values']['pass'] : $form_state['values']['old_password'];
    $form_state['values']['location'] = $this->glue_url($form_state['values'], FALSE);
    parent::edit_form_submit($form, $form_state);
  }
}

