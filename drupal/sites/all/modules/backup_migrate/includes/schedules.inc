<?php

/**
 * @file
 * All of the schedule handling code needed for Backup and Migrate.
 */

backup_migrate_include('crud');

/**
 * Run the preconfigured schedules. Called on cron.
 */
function backup_migrate_schedules_run() {
  backup_migrate_include('profiles');
  foreach (backup_migrate_get_schedules() as $schedule) {
    $schedule->cron();
  }
  backup_migrate_cleanup();
}

/**
 * Get all the available backup schedules.
 */
function backup_migrate_get_schedules() {
  static $schedules = NULL;
  // Get the list of schedules and cache them locally.
  if ($schedules === NULL) {
    $schedules = backup_migrate_crud_get_items('schedule');
  }
  return $schedules;
}

/**
 * Get the schedule info for the schedule with the given ID, or NULL if none exists.
 */
function backup_migrate_get_schedule($schedule_id) {
  $schedules = backup_migrate_get_schedules();
  return @$schedules[$schedule_id];
}

/**
 * A schedule class for crud operations.
 */
class backup_migrate_schedule extends backup_migrate_item {
  var $db_table = "backup_migrate_schedules";
  var $type_name = 'schedule';
  var $singular = 'schedule';
  var $plural = 'schedules';
  var $default_values = array();

  /**
   * This function is not supposed to be called. It is just here to help the po extractor out.
   */
  function strings() {
    // Help the pot extractor find these strings.
    t('Schedule');
    t('Schedules');
    t('schedule');
    t('schedules');
  }

  /**
   * Get the default values for this item.
   */
  function get_default_values() {
    return array(
        'name' => t("Untitled Schedule"),
        'source_id' => 'db',
        'enabled' => 1,
        'keep' => 0,
        'period' => 60 * 60 * 24,
        'storage' => BACKUP_MIGRATE_STORAGE_NONE
      );
  }

  /**
   * Get the columns needed to list the type.
   */  
  function get_list_column_info() {
    $out = parent::get_list_column_info();
    $out = array(
      'name'                  => array('title' => t('Name')),
      'destination_name'      => array('title' => t('Destination'), 'html' => TRUE),
      'profile_name'          => array('title' => t('Profile'), 'html' => TRUE),
      'frequency_description' => array('title' => t('Frequency')),
      'keep_description'      => array('title' => t('Keep')),
      'enabled_description'   => array('title' => t('Enabled')),
      'last_run_description'  => array('title' => t('Last run')),
    ) + $out;
    return $out;
  }

  /**
   * Get a row of data to be used in a list of items of this type.
   */
  function get_list_row() {
    drupal_add_css(drupal_get_path('module', 'backup_migrate') .'/backup_migrate.css');
    $row = parent::get_list_row();
    if (!$this->is_enabled()) {
      foreach ($row as $key => $field) {
        $row[$key] = array('data' => $field, 'class' => 'schedule-list-disabled');
      }
    }
    return $row;
  }

  /**
   * Is the schedule enabled and valid.
   */
  function is_enabled() {
    $destination = $this->get_destination();
    $profile = $this->get_profile();
    return (!empty($this->enabled) && !empty($destination) && !empty($profile));
  }

  /**
   * Get the destination object of the schedule.
   */
  function get_destination() {
    backup_migrate_include('destinations');
    return backup_migrate_get_destination($this->get('destination_id'));
  }

  /**
   * Get the name of the destination.
   */
  function get_destination_name() {
    if ($destination = $this->get_destination()) {
      return check_plain($destination->get_name());
    }
    return '<div class="row-error">'. t("Missing") .'</div>';
  }

  /**
   * Get the destination of the schedule.
   */
  function get_profile() {
    backup_migrate_include('profiles');
    return backup_migrate_get_profile($this->get('profile_id'));
  }

  /**
   * Get the name of the source.
   */
  function get_profile_name() {
    if ($profile = $this->get_profile()) {
      return check_plain($profile->get_name());
    }
    return '<div class="row-error">'. t("Missing") .'</div>';
  }

  /**
   * Format a frequency in human-readable form.
   */
  function get_frequency_description() {
    $period = $this->get_frequency_period();
    $out = format_plural(($this->period / $period['seconds']), $period['singular'], $period['plural']);
    return $out;
  }

  /**
   * Format the number to keep in human-readable form.
   */
  function get_keep_description() {
    return !empty($this->keep) ? $this->keep : t('All');
  }

  /**
   * Format the enabled status in human-readable form.
   */
  function get_enabled_description() {
    return !empty($this->enabled) ? t('Enabled') : t('Disabled');
  }

  /**
   * Format the enabled status in human-readable form.
   */
  function get_last_run_description() {
    return !empty($this->last_run) ? format_date($this->last_run, 'small') : t('Never');
  }

  /**
   * Get the number of excluded tables.
   */
  function get_exclude_tables_count() {
    return count($this->exclude_tables) ? count($this->exclude_tables) : t("No tables excluded");
  }

  /**
   * Get the number of excluded tables.
   */
  function get_nodata_tables_count() {
    return count($this->nodata_tables) ? count($this->nodata_tables) : t("No data omitted");
  }

  /**
   * Get the edit form.
   */
  function edit_form() {
    $form = parent::edit_form();
    backup_migrate_include('destinations', 'profiles');

    $form['enabled'] = array(
      "#type" => "checkbox",
      "#title" => t("Enabled"),
      "#default_value" => $this->get('enabled'),
    );
    $form['name'] = array(
      "#type" => "textfield",
      "#title" => t("Schedule Name"),
      "#default_value" => $this->get('name'),
    );

    $form += _backup_migrate_get_source_form($this->get('source_id'));

    $form['profile_id'] = array(
      "#type" => "select",
      "#title" => t("Settings Profile"),
      "#options" => _backup_migrate_get_profile_form_item_options(),
      "#default_value" => $this->get('profile_id'),
    );
    $form['profile_id']['#description'] = ' '. l(t("Create new profile"), BACKUP_MIGRATE_MENU_PATH . "/profile/add");
    if (!$form['profile_id']['#options']) {
      $form['profile_id']['#options'] = array('0' => t('-- None Available --'));
    }

    $period_options = array();
    foreach ($this->frequency_periods() as $type => $period) {
      $period_options[$type] = $period['title'];
    }
    $default_period     = $this->get_frequency_period();
    $default_period_num = $this->get('period') / $default_period['seconds'];

    $form['period']     = array(
      "#type" => "item",
      "#title" => t("Backup every"),
      "#prefix" => '<div class="container-inline">',
      "#suffix" => '</div>',
      "#tree" => TRUE,
    );
    $form['period']['number'] = array(
      "#type" => "textfield",
      "#size" => 6,
      "#default_value" => $default_period_num,
    );
    $form['period']['type'] = array(
      "#type" => "select",
      "#options" => $period_options,
      "#default_value" => $default_period['type'],
    );

    $form['keep'] = array(
      "#type" => "textfield",
      "#size" => 6,
      "#title" => t("Number of Backup files to keep"),
      "#description" => t("The number of backup files to keep before deleting old ones. Use 0 to never delete backups. <strong>Other files in the destination directory will get deleted if you specify a limit.</strong>"),
      "#default_value" => $this->get('keep'),
    );
    $destination_options = _backup_migrate_get_destination_form_item_options('scheduled backup');
    $form['destination_id'] = array(
      "#type" => "select",
      "#title" => t("Destination"),
      "#description" => t("Choose where the backup file will be saved. Backup files contain sensitive data, so be careful where you save them."),
      "#options" => $destination_options,
      "#default_value" => $this->get('destination_id'),
    );
    $form['destination_id']['#description'] .= ' '. l(t("Create new destination"), BACKUP_MIGRATE_MENU_PATH . "/destination/add");

    return $form;
  }

  /**
   * Submit the edit form.
   */
  function edit_form_validate($form, &$form_state) {
    if (!is_numeric($form_state['values']['period']['number']) || $form_state['values']['period']['number'] <= 0) {
      form_set_error('period][number', t('Backup period must be a number greater than 0.'));
    }
    if (!is_numeric($form_state['values']['keep']) || $form_state['values']['keep'] < 0) {
      form_set_error('keep', t('Number to keep must be an integer greater than or equal to 0.'));
    }
    parent::edit_form_validate($form, $form_state);
  }

  /**
   * Submit the edit form.
   */
  function edit_form_submit($form, &$form_state) {
    $periods = $this->frequency_periods();
    $period = $periods[$form_state['values']['period']['type']];
    $form_state['values']['period'] = $form_state['values']['period']['number'] * $period['seconds'];
    parent::edit_form_submit($form, $form_state);
  }

  /**
   * Get the period of the frequency (ie: seconds, minutes etc.)
   */
  function get_frequency_period() {
    foreach (array_reverse($this->frequency_periods()) as $period) {
      if ($period['seconds'] && ($this->period % $period['seconds']) === 0) {
        return $period;
      }
    }
  }

  /**
   * Get a list of available backup periods. Only returns time periods which have a
   *  (reasonably) consistent number of seconds (ie: no months).
   */
  function frequency_periods() {
    return array(
      'seconds' => array('type' => 'seconds', 'seconds' => 1, 'title' => t('Seconds'), 'singular' => t('Once a second'), 'plural' => t('Every @count seconds')),
      'minutes' => array('type' => 'minutes', 'seconds' => 60, 'title' => t('Minutes'), 'singular' => t('Once a minute'), 'plural' => t('Every @count minutes')),
      'hours' => array('type' => 'hours', 'seconds' => 3600, 'title' => t('Hours'), 'singular' => t('Once an hour'), 'plural' => t('Every @count hours')),
      'days' => array('type' => 'days', 'seconds' => 86400, 'title' => t('Days'), 'singular' => t('Once a day'), 'plural' => t('Every @count days')),
      'weeks' => array('type' => 'weeks', 'seconds' => 604800, 'title' => t('Weeks'), 'singular' => t('Once a week'), 'plural' => t('Every @count weeks')),
    );
  }

  /**
   * Get the message to send to the user when confirming the deletion of the item.
   */
  function delete_confirm_message() {
    return t('Are you sure you want to delete the profile %name? Any schedules using this profile will be disabled.', array('%name' => $this->get('name')));
  }

  /**
   * Perform the cron action. Run the backup if enough time has elapsed.
   */
  function cron() {
    $now = time();

    // Add a small negative buffer (1% of the entire period) to the time to account for slight difference in cron run length.
    $wait_time = $this->period - ($this->period * variable_get('backup_migrate_schedule_buffer', 0.01));

    if ($this->is_enabled() && ($now - $this->last_run) >= $wait_time) {
      if ($settings = $this->get_profile()) {
        $settings->destination_id = $this->destination_id;
        $settings->source_id = $this->source_id;
        backup_migrate_perform_backup($settings);
        $this->update_last_run($now);
        $this->remove_expired_backups();
      }
      else {
        backup_migrate_backup_fail("Schedule '%schedule' could not be run because requires a profile which is missing.", array('%schedule' => $schedule->get_name()), $settings);
      }
    }
  }

  /**
   * Set the last run time of a schedule to the given timestamp, or now if none specified.
   */
  function update_last_run($timestamp = NULL) {
    if ($timestamp === NULL) {
      $timestamp = time();
    }
    db_query("UPDATE {backup_migrate_schedules}
                 SET last_run = %d
               WHERE schedule_id = '%s'",
      $timestamp,
      $this->get_id()
    );
  }

  /**
   * Remove older backups keeping only the number specified by the aministrator.
   */
  function remove_expired_backups() {
    backup_migrate_include('destinations');

    $num_to_keep = $this->keep;
    // If num to keep is not 0 (0 is infinity).
    if ($num_to_keep && ($destination = $this->get_destination())) {
      $i = 0;
      if ($destination->op('delete') && $destination_files = $destination->list_files()) {
        // Sort the files by modified time.
        foreach ($destination_files as $file) {
          if ($file->is_recognized_type() && $destination->can_delete_file($file->file_id())) {
            $files[str_pad($file->info('filetime'), 10, "0", STR_PAD_LEFT) ."-". $i++] = $file;
          }
        }
  
        // If we are beyond our limit, remove as many as we need.
        $num_files = count($files);
  
        if ($num_files > $num_to_keep) {
          $num_to_delete = $num_files - $num_to_keep;
          // Sort by date.
          ksort($files);
          // Delete from the start of the list (earliest).
          for ($i = 0; $i < $num_to_delete; $i++) {
            $file = array_shift($files);
            $destination->delete_file($file->file_id());
          }
        }
      }
    }
  }
}
