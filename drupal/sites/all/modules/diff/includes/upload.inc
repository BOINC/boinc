<?php

/**
 * @file
 * Implementation of hook_diff() for file attachments.
 */

/**
 * Implementation of hook_diff() for file attachments.
 */
function upload_diff($old_node, $new_node) {
  $result = array();
  $old_files = array();
  if (isset($old_node->files)) {
    foreach ($old_node->files as $file) {
      $old_files[] = $file->filename;
    }
  }
  $new_files = array();
  if (isset($new_node->files)) {
    foreach ($new_node->files as $key => $file) {
      if (is_array($file)) {
        // During editing the files are stored as arrays, not objects.
        if ($file['remove']) {
          // It looks better if a blank line is inserted for removed files.
          $new_files[] = '';
        }
        else {
          $new_files[] = $file['filename'];
        }
      }
      else {
        $new_files[] = $file->filename;
      }
    }
  }
  $result['attachments'] = array(
    '#name' => t('Attachments'),
    '#old' => $old_files,
    '#new' => $new_files,
    '#weight' => 30,
    '#format' => array(
      'show_header' => FALSE,
    )
  );
  return $result;
}
