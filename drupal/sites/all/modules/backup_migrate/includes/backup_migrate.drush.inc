<?php


/**
 * @file
 * Drush commands for backup and migrate.
 */
 
/**
 * Implementation of hook_drush_command().
 */
function backup_migrate_drush_command() {
  $items['bam-backup'] = array(
    'callback' => 'backup_migrate_drush_backup',
    'description' => dt('Backup the site\'s database with Backup and Migrate.'),
    'aliases' => array('bb'),
    'examples' => array(
      'drush bam-backup' => 'Backup the default databse to the manual backup directory using the default settings.', 
      'drush bam-backup db scheduled mysettings' => 'Backup the database to the scheduled directory using a settings profile called "mysettings"', 
      'drush bam-backup files' => 'Backup the files directory to the manual directory using the default settings. The Backup and Migrate Files module is required for files backups.', 
    ),
    'arguments' => array(
      'source'        => "Optional. The id of the source (usually a database) to backup. Use 'drush bam-sources' to get a list of sources. Defaults to 'db'",
      'destination'   => "Optional. The id of destination to send the backup file to. Use 'drush bam-destinations' to get a list of destinations. Defaults to 'manual'",
      'profile'       => "Optional. The id of a settings profile to use. Use 'drush bam-profiles' to get a list of available profiles. Defaults to 'default'",
    ),
  );
  $items['bam-restore'] = array(
    'callback' => 'backup_migrate_drush_restore',
    'description' => dt('Restore the site\'s database with Backup and Migrate.'),
    'arguments' => array(
      'source'        => "Required. The id of the source (usually a database) to restore the backup to. Use 'drush bam-sources' to get a list of sources. Defaults to 'db'",
      'destination'   => "Required. The id of destination to send the backup file to. Use 'drush bam-destinations' to get a list of destinations. Defaults to 'manual'",
      'backup id'     => "Required. The id of a backup file restore. Use 'drush bam-backups' to get a list of available backup files.",
    ),
    'options' => array(
      'yes' => 'Skip confirmation',
    ),
  );
  $items['bam-destinations'] = array(
    'callback' => 'backup_migrate_drush_destinations',
    'description' => dt('Get a list of available destinations.'),
  );

  $items['bam-sources'] = array(
    'callback' => 'backup_migrate_drush_sources',
    'description' => dt('Get a list of available sources.'),
  );
  $items['bam-profiles'] = array(
    'callback' => 'backup_migrate_drush_profiles',
    'description' => dt('Get a list of available settings profiles.'),
  );
  $items['bam-backups'] = array(
    'callback' => 'backup_migrate_drush_destination_files',
    'description' => dt('Get a list of previously created backup files.'),
    'arguments' => array(
      'destination'   => "Required. The id of destination to list backups from. Use 'drush bam-destinations' to get a list of destinations.",
    ),
  );
  return $items;
}

/**
 * Implementation of hook_drush_help().
 */
function backup_migrate_drush_help($section) {
  switch ($section) {
    case 'drush:bam-backup':
      return dt("Backup the site's database using default settings.");
    case 'drush:bam-restore':
      return dt('Restore the site\'s database with Backup and Migrate.');
    case 'drush:bam-destinations':
      return dt('Get a list of available destinations.');
    case 'drush:bam-profiles':
      return dt('Get a list of available settings profiles.');
    case 'drush:bam-backups':
      return dt('Get a list of previously created backup files.');
  }
}

/**
 * Backup the default database.
 */
function backup_migrate_drush_backup($source_id = 'db', $destination_id = 'manual', $profile_id = 'default') {
  backup_migrate_include('profiles', 'destinations');

  // Set the message mode to logging.
  _backup_migrate_message_callback('_backup_migrate_message_drush');

  if (!backup_migrate_get_destination($source_id)) {
    _backup_migrate_message("Could not find the source '@source'. Try using 'drush bam-sources' to get a list of available sources or use 'db' to backup the Drupal database.", array('@source' => $source_id), 'error');
    return;
  }
  if (!backup_migrate_get_destination($destination_id)) {
    _backup_migrate_message("Could not find the destination '@destination'. Try using 'drush bam-destinations' to get a list of available destinations.", array('@destination' => $destination_id), 'error');
    return;
  }
  $settings = backup_migrate_get_profile($profile_id);
  if(!$settings) {
    _backup_migrate_message("Could not find the profile '@profile'. Try using 'drush bam-profiles' to get a list of available profiles.", array('@profile' => $profile_id), 'error');
    return;
  }

  _backup_migrate_message('Starting backup...');
  $settings->destination_id = $destination_id;
  $settings->source_id = $source_id;
  backup_migrate_perform_backup($settings);
}

/**
 * Restore to the default database.
 */
function backup_migrate_drush_restore($source_id = '', $destination_id = '', $file_id = '') {
  drush_print(dt('Restoring will delete some or all of your data and cannot be undone. ALWAYS TEST YOUR BACKUPS ON A NON-PRODUCTION SERVER!'));
  if (!drush_confirm(dt('Are you sure you want to restore the database?'))) {
    return drush_user_abort();
  }

  backup_migrate_include('profiles', 'destinations');

  // Set the message mode to drush output.
  _backup_migrate_message_callback('_backup_migrate_message_drush');

  if (!backup_migrate_get_destination($source_id)) {
    _backup_migrate_message("Could not find the source '@source'. Try using 'drush bam-sources' to get a list of available sources or use 'db' to backup the Drupal database.", array('@source' => $source_id), 'error');
    return;
  }
  if (!$destination = backup_migrate_get_destination($destination_id)) {
    _backup_migrate_message("Could not find the destination '@destination'. Try using 'drush bam-destinations' to get a list of available destinations.", array('@destination' => $destination_id), 'error');
    return;
  }
  if (!$file_id || !$file = backup_migrate_destination_get_file($destination_id, $file_id)) {
    _backup_migrate_message("Could not find the file '@file'. Try using 'drush bam-backups @destination' to get a list of available backup files in this destination destinations.", array('@destination' => $destination_id, '@file' => $file_id), 'error');
    return;
  }

  _backup_migrate_message('Starting restore...');
  $settings = array('source_id' => $source_id);
  backup_migrate_perform_restore($destination_id, $file_id);
}

/**
 * Get a list of available destinations.
 */
function backup_migrate_drush_destinations() {
  return _backup_migrate_drush_destinations('all');
}

/**
 * Get a list of available sources.
 */
function backup_migrate_drush_sources() {
  return _backup_migrate_drush_destinations('source');
}


/**
 * Get a list of available destinations with the given op.
 */
function _backup_migrate_drush_destinations($op = NULL) {
  backup_migrate_include('destinations');
  $rows = array(array(dt('ID'), dt('Name'), dt('Operations')));
  foreach (backup_migrate_get_destinations($op) as $destination) {
    $rows[] = array(
      $destination->get_id(),
      $destination->get_name(),
      implode (', ', $destination->ops()),
    );
  }
  drush_print_table($rows, TRUE, array(32, 32));
}

/**
 * Get a list of available profiles.
 */
function backup_migrate_drush_profiles() {
  backup_migrate_include('profiles');
  $rows = array(array(dt('ID'), dt('Name')));
  foreach (backup_migrate_get_profiles() as $profile) {
    $rows[] = array(
      $profile->get_id(),
      $profile->get_name(),
    );
  }
  drush_print_table($rows, TRUE, array(32, 32));
}

/**
 * Get a list of files in a given destination
 */
function backup_migrate_drush_destination_files($destination_id = NULL) {
  backup_migrate_include('destinations');

  // Set the message mode to drush output.
  _backup_migrate_message_callback('_backup_migrate_message_drush');

  if (!$destination_id) {
    _backup_migrate_message("You must specify an existing destination. Try using 'drush bam-destinations' to get a list of available destinations.", array('@destination' => $destination_id), 'error');
    return;
  }
  if (!$destination = backup_migrate_get_destination($destination_id)) {
    _backup_migrate_message("Could not find the destination '@destination'. Try using 'drush bam-destinations' to get a list of available destinations.", array('@destination' => $destination_id), 'error');
    return;
  }

  $out = array(array(
    dt('Filename'),
    dt('Date'),
    dt('Age'),
    dt('Size'),
  ));

  $files      = $destination->list_files();
  $i          = 0;
  foreach ((array)$files as $file) {
    // Show only files that can be restored from.
    if ($file->is_recognized_type()) {
      $info = $file->info();
      $out[] = array(
        check_plain($info['filename']),
        format_date($info['filetime'], 'small'),
        format_interval(time() - $info['filetime'], 1),
        format_size($info['filesize']),
      );
    }
  }
  if (count($out) > 1) {
    drush_print_table($out, TRUE);
  }
  else {
    drush_print(dt('There are no backup files to display.'));
  }
}

/**
 * Send a message to the drush log.
 */
function _backup_migrate_message_drush($message, $replace, $type) {
  // Use drush_log to display to the user.
  drush_log(strip_tags(dt($message, $replace)), str_replace('status', 'notice', $type));
  // Watchdog log the message as well for admins.
  _backup_migrate_message_log($message, $replace, $type);
}
