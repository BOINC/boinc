<?php

/**
 * @file
 * Handles ajax file operations.
 */

/**
 * Ajax operation: navigate
 */
function imce_js_navigate(&$imce) {
  return array(
    'files' => theme('imce_file_list', array('imce' => &$imce)),
    'dirsize' => format_size($imce['dirsize']),
    'subdirectories' => array_map('rawurlencode', $imce['subdirectories']),
    'perm' => $imce['perm']
  );
}

/**
 * Ajax operation: upload
 */
function imce_js_upload(&$imce) {
  if ($imce['perm']['upload']) {
    $_POST['op'] = t('Upload');
    drupal_get_form('imce_upload_form', array('imce' => &$imce));
    return array('added' => isset($imce['added']) ? $imce['added'] : NULL, 'dirsize' => format_size($imce['dirsize']));
  }
}

/**
 * Ajax operation: thumbnails
 */
function imce_js_thumb(&$imce) {
  if ($imce['perm']['thumb']) {
    $_POST['op'] = t('Create thumbnails');
    return imce_process_fileop($imce);
  }
}

/**
 * Ajax operation: delete
 */
function imce_js_delete(&$imce) {
  if ($imce['perm']['delete']) {
    $_POST['op'] = t('Delete');
    return imce_process_fileop($imce);
  }
}

/**
 * Ajax operation: resize
 */
function imce_js_resize(&$imce) {
  if ($imce['perm']['resize']) {
    $_POST['op'] = t('Resize');
    return imce_process_fileop($imce);
  }
}

/**
 * Process file operations form
 */
function imce_process_fileop(&$imce) {
  drupal_get_form('imce_fileop_form', array('imce' => &$imce));
  return array('added' => isset($imce['added']) ? $imce['added'] : NULL, 'removed' => isset($imce['removed']) ? $imce['removed'] : NULL, 'dirsize' => format_size($imce['dirsize']));
}