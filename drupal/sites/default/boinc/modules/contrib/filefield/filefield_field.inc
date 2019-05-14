<?php

/**
 * @file
 * FileField CCK field hooks and callbacks.
 */

/**
 * Implementation of CCK's hook_field_settings($op = 'form').
 */
function filefield_field_settings_form($field) {
  drupal_add_js(drupal_get_path('module', 'filefield') .'/filefield.js');

  $form = array();

  $form['list_field'] = array(
    '#type' => 'radios',
    '#title' => t('List field'),
    '#options' => array(0 => t('Disabled'), 1 => t('Enabled')),
    '#default_value' => $field['list_field'] === '' ? 0 : (int) $field['list_field'],
    '#description' => t('Display a checkbox where users may choose if a file should be shown when viewing the content. Most formatters other than <em>Generic file</em> do not support this option.'),
    '#attributes' => array('class' => 'filefield-list-field'),
  );
  $form['list_default'] = array(
    '#type' => 'checkbox',
    '#title' => t('Files listed by default'),
    '#default_value' => $field['list_default'] === '' ? 1 : (int) $field['list_default'],
  );
  $form['description_field'] = array(
    '#type' => 'radios',
    '#title' => t('Description field'),
    '#default_value' => $field['description_field'] === '' ? 0 : (int) $field['description_field'],
    '#options' => array(0 => t('Disabled'), 1 => t('Enabled')),
    '#description' => t('Display a text field where users may enter a description about the uploaded file. The description text is used when using the <em>Generic file</em> formatter to link to the file, the file name will be used otherwise. Most other formatters do not use the description field on output.'),
  );

  return $form;
}

/**
 * Implementation of CCK's hook_field_settings($op = 'save').
 */
function filefield_field_settings_save($field) {
  return array('list_field', 'list_default', 'description_field');
}

/**
 * Implementation of CCK's hook_field_settings($op = 'database_columns').
 */
function filefield_field_settings_database_columns($field) {
  return array(
    'fid' => array('type' => 'int', 'not null' => FALSE, 'views' => TRUE),
    'list' => array('type' => 'int', 'size' => 'tiny', 'not null' => FALSE, 'views' => TRUE),
    'data' => array('type' => 'text', 'serialize' => TRUE, 'views' => TRUE),
  );
}

/**
 * Implementation of CCK's hook_field_settings($op = 'views_data').
 */
function filefield_field_settings_views_data($field) {
  $data = content_views_field_views_data($field);
  $db_info = content_database_info($field);
  $table_alias = content_views_tablename($field);

  // By defining the relationship, we already have a "Has file" filter
  // plus all the filters that Views already provides for files.
  // No need for having a filter by ourselves.
  unset($data[$table_alias][$field['field_name'] .'_fid']['filter']);

  // Add a relationship for related file.
  $data[$table_alias][$field['field_name'] .'_fid']['relationship'] = array(
    'base' => 'files',
    'field' => $db_info['columns']['fid']['column'],
    'handler' => 'content_handler_relationship',
    'label' => t($field['widget']['label']),
    'content_field_name' => $field['field_name'],
    'skip base' => array('files'),
  );

  // Use the Views boolean handler for filtering the list value.
  $data[$table_alias][$field['field_name'] .'_list']['filter']['handler'] = 'views_handler_filter_boolean_operator';

  // Add our special handler for serialized data.
  $data[$table_alias][$field['field_name'] .'_data']['field'] = array(
    'title' => $data[$table_alias][$field['field_name'] .'_data']['title'],
    'title short' => $data[$table_alias][$field['field_name'] .'_data']['title short'],
    'help' => t('Can be used to display specific alt, title, or description information. May cause duplicate rows if grouping fields.'),
    'field' => $db_info['columns']['data']['column'],
    'table' => $db_info['table'],
    'handler' => 'filefield_handler_field_data',
    'click sortable' => FALSE,
    'access callback' => 'content_access',
    'access arguments' => array('view', $field),
  );

  // Remove handlers that are probably unnecessary.
  unset($data[$table_alias][$field['field_name'] .'_data']['filter']);
  unset($data[$table_alias][$field['field_name'] .'_data']['argument']);
  unset($data[$table_alias][$field['field_name'] .'_list']['argument']);

  // Set up relationship with file views.
  $data[$table_alias]['table']['join']['files'] = array(
    'table' => $db_info['table'],
    'left_field' => 'fid',
    'field' => $db_info['columns']['fid']['column'],
  );
  $data[$table_alias]['vid'] = array(
    'title' => t($field['widget']['label']),
    'help' => t('The node the uploaded file is attached to'),
    'relationship' => array(
      'label' => t($field['widget']['label']),
      'base' => 'node',
      'base field' => 'vid',
      // This allows us to not show this relationship if the base is already
      // node so users won't create circular relationships.
      'skip base' => array('node', 'node_revisions'),
    ),
  );

  return $data;
}

/**
 * Implementation of CCK's hook_field($op = 'load').
 */
function filefield_field_load($node, $field, &$items, $teaser, $page) {
  if (empty($items)) {
    return array();
  }
  foreach ($items as $delta => $item) {
    // Despite hook_content_is_empty(), CCK still doesn't filter out
    // empty items from $op = 'load', so we need to do that ourselves.
    if (empty($item['fid']) || !($file = field_file_load($item['fid']))) {
      $items[$delta] = NULL;
    }
    else {
      if (isset($item['data']) && !empty($item['data'])) {
        $item['data'] = unserialize($item['data']);
      }
      // Temporary fix to unserialize data serialized multiple times.
      // See the FileField issue http://drupal.org/node/402860.
      // And the CCK issue http://drupal.org/node/407446.
      while (!empty($item['data']) && is_string($item['data'])) {
        $item['data'] = unserialize($item['data']);
      }
      // Merge any data added by modules in hook_file_load().
      if (isset($file['data']) && isset($item['data'])) {
        $file['data'] = array_merge((array) $file['data'], (array) $item['data']);
      }
      $items[$delta] = array_merge($file, $item);
    }
  }

  return array($field['field_name'] => $items);
}

/**
 * Implementation of CCK's hook_field($op = 'insert').
 */
function filefield_field_insert($node, $field, &$items, $teaser, $page) {
  return filefield_field_update($node, $field, $items, $teaser, $page);
}

/**
 * Implementation of CCK's hook_field($op = 'update').
 */
function filefield_field_update($node, $field, &$items, $teaser, $page) {

  // Accumulator to gather current fid to compare with the original node
  // for deleting replaced files.
  $curfids = array();
  foreach ($items as $delta => $item) {
    $items[$delta] = field_file_save($node, $item);
    // Remove items from the array if they have been deleted.
    if (empty($items[$delta]) || empty($items[$delta]['fid'])) {
      $items[$delta] = NULL;
    }
    else {
      $curfids[] = $items[$delta]['fid'];
    }
  }

  // If this is a new node there are no old items to worry about.
  // On new revisions, old files are always maintained in the previous revision.
  if ($node->is_new || !empty($node->revision) || !empty($node->skip_filefield_delete)) {
    return;
  } 

  // Delete items from original node.
  $orig = node_load($node->nid); 
  // If there are, figure out which ones must go.
  if (!empty($orig->{$field['field_name']})) {
    foreach ($orig->{$field['field_name']} as $oitem) {
      if (isset($oitem['fid']) && !in_array($oitem['fid'], $curfids)) {
        // For hook_file_references, remember that this is being deleted.
        $oitem['field_name'] = $field['field_name'];
        $oitem['delete_vid'] = $orig->vid;
        filefield_field_delete_file($oitem, $field); 
      }
    }
  }
}

/**
 * Implementation of CCK's hook_field($op = 'delete_revision').
 */
function filefield_field_delete_revision($node, $field, &$items, $teaser, $page) {
  foreach ($items as $delta => $item) {
    if (isset($item['fid'])) {
      // For hook_file_references, remember that this is being deleted.
      $item['field_name'] = $field['field_name'];
      $item['delete_vid'] = $node->vid;
      if (filefield_field_delete_file($item, $field)) {
        $items[$delta] = NULL;
      }
    }
  }
}

/**
 * Implementation of CCK's hook_field($op = 'delete').
 */
function filefield_field_delete($node, $field, &$items, $teaser, $page) {
  foreach ($items as $delta => $item) {
    if (isset($item['fid'])) {
      // For hook_file_references(), remember that this is being deleted.
      $item['field_name'] = $field['field_name'];
      // Pass in the nid of the node that is being removed so all references can
      // be counted in hook_file_references().
      $item['delete_nid'] = $node->nid;
      filefield_field_delete_file($item, $field);
    }
  }

  // Delete all the remaining items present only in older revisions.
  $db_info = content_database_info($field);
  $result = db_query('SELECT vid, f.* FROM {' . $db_info['table'] . '} t INNER JOIN {files} f ON t.' . $db_info['columns']['fid']['column'] . ' = f.fid WHERE nid = %d AND vid != %d', $node->nid, $node->vid);
  while ($item = db_fetch_array($result)) {
    if (isset($item['fid'])) {
      $item['field_name'] = $field['field_name'];
      $item['delete_vid'] = $item['vid'];
      filefield_field_delete_file($item, $field); 
    }
  }
}

/**
 * Check that FileField controls a file before attempting to deleting it.
 */
function filefield_field_delete_file($file, $field) {
  $file = (object) $file;

  // Remove the field_name and delete_nid properties so that references can be
  // counted including the files to be deleted.
  $field_name = isset($file->field_name) ? $file->field_name : NULL;
  $delete_nid = isset($file->delete_nid) ? $file->delete_nid : NULL;
  unset($file->field_name, $file->delete_nid);

  // To prevent FileField from deleting files it doesn't know about, check the
  // FileField reference count. Temporary files can be deleted because they
  // are not yet associated with any content at all.
  if ($file->status == 0 || filefield_get_file_reference_count($file, $field) > 0) {
    $file->field_name = $field_name;
    $file->delete_nid = $delete_nid;
    return field_file_delete($file);
  }

  // Even if the file is not deleted, return TRUE to indicate the FileField
  // record can be removed from the FileField database tables.
  return TRUE;
}

/**
 * Implementation of CCK's hook_field($op = 'sanitize').
 */
function filefield_field_sanitize($node, $field, &$items, $teaser, $page) {
  foreach ($items as $delta => $item) {
    // Cleanup $items during node preview.
    if (empty($item['fid']) || !empty($item['delete'])) {
      // Check for default images at the widget level.
      // TODO: Provide an API to ImageField to do this itself?
      if (!empty($field['widget']['use_default_image']) && !empty($field['widget']['default_image']['filepath']) && $delta == 0) {
        $items[$delta] = $field['widget']['default_image'];
        $items[$delta]['default'] = TRUE;
      }
      else {
        $items[$delta] = NULL;
        continue;
      }
    }

    // Add nid so formatters can create a link to the node.
    $items[$delta]['nid'] = isset($node->nid) ? $node->nid : NULL;

    // Get the 'data' column stored by CCK into an array. This is necessary
    // for Views, which doesn't call the "load" $op and to fix an issue with
    // CCK double-serializing data.
    // See the FileField issue http://drupal.org/node/402860.
    // And the CCK issue http://drupal.org/node/407446.
    while (!empty($items[$delta]['data']) && is_string($items[$delta]['data'])) {
      $items[$delta]['data'] = unserialize($items[$delta]['data']);
    }

    // Load the complete file if a filepath is not available.
    if (!empty($item['fid']) && empty($item['filepath'])) {
      $file = (array) field_file_load($item['fid']);
      if (isset($file['data']) && isset($items[$delta]['data'])) {
        $file['data'] = array_merge($file['data'], $items[$delta]['data']);
      }
      $items[$delta] = array_merge($file, $items[$delta]);
    }

    // Verify the file exists on the server.
    if (!empty($item['filepath']) && !file_exists($item['filepath'])) {
      watchdog('filefield', 'FileField was trying to display the file %file, but it does not exist.', array('%file' => $item['filepath']), WATCHDOG_WARNING);
    }
  }
}
