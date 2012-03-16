<?php


/**
 * @file
 * This filter performs tha actual backup or restore operation. Not technically a filter per-se, but it does need to fit in the call chain.
 */

/**
 * A filter backup or migrate the specified source.
 *
 * @ingroup backup_migrate_filters
 */
class backup_migrate_filter_backup_restore extends backup_migrate_filter {
  var $op_weights = array('backup' => 0, 'restore' => 0);

  /**
   * Get the default destinations for this filter.
   */
  function destinations() {
    $out = array();
    foreach ($this->_get_destination_types() as $destination) {
      if (method_exists($destination, 'destinations')) {
        $out += $destination->destinations();
      }
    }
    return $out;
  }


  /**
   * Get the default backup settings for this filter.
   */
  function backup_settings_default() {
    backup_migrate_include('destinations');
    $out = array();
    foreach (backup_migrate_get_destinations('source') as $destination) {
      $out['destinations'][$destination->get_id()] = $destination->backup_settings_default();
    }
    return $out;
  }

  /**
   * Get the form for the settings for this filter.
   */
  function backup_settings_form_validate($form, &$form_state) {
    foreach ($this->_get_destination_types() as $destination) {
      $destination->backup_settings_form_validate($form, $form_state);
    }
  }

  /**
   * Submit the settings form. Any values returned will be saved.
   */
  function backup_settings_form_submit($form, $form_state) {
    foreach ($this->_get_destination_types() as $destination) {
      $destination->backup_settings_form_submit($form, $form_state);
    }
  }

  /**
   * Get the default restore settings for this filter.
   */
  function restore_settings_default() {
    $out = array();
    foreach ($this->_get_destination_types() as $destination) {
      $out += $destination->restore_settings_default();
    }
    return $out;
  }

  /**
   * Get the form for the backup settings for this filter.
   */
  function backup_settings_form($settings) {
    backup_migrate_include('destinations');
    $out = array('destinations' => array(
      '#tree' => TRUE,
      
    ));
    foreach (backup_migrate_get_destinations('source') as $destination) {
      $destination_settings = (array)(@$settings['destinations'][$destination->get_id()]) + $settings;
      if ($form = $destination->backup_settings_form($destination_settings)) {
        $out['destinations'][$destination->get_id()] = array(
          '#type' => 'fieldset',
          '#title' => t('!name Backup Options', array('!name' => $destination->get('name'))),
          "#collapsible" => TRUE,
          "#collapsed" => TRUE,
          '#tree' => TRUE,
          '#parents' => array('filters', 'destinations', $destination->get_id()),
        ) + $form;
      }
    }
    return $out;
  }

  /**
   * Get the form for the restore settings for this filter.
   */
  function restore_settings_form($settings) {
    $form = array();
    foreach ($this->_get_destination_types() as $destination) {
      $destination->restore_settings_form($form, $settings);
    }
    return $form;
  }

  /**
   * Get the file types supported by this destination.
   */
  function file_types() {
    $types = array();
    foreach ($this->_get_destination_types() as $destination) {
      $types += $destination->file_types();
    }
    return $types;
  }

  /**
   * Backup the data from the source specified in the settings.
   */
  function backup($file, &$settings) {
    if ($source = $settings->get_source()) {
      if (!empty($settings->filters['destinations'][$source->get_id()])) {
        $settings->filters = (array)($settings->filters['destinations'][$source->get_id()]) + $settings->filters;
      }
      $file = $source->backup_to_file($file, $settings);
      return $file;
    }
    backup_migrate_backup_fail("Could not run backup because the source '%source' is missing.", array('%source' => $settings->source_id), $settings);
    return FALSE;
  }

  /**
   * Restore the data from to source specified in the settings.
   */
  function restore($file, &$settings) {
    if ($source = $settings->get_source()) {
      if (!empty($settings->filters['destinations'][$source->get_id()])) {
        $settings->filters = (array)($settings->filters['destinations'][$source->get_id()]) + $settings->filters;
      }
      $num = $source->restore_from_file($file, $settings);
      return $num ? $file : FALSE;
    }
    backup_migrate_restore_fail("Could not run restore because the source '%source' is missing.", array('%source' => $settings->source_id), $settings);
    return FALSE;
  }

  /**
   * Get a list of dummy destinations representing each of the available destination types.
   */
  function _get_destination_types() {
    backup_migrate_include('destinations');
    static $destinations = NULL;
    if (!is_array($destinations)) {
      $destinations = array();
      $types = backup_migrate_get_destination_types();
      // If no (valid) node type has been provided, display a node type overview.
      foreach ($types as $key => $type) {
        // Include the necessary file if specified by the type.
        if (!empty($type['file'])) {
          require_once './'. $type['file'];
        }
        $destinations[] = new $type['class'](array());
      }
    }
    return $destinations;
  }
}
