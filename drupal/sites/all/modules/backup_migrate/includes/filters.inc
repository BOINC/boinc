<?php


/**
 * @file
 * All of the filter handling code needed for Backup and Migrate.
 */

/**
 * Get the available destination types.
 */
function backup_migrate_get_filters($op = NULL) {
  static $filters = NULL;
  if ($filters === NULL) {
    $filters = array();
    $definitions = module_invoke_all('backup_migrate_filters');
    foreach ($definitions as $definition) {
      // Include the necessary file if specified by the filter.
      if (!empty($definition['file'])) {
        require_once './'. $definition['file'];
      }
      $filters[] = new $definition['class'];
    }
  }
  $sort = array();
  // Sort the filters based on the weight for the given operation.
  foreach ($filters as $filter) {
    $sort[] = $filter->weight($op);
  }
  array_multisort($sort, SORT_ASC, SORT_NUMERIC, $filters);
  return $filters;
}

/**
 * Implementation of hook_backup_migrate_filters().
 *
 * Get the built in Backup and Migrate filters.
 */
function backup_migrate_backup_migrate_filters() {
  return array(
    'backup_restore' => array(
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/filters.backup_restore.inc',
      'class' => 'backup_migrate_filter_backup_restore',
    ),
    'compression' => array(
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/filters.compression.inc',
      'class' => 'backup_migrate_filter_compression',
    ),
    'encryption' => array(
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/filters.encryption.inc',
      'class' => 'backup_migrate_filter_encryption',
    ),
    'statusnotify' => array(
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/filters.statusnotify.inc',
      'class' => 'backup_migrate_filter_statusnotify',
    ),
    'utils' => array(
      'file' => drupal_get_path('module', 'backup_migrate') .'/includes/filters.utils.inc',
      'class' => 'backup_migrate_filter_utils',
    ),
  );
}

/**
 * Invoke the given method on all of the available filters.
 */
function backup_migrate_filters_invoke_all() {
  $args    = func_get_args();
  $op      = array_shift($args);
  $out     = array();
  $filters = backup_migrate_get_filters($op);
  foreach ($filters as $filter) {
    if (method_exists($filter, $op)) {
      /* call_user_func_array() ignores the function signature, so we cannot
       * use it to pass references. (Call-time pass-by-reference is deprecated
       * in PHP5.3.) Work around it, since we have unknown function signatures.
       */
      switch (count($args)) {
        case 1:
          $ret = $filter->$op($args[0]);
          break;

        case 2:
          $ret = $filter->$op($args[0], $args[1]);
          break;

        default:
          // This assumes that no functions with more than 2 arguments expect a
          // reference as argument. If so, add another 'case block'.
          $ret = call_user_func_array(array($filter, $op), $args);
      }
      $out = array_merge_recursive($out, (array) $ret);
    }
  }
  return $out;
}

/**
 * Filter a backup file before sending it to the destination.
 */
function backup_migrate_filters_backup($file, &$settings) {
  $filters = backup_migrate_get_filters('backup');
  foreach ($filters as $filter) {
    if ($file) {
      $file = $filter->backup($file, $settings);
    }
  }
  return $file;
}

/**
 * Filter a backup file before sending it to the destination.
 */
function backup_migrate_filters_restore($file, &$settings) {
  $filters = backup_migrate_get_filters('restore');
  foreach ($filters as $filter) {
    if ($file) {
      $file = $filter->restore($file, $settings);
    }
  }
  return $file;
}

/**
 * Get the backup settings for all of the filters.
 */
function backup_migrate_filters_settings_form($settings, $op) {
  $out = backup_migrate_filters_invoke_all($op .'_settings_form', $settings);
  $out = backup_migrate_filters_settings_form_set_parents($out);
  return $out;
}

/**
 * Add a form parent to the filter settings so that the filter values are saved in the right table.
 */
function backup_migrate_filters_settings_form_set_parents($form) {
  foreach (element_children($form) as $key) {
    if (!isset($form[$key]['#parents'])) {
      $form[$key]['#parents'] = array('filters', $key);
      $form[$key] = backup_migrate_filters_settings_form_set_parents($form[$key]);
    }
  }
  return $form;
}


/**
 * Validate all the filters.
 */
function backup_migrate_filters_settings_form_validate($op, $form, &$form_state) {
  backup_migrate_filters_invoke_all($op .'_settings_form_validate', $form, $form_state);
}

/**
 * Submit all of the filters.
 */
function backup_migrate_filters_settings_form_submit($op, $form, &$form_state) {
  backup_migrate_filters_invoke_all($op .'_settings_form_submit', $form, $form_state);
}


/**
 * Get the default settings for the filters.
 */
function backup_migrate_filters_settings_default($op) {
  return backup_migrate_filters_invoke_all($op .'_settings_default');
}

/**
 * Get the file types for all of the filters.
 */
function backup_migrate_filters_file_types() {
  return backup_migrate_filters_invoke_all('file_types');
}

/**
 * A base class for basing filters on.
 */
class backup_migrate_filter {
  var $weight = 0;
  var $op_weights = array();

  /**
   * Get the weight of the filter for the given op.
   */
  function weight($op = NULL) {
    if ($op && isset($this->op_weights[$op])) {
      return $this->op_weights[$op];
    }
    return $this->weight;
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

  /**
   * Get a list of file types handled by this filter.
   */
  function file_types() {
    return array();
  }

  /**
   * Declare any default destinations for this filter.
   */
  function destinations() {
    return array();
  }


  /**
   * This function is called on a backup file after the backup has been completed.
   */
  function backup($file, $settings) {
    return $file;
  }

  /**
   * This function is called on a backup file before importing it.
   */
  function restore($file, $settings) {
    return $file;
  }

  /**
   * This function is called immediately prior to backup.
   */
  function pre_backup($source, $file, $settings) {

  }

  /**
   * This function is called immediately post backup.
   */
  function post_backup($source, $file, $settings) {

  }
}

