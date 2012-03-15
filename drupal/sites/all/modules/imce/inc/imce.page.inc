<?php

/**
 * @file
 * Implements the file browser.
 */

/**
 * q = imce.
 */
function imce() {
  module_invoke('admin_menu', 'suppress');//suppress admin_menu
  $jsop = isset($_GET['jsop']) ? $_GET['jsop'] : NULL;
  print imce_page($GLOBALS['user'], $jsop);
  exit();
}

/**
 * q = user/x/imce.
 */
function imce_user_page($account) {
  return theme('imce_user_page', $account);
}

/**
 * Returns the imce page for the specified user.
 */
function imce_page($user, $jsop = NULL) {
  return theme('imce_page', imce_content($user, $jsop));
}

/**
 * Returns the content of the file browser.
 */
function imce_content($user, $jsop = NULL) {

  //execute ajax calls.
  if ($jsop) {
    return imce_js($user, $jsop);
  }

  //initiate configuration profile
  if (!$imce = imce_initiate_profile($user)) {
    return '';
  }
  imce_process_profile($imce);//get active directory content

  //Before creating the content let's add main files required for imce to function properly.
  $path = drupal_get_path('module', 'imce');
  drupal_add_js($path . '/js/jquery.form.js');
  drupal_add_js($path . '/js/imce.js');
  drupal_add_js($path . '/js/imce_extras.js');
  drupal_add_css($path . '/css/imce-content.css');

  //process forms.
  //reference imce inside an array so it will stay referenced during argument copy of drupal_get_form
  $imce_ref = array('imce' => &$imce);
  $forms = '';

  if (!$imce['error']) {
    //process file upload.
    if (imce_perm_exists($imce, 'upload')) {
      $forms .= drupal_get_form('imce_upload_form', $imce_ref);
    }
    //process file operations.
    $forms .= drupal_get_form('imce_fileop_form', $imce_ref);
  }

  //run custom content functions. possible to insert extra forms. content is invisible when js is enabled.
  foreach (variable_get('imce_custom_content', array()) as $func => $state) {
    if ($state && function_exists($func) && $output = $func($imce)) {
      $forms .= $output;
    }
  }

  $content = theme('imce_content', imce_create_tree($imce), $forms, $imce_ref);

  //make necessary changes for js conversion
  $imce['dir'] = str_replace('%2F', '/', rawurlencode($imce['dir']));
  unset($imce['files'], $imce['name'], $imce['directories'], $imce['subdirectories'], $imce['filesize'], $imce['quota'], $imce['tuquota'], $imce['thumbnails'], $imce['uid'], $imce['usertab']);

  drupal_add_js($imce_ref, 'setting');

  return $content;
}

/**
 * Ajax operations. q=imce&jsop={op}
 */
function imce_js($user, $jsop = '') {
  $response = array();

  //data
  if ($imce = imce_initiate_profile($user)) {
    imce_process_profile($imce);
    if (!$imce['error']) {
      module_load_include('inc', 'imce', 'inc/imce.js');
      if (function_exists($func = 'imce_js_'. $jsop)) {
        $response['data'] = $func($imce);
      }
      // Allow alteration of response.
      foreach (variable_get('imce_custom_response', array()) as $func => $state) {
        if ($state && function_exists($func)) {
          $func($jsop, $response, $imce, $user);
        }
      }
    }
  }
  //messages
  $response['messages'] = drupal_get_messages();

  //disable devel log.
  $GLOBALS['devel_shutdown'] = FALSE;
  //for upload we must return plain text header.
  drupal_set_header('Content-Type: text/'. ($jsop == 'upload' ? 'html' : 'javascript') .'; charset=utf-8');
  print drupal_to_js($response);
  exit();
}

/**
 * Upload form.
 */
function imce_upload_form(&$form_state, $ref) {
  $imce =& $ref['imce'];
  $form['imce'] = array(
    '#type' => 'file',
    '#title' => t('File'),
    '#size' => 30,
  );
  if (!empty($imce['thumbnails'])) {
    $form['thumbnails'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Create thumbnails'),
      '#options' => imce_thumbnail_options($imce['thumbnails']),
    );
  }
  $form['upload'] = array(
    '#type' => 'submit',
    '#value' => t('Upload'),
    '#submit' => $imce['perm']['upload'] ? array('imce_upload_submit') : NULL,
  );
  $form = array('fset_upload' => array('#type' => 'fieldset', '#title' => t('Upload file')) + $form);
  $form['#attributes']['enctype'] = 'multipart/form-data';
  $form['#action'] = $imce['url'];
  return $form;
}

/**
 * File operations form.
 */
function imce_fileop_form(&$form_state, $ref) {
  $imce =& $ref['imce'];
  $form['filenames'] = array(
    '#type' => 'textfield',
    '#title' => t('Selected files'),
    '#maxlength' => $imce['filenum'] ? $imce['filenum']*255 : NULL,
  );

  //thumbnail
  if (!empty($imce['thumbnails']) && imce_perm_exists($imce, 'thumb')) {
    $form['fset_thumb'] = array(
      '#type' => 'fieldset',
      '#title' => t('Thumbnails'),
    ) + imce_thumb_form($imce);
  }

  //delete
  if (imce_perm_exists($imce, 'delete')) {
    $form['fset_delete'] = array(
      '#type' => 'fieldset',
      '#title' => t('Delete'),
    ) + imce_delete_form($imce);
  }

  //resize
  if (imce_perm_exists($imce, 'resize')) {
    $form['fset_resize'] = array(
      '#type' => 'fieldset',
      '#title' => t('Resize'),
    ) + imce_resize_form($imce);
  }
 
  $form['#action'] = $imce['url'];
  return $form;
}

/**
 * Thumbnail form.
 */
function imce_thumb_form(&$imce) {
  $form['thumbnails'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Thumbnails'),
    '#options' => imce_thumbnail_options($imce['thumbnails']),
  );
  $form['thumb'] = array(
    '#type' => 'submit',
    '#value' => t('Create thumbnails'),
    '#submit' => $imce['perm']['thumb'] ? array('imce_thumb_submit') : NULL,
  );
  return $form;
}

/**
 * Delete form.
 */
function imce_delete_form(&$imce) {
  $form['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#submit' => $imce['perm']['delete'] ? array('imce_delete_submit') : NULL,
  );
  return $form;
}

/**
 * Resizing form.
 */
function imce_resize_form(&$imce) {
  $form['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Width x Height'),
    '#size' => 5,
    '#maxlength' => 4,
    '#prefix' => '<div class="container-inline">',
  );
  $form['height'] = array(
    '#type' => 'textfield',
    '#size' => 5,
    '#maxlength' => 4,
    '#prefix' => 'x',
  );
  $form['resize'] = array(
    '#type' => 'submit',
    '#value' => t('Resize'),
    '#submit' => $imce['perm']['resize'] ? array('imce_resize_submit') : NULL,//permission for submission
    '#suffix' => '</div>',
  );
  $form['copy'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create a new image'),
    '#default_value' => 1,
  );
  return $form;
}

/**
 * Validate file operations form.
 */
function imce_fileop_form_validate($form, &$form_state) {
  $imce =& $form['#parameters'][2]['imce'];

  //check if the filenames is empty
  if ($form_state['values']['filenames'] == '') {
    return form_error($form['filenames'], t('Please select a file.'));
  }

  //filenames come seperated by colon
  $filenames = explode(':', $form_state['values']['filenames']);
  $cnt = count($filenames);
  //check the number of files.
  if ($imce['filenum'] && $cnt > $imce['filenum']) {
    return form_error($form['filenames'], t('You are not allowed to operate on more than %num files.', array('%num' => $imce['filenum'])));
  }

  //check if there is any illegal choice
  for ($i = 0; $i < $cnt; $i++) {
    $filenames[$i] = $filename = rawurldecode($filenames[$i]);
    if (!isset($imce['files'][$filename])) {
      watchdog('imce', 'Illegal choice %choice in !name element.', array('%choice' => $filename, '!name' => t('directory (%dir)', array('%dir' => file_directory_path() . ($imce['dir'] == '.' ? '' : '/'. $imce['dir'])))), WATCHDOG_ERROR);
      return form_error($form['filenames'], t('An illegal choice has been detected. Please contact the site administrator.'));
    }
  }

  $form_state['values']['filenames'] = $filenames;
}

/**
 * Submit upload form.
 */
function imce_upload_submit($form, &$form_state) {
  $form_state['redirect'] = FALSE;
  $imce =& $form['#parameters'][2]['imce'];
  $validators = array('imce_validate_all' => array(&$imce));
  $dirpath = file_directory_path() . ($imce['dir'] == '.' ? '' : '/'. $imce['dir']);

  //save uploaded file.
  $replace = variable_get('imce_settings_replace', FILE_EXISTS_RENAME);
  if ($file = file_save_upload('imce', $validators, $dirpath, $replace)) {

    //core bug #203204.
    @chmod($file->filepath, 0664);

    //core bug #54223.
    if ($replace == FILE_EXISTS_RENAME) {
      $name = basename($file->filepath);
      if ($name != $file->filename) {
        $file->filename = $name;
        drupal_set_message(t('The file has been renamed to %filename.', array('%filename' => $file->filename)));
      }
    }
    elseif ($replace == FILE_EXISTS_REPLACE) {//check duplicates
      if ($_file = db_fetch_object(db_query("SELECT fid FROM {files} WHERE filepath = '%s' AND fid <> %d", $file->filepath, $file->fid))) {
        db_query("DELETE FROM {files} WHERE fid = %d", $file->fid);
        $file->fid = $_file->fid;
      }
    }

    $file->uid = $imce['uid'];//global user may not be the owner.
    $file->status = FILE_STATUS_PERMANENT;//make permanent
    drupal_write_record('files', $file, array('fid'));//update
    imce_file_register($file);
    drupal_set_message(t('%filename has been uploaded.', array('%filename' => $file->filename)));

    //update file list
    $img = imce_image_info($file->filepath);
    $file->width = $img ? $img['width'] : 0;
    $file->height = $img ? $img['height'] : 0;
    imce_add_file($file, $imce);

    //create thumbnails
    if (isset($form_state['values']['thumbnails']) && $img) {
      imce_create_thumbnails($file->filename, $imce, $form_state['values']['thumbnails']);
    }
  }
  else {
    drupal_set_message(t('Upload failed.'), 'error');
  }
}

/**
 * Submit thumbnail form.
 */
function imce_thumb_submit($form, &$form_state) {
  $form_state['redirect'] = FALSE;
  $imce =& $form['#parameters'][2]['imce'];
  //create thumbnails
  imce_process_files($form_state['values']['filenames'], $imce, 'imce_create_thumbnails', array($form_state['values']['thumbnails']));
}

/**
 * Submit delete form.
 */
function imce_delete_submit($form, &$form_state) {
  $form_state['redirect'] = FALSE;
  $imce =& $form['#parameters'][2]['imce'];

  $deleted = imce_process_files($form_state['values']['filenames'], $imce, 'imce_delete_file');

  if (!empty($deleted)) {
    drupal_set_message(t('File deletion successful: %files.', array('%files' => utf8_encode(implode(', ', $deleted)))));
  }

}

/**
 * Submit resize form.
 */
function imce_resize_submit($form, &$form_state) {
  $form_state['redirect'] = FALSE;
  $imce =& $form['#parameters'][2]['imce'];

  //check dimensions
  $width = (int) $form_state['values']['width'];
  $height = (int) $form_state['values']['height'];
  list($maxw, $maxh) = explode('x', $imce['dimensions']);
  if ($width < 1 || $height < 1 || ($maxw && ($width > $maxw || $height > $maxh))) {
    drupal_set_message(t('Please specify dimensions within the allowed range that is from 1x1 to @dimensions.', array('@dimensions' => $imce['dimensions'] ? $imce['dimensions'] : t('unlimited'))), 'error');
    return;
  }

  $resized = imce_process_files($form_state['values']['filenames'], $imce, 'imce_resize_image', array($width, $height, $form_state['values']['copy']));

  if (!empty($resized)) {
    drupal_set_message(t('File resizing successful: %files.', array('%files' => utf8_encode(implode(', ', $resized)))));
  }

}

/**
 * Do batch operations on files.
 * Used by delete, resize, create thumbnail submissions.
 */
function imce_process_files($filenames, &$imce, $function, $args = array()) {
  $args = array_merge(array('', &$imce), $args);
  $processed = array();

  foreach ($filenames as $filename) {
    $args[0] = $filename;
    if (call_user_func_array($function, $args)) {
      $processed[] = $filename;
    }
  }

  return $processed;
}

/**
 * Delete a file in the file list.
 */
function imce_delete_file($filename, &$imce) {
  $filepath = file_directory_path() . ($imce['dir'] == '.' ? '' : '/'. $imce['dir']) .'/'. $filename;
  if (!imce_delete_filepath($filepath)) {
    return FALSE;
  }
  imce_remove_file($filename, $imce);
  return TRUE;
}

/**
 * Delete a file by path.
 */
function imce_delete_filepath($filepath) {
  $file = db_fetch_object(db_query("SELECT * FROM {files} WHERE filepath = '%s'", $filepath));

  //file exists in database
  if ($file) {
    //prevent imce returning ref count
    $file->imce_noref = TRUE;
    //check references
    $refs = array_filter(module_invoke_all('file_references', $file));
    //file is in use
    if (!empty($refs)) {
      drupal_set_message(t('%filename is in use by another application.', array('%filename' => $file->filename)), 'error');
      return FALSE;
    }
    //prepare deletion
    module_invoke_all('file_delete', $file);
    if (!file_delete($file->filepath)) {
      return FALSE;
    }
    db_query('DELETE FROM {files} WHERE fid = %d', $file->fid);
  }
  //not in db. probably loaded via ftp.
  elseif (!file_delete($filepath)) {
    return FALSE;
  }

  return TRUE;
}

/**
 * Create all selected thumbnails.
 */
function imce_create_thumbnails($filename, &$imce, $values) {
  $created = array();
  foreach ($imce['thumbnails'] as $thumbnail) {
    if ($values[$thumbnail['name']] && imce_create_thumbnail($filename, $imce, $thumbnail)) {
      $created[] = $thumbnail['name'];
    }
  }
  if (!empty($created)) {
  drupal_set_message(t('Thumbnail creation (%thumbnames) successful for %filename.', array('%thumbnames' => implode(', ', $created), '%filename' => utf8_encode($filename))));
  }
  return $created;
}

/**
 * Create a thumbnail.
 */
function imce_create_thumbnail($filename, &$imce, $thumbnail) {
  //generate thumbnail name
  $name = $thumbnail['prefix'];
  if ($thumbnail['suffix'] != '' && $dot = strrpos($filename, '.')) {
    $name .= substr($filename, 0, $dot);
    $name .= $thumbnail['suffix'];
    $name .= substr($filename, $dot);
  }
  else {
    $name .= $filename;
  }
  //scale the image
  list($width, $height) = explode('x', $thumbnail['dimensions']);
  return imce_resize_image($filename, $imce, $width, $height, TRUE, $name, variable_get('imce_settings_thumb_method', 'scale_and_crop'));
}

/**
 * Resize an image in the file list. Also used for thumbnail creation.
 */
function imce_resize_image($filename, &$imce, $width, $height, $copy = TRUE, $dest = FALSE, $op = 'resize') {
  $dirpath = file_directory_path() . ($imce['dir'] == '.' ? '' : '/'. $imce['dir']);
  $filepath = $dirpath .'/'. $filename;

  //check if the file is an image
  if (!$imce['files'][$filename]['width'] || !$img = imce_image_info($filepath)) {
    drupal_set_message(t('%filename is not an image.', array('%filename' => utf8_encode($filename))), 'error', FALSE);
    return FALSE;
  }

  if (substr($op, 0, 5) == 'scale' && !($width < $img['width'] || $height < $img['height'])) {
    drupal_set_message(t('Scaling up is not allowed.'), 'error', FALSE);
    return FALSE;
  }

  //create file object
  $file = new stdClass();
  $file->filepath = $dirpath .'/'. $dest;
  if (!$dest || $dest == $filename) {
    $file->filepath = $copy ? file_create_filename($filename, $dirpath) : $filepath;
  }
  $file->filename = basename($file->filepath);

  //check if a file having the same properties exists already.
  if (isset($imce['files'][$file->filename])) {
    if (($f = $imce['files'][$file->filename]) && $f['width'] == $width && $f['height'] == $height) {
      drupal_set_message(t('%filename(%dimensions) already exists.', array('%filename' => utf8_encode($file->filename), '%dimensions' => $width .'x'. $height)), 'error');
      return FALSE;
    }
  }

  //validate file name
  $errors = file_validate_name_length($file);
  if (!empty($errors)) {
    drupal_set_message($errors[0], 'error');
    return FALSE;
  }

  //resize image to a temp file
  $temp = tempnam(realpath(file_directory_temp()), 'imc');
  register_shutdown_function('file_delete', $temp);
  $function = 'image_'. $op;
  if (!$function($filepath, $temp, $width, $height)) {
    drupal_set_message(t('%filename cannot be resized to %dimensions', array('%filename' => utf8_encode($filename), '%dimensions' => $width .'x'. $height)), 'error', FALSE);
    return FALSE;
  }

  //validate quota
  $file->filesize = filesize($temp);
  $overwrite = $file->filename == $filename;
  if (!imce_validate_quotas($file, $imce, $overwrite ? -$imce['files'][$filename]['size'] : 0)) {
    return FALSE;
  }

  //copy from temp to filepath
  if (!@copy($temp, $file->filepath)) {
    drupal_set_message(t('The selected file %file could not be copied.', array('%file' => utf8_encode($file->filename))), 'error', FALSE);
    return FALSE;
  }
  @chmod($file->filepath, 0664);

  //build the rest of the file object
  $file->uid = $imce['uid'];
  $file->filemime = $img['mime'];
  $file->status = FILE_STATUS_PERMANENT;
  $file->timestamp = time();

  //if we are overwriting the file and it is already in database.
  $update = array();
  if ($overwrite && $_file = db_fetch_object(db_query("SELECT f.* FROM {files} f WHERE f.filepath = '%s'", $file->filepath))) {
    $file->fid = $_file->fid;
    $file->uid = $_file->uid;
    $update[] = 'fid';
  }

  //save the file
  drupal_write_record('files', $file, $update);
  imce_file_register($file);

  //update file list
  //if the file was scaled get the new dimensions
  if ($op == 'scale') {
    $img = imce_image_info($file->filepath);
    $width = $img['width'];
    $height = $img['height'];
  }
  $file->width = $width;
  $file->height = $height;
  imce_add_file($file, $imce);

  return $file;
}

/**
 * Add a new file to the file list.
 */
function imce_add_file($file, &$imce) {
  $imce['dirsize'] += $file->filesize;
  if (isset($imce['files'][$file->filename])) {
    $imce['dirsize'] -= $imce['files'][$file->filename]['size'];
  }
  $imce['files'][$file->filename] = array(
    'name' => $file->filename,
    'size' => $file->filesize,
    'width' => $file->width,
    'height' => $file->height,
    'date' => $file->timestamp
  );
  if (isset($_GET['jsop'])) {
    $add = $imce['files'][$file->filename];
    $add['name'] = rawurlencode($file->filename);
    $add['fsize'] = format_size($file->filesize);
    $add['fdate'] = format_date($file->timestamp, 'small');
    $add['id'] = $file->fid;
    $imce['added'][] = $add;
  }
}

/**
 * Remove a file from the file list.
 */
function imce_remove_file($filename, &$imce) {
  if (isset($imce['files'][$filename])) {
    $imce['dirsize'] -= $imce['files'][$filename]['size'];
    unset($imce['files'][$filename]);
    if (isset($_GET['jsop'])) {
      $imce['removed'][] = rawurlencode($filename);
    }
  }
}

/**
 * Validate uploaded file.
 */
function imce_validate_all(&$file, $imce) {

  //fix FILE_EXISTS_ERROR bug. core bug #54223.
  if (!$file->destination && variable_get('imce_settings_replace', FILE_EXISTS_RENAME) == FILE_EXISTS_ERROR) {
    return array(t('File browser is set to reject the upload of existing files.'));
  }

  //validate image resolution only if filesize validation passes.
  //because user might have uploaded a very big image
  //and scaling it may exploit system memory.
  $errors = imce_validate_filesize($file, $imce['filesize']);
  //image resolution validation
  if (empty($errors) && preg_match('/\.(png|gif|jpe?g)$/i', $file->filename)) {
    $errors = array_merge($errors, file_validate_image_resolution($file, $imce['dimensions']));
  }
  //directory quota validation
  if ($imce['quota']) {
    $errors = array_merge($errors, imce_validate_quota($file, $imce['quota'], $imce['dirsize']));
  }
  //file extension validation
  if ($imce['extensions'] != '*') {
    $errors = array_merge($errors, file_validate_extensions($file, $imce['extensions']));
  }
  //user quota validation. check it if no errors were thrown.
  if (empty($errors) && $imce['tuquota']) {
    $errors = imce_validate_tuquota($file, $imce['tuquota'], file_space_used($imce['uid']));
  }
  return $errors;
}

/**
 * Validate filesize for maximum allowed file size.
 */
function imce_validate_filesize($file, $maxsize = 0) {
  $errors = array();
  if ($maxsize && $file->filesize > $maxsize) {
    $errors[] = t('The file is %filesize exceeding the maximum file size of %maxsize.', array('%filesize' => format_size($file->filesize), '%maxsize' => format_size($maxsize)));
  }
  return $errors;
}

/**
 * Validate filesize for directory quota.
 */
function imce_validate_quota($file, $quota = 0, $currentsize = 0) {
  $errors = array();
  if ($quota && ($currentsize + $file->filesize) > $quota) {
    $errors[] = t('%filename is %filesize which would exceed your directory quota. You are currently using %size of %total_quota.', array('%size' => format_size($currentsize), '%total_quota' => format_size($quota), '%filesize' => format_size($file->filesize), '%filename' => utf8_encode($file->filename)));
  }
  return $errors;
}

/**
 * Validate filesize for total user quota.
 */
function imce_validate_tuquota($file, $quota = 0, $currentsize = 0) {
  $errors = array();
  if ($quota && ($currentsize + $file->filesize) > $quota) {
    $errors[] = t('%filename is %filesize which would exceed your total user quota. You are currently using %size of %total_quota.', array('%size' => format_size($currentsize), '%total_quota' => format_size($quota), '%filesize' => format_size($file->filesize), '%filename' => utf8_encode($file->filename)));
  }
  return $errors;
}

/**
 * Validate both directory and total user quota. Returns true/false not errors.
 */
function imce_validate_quotas($file, &$imce, $add = 0) {
  $errors = imce_validate_quota($file, $imce['quota'], $imce['dirsize'] + $add);
  if (empty($errors) && $imce['tuquota']) {
    $errors = imce_validate_tuquota($file, $imce['tuquota'], file_space_used($imce['uid']) + $add);
  }
  if (!empty($errors)) {
    drupal_set_message($errors[0], 'error');
    return FALSE;
  }
  return TRUE;
}

/**
 * Check if the file is an image and return info.
 */
function imce_image_info($file) {
  if (is_file($file) && ($dot = strrpos($file, '.')) && in_array(strtolower(substr($file, $dot+1)), array('jpg', 'jpeg', 'gif', 'png')) && ($info = @getimagesize($file)) && in_array($info[2], array(IMAGETYPE_JPEG, IMAGETYPE_GIF, IMAGETYPE_PNG)) ) {
    return array('width' => $info[0], 'height' => $info[1], 'type' => $info[2], 'mime' => $info['mime']);
  }
  return FALSE;
}

/**
 * Return thumbnails as options to be used in upload form.
 */
function imce_thumbnail_options($thumbs = array()) {
  $options = array();
  foreach ($thumbs as $thumb) {
    $options[$thumb['name']] = $thumb['name'] .' ('. $thumb['dimensions'] .')';
  }
  return $options;
}

/**
 * Initiate and return configuration profile for the $user.
 */
function imce_initiate_profile($user) {

  //check user profile and translate tokens in directory paths and evaluate php paths.
  if ($imce = imce_user_profile($user)) {
    // Allow alteration of raw profile
    foreach (variable_get('imce_custom_init', array()) as $func => $state) {
      if ($state && function_exists($func)) {
        $func($imce, $user);
      }
    }
    imce_process_directories($imce, $user);
    if (!empty($imce['directories'])) {
      $imce['uid'] = (int) $user->uid;
      $imce['url'] = url($_GET['q']);
      $imce['clean'] = variable_get('clean_url', 0) == 1;
      $imce['absurls'] = variable_get('imce_settings_absurls', 0) == 1;
      $private = variable_get('file_downloads', '') == FILE_DOWNLOADS_PRIVATE;
      $imce['furl'] = $private ? url('system/files') : base_path() . file_directory_path();
      $imce['filesize'] *= 1048576;//convert from Mb to byte
      $imce['quota'] *= 1048576;
      $imce['tuquota'] *= 1048576;
      $imce['filenum'] = (int) $imce['filenum'];
      //check and set the active directory
      if ($info = imce_working_directory($imce)) {
        $imce['direct'] = isset($imce['directories'][$info['name']]);
        $imce['directories'][$info['name']] = $info;
        $imce['dir'] = $info['name'];
        $imce['perm'] = $info;//copy permissions of the active directory.
        unset($imce['perm']['name']);
      }
      else {
        drupal_set_message(t('Unable to get a working directory for the file browser!'), 'error');
        $imce['dir'] = FALSE;
        $imce['error'] = TRUE;
      }
      return $imce;
    }
    drupal_set_message(t('There is no valid directory specified for the file browser!'), 'error');
  }
  else {
    drupal_set_message(t('You do not have access to any configuration profile to use the file browser!'), 'error');
  }

  return FALSE;
}

/**
 * Get files and folders of the actve directory. Do custom processing.
 */
function imce_process_profile(&$imce) {
  //get directory content. do a custom scan if it is set
  $scan = ($scan = variable_get('imce_custom_scan', '')) && function_exists($scan) ? $scan : 'imce_scan_directory';
  $imce += $scan($imce['dir'], $imce);

  //run custom process functions
  foreach (variable_get('imce_custom_process', array()) as $func => $state) {
    if ($state && function_exists($func)) {
      $func($imce);
    }
  }

  //set subdirectories
  if (!$imce['error'] && !imce_subdirectories_accessible($imce)) {
    $imce['subdirectories'] = array();
  }
}

/**
 * Translate tokens and evaluate php in directory names.
 * Convert directories into an associative array (dirname => info)
 */
function imce_process_directories(&$imce, $user) {
  $directories = $imce['directories'];
  $paths = array();
  $translate = array('%uid' => $user->uid);

  foreach ($directories as $directory) {
    if (substr($directory['name'], 0, 4) == 'php:') {
      $directory['name'] = eval(substr($directory['name'], 4));
     //php may return an array of directories
      if (is_array($directory['name'])) {
        foreach ($directory['name'] as $name) {
          $paths[$name] = array('name' => $name) + $directory;
        }
        continue;
      }
    }
    else {
      $directory['name'] = strtr($directory['name'], $translate);
    }
    if ($directory['name']) {
      $paths[$directory['name']] = $directory;
    }
  }

  $imce['directories'] = $paths;
}

/**
 * Return an avaliable directory for the profile.
 */
function imce_working_directory(&$imce) {
  //Do not use session if there is only one directory assigned.
  $sess = TRUE;
  if (count($imce['directories']) < 2) {
    $perms = reset($imce['directories']);
    if (!isset($perms['subnav']) || !$perms['subnav']) {
      $sess = FALSE;
    }
  }
  //check GET.
  if (isset($_GET['dir'])) {
    if ($info = imce_directory_info($_GET['dir'], $imce)) {
      if (imce_check_directory($_GET['dir'], $imce)) {
        if ($sess) {
          $_SESSION['imce_directory'] = rawurlencode($info['name']);
        }
      }
      else {
        $info = FALSE;
      }
    }
    else {
      imce_inaccessible_directory($_GET['dir'], $imce);
    }
    return $info;
  }

  //check session
  if ($sess && isset($_SESSION['imce_directory'])) {
    $dirname = rawurldecode($_SESSION['imce_directory']);
    if ($info = imce_directory_info($dirname, $imce)) {
      if (imce_check_directory($dirname, $imce)) {
        return $info;
      }
    }
  }

  //or the whole list.
  foreach ($imce['directories'] as $dirname => $info) {
    if (imce_check_directory($dirname, $imce)) {
      if ($sess) {
        $_SESSION['imce_directory'] = rawurlencode($dirname);
      }
      return $info;
    }
  }

  return FALSE;
}

/**
 * Create a writable directory(any level) under file system directory.
 */
function imce_check_directory($dirname, $imce = array()) {

  if (!imce_reg_dir($dirname)) {
    return imce_inaccessible_directory($dirname, $imce);
  }

  $root = file_directory_path();
  $dirpath = $root .'/'. $dirname;

  if (!file_check_directory($dirpath)) {//directory does not exist. try to create it.
    $path = $root;
    foreach (explode('/', $dirname) as $arg) {
      $path .= '/'. $arg;
      if (!file_check_directory($path, FILE_CREATE_DIRECTORY)) {
        return imce_inaccessible_directory($dirname, $imce);
      }
    }
  }

  return TRUE;
}

/**
 * Generate and log a directory access error.
 */
function imce_inaccessible_directory($dirname, $imce = array()) {
  if (is_string($dirname)) {
    $dirname = utf8_encode($dirname);
    drupal_set_message(t('Directory %dirname is not accessible.', array('%dirname' => $dirname)), 'error');
    watchdog('imce', 'Access to %directory was denied.', array('%directory' => $dirname), WATCHDOG_ERROR);
  }
  return FALSE;
}

/**
 * Return the permissions for a directory that is accessed directly or indirectly.
 * A child of a predefined directory in the directory list takes its parent's properties.
 * If it has multiple parents, it gets the properties of the latter in the list.
 */
function imce_directory_info($dirname, $imce) {

  if (isset($imce['directories'][$dirname])) {
    return $imce['directories'][$dirname];
  }

  $info = FALSE;
  $root = file_directory_path();
  $dirpath = $root .'/'. $dirname;
  if (imce_reg_dir($dirname) && file_check_directory($dirpath)) {
    foreach ($imce['directories'] as $name => $prop) {
      if ($prop['subnav'] && ($name === '.' || strpos($dirname . '/', $name . '/') === 0)) {
        $info = $prop;
        $info['name'] = $dirname;
      }
    }
  }

  return $info;
}

/**
 * Detect if the subdirectories are accessible through any directory(not just the current one) in the list.
 */
function imce_subdirectories_accessible(&$imce) {

  if (!empty($imce['subdirectories'])) {
    if (!empty($imce['perm']['subnav'])) {
      return TRUE;
    }
    $root = file_directory_path() .'/';
    //checking only the first one is sufficient.
    $dirname = ($imce['dir'] == '.' ? '' : $imce['dir'] .'/') . $imce['subdirectories'][0];
    $dirpath = $root . $dirname;

    //check if any setting is applicable for this subdirectory through any directory in the list.
    foreach ($imce['directories'] as $name => $info) {
      $name = (string) $name;
      if ($info['subnav'] && $dirname != $name && ($name == '.' || strpos($dirname . '/', $name . '/') === 0)) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

/**
 * Check if a permission is given to at least one directory in the list.
 */
function imce_perm_exists(&$imce, $perm) {
  static $perms = array();

  if (isset($perms[$perm])) {
    return $perms[$perm];
  }

  if (isset($imce['perm'][$perm]) && $imce['perm'][$perm]) {
    return $perms[$perm] = TRUE;
  }

  foreach ($imce['directories'] as $name => $info) {
    if (isset($info[$perm]) && $info[$perm]) {
      return $perms[$perm] = TRUE;
    }
  }

  return $perms[$perm] = FALSE;
}

/**
 * Scan directory and return file list, subdirectories, and total size.
 */
function imce_scan_directory($dirname, $imce = array()) {

  $directory = array('dirsize' => 0, 'files' => array(), 'subdirectories' => array(), 'error' => FALSE);
  $dirpath = file_directory_path() .'/'. $dirname;

  if (!is_string($dirname) || $dirname == '' || !$handle = opendir($dirpath)) {
    imce_inaccessible_directory($dirname, $imce);
    $directory['error'] = TRUE;
    return $directory;
  }

  while (($file = readdir($handle)) !== FALSE) {

    // Do not include dot files and folders
    if (substr($file, 0, 1) === '.') {
      continue;
    }

    $path = $dirpath .'/'. $file;

    if (is_dir($path)) {
      $directory['subdirectories'][] = $file;
      continue;
    }

    $width = $height = 0;
    if ($img = imce_image_info($path)) {
      $width = $img['width'];
      $height = $img['height'];
    }
    $size = filesize($path);
    $date = filemtime($path);
    $directory['files'][$file] = array(
      'name' => $file,
      'size' => $size,
      'width' => $width,
      'height' => $height,
      'date' => $date
    );
    $directory['dirsize'] += $size;
  }

  closedir($handle);
  sort($directory['subdirectories']);
  return $directory;
}

/**
 * Create directory tree.
 */
function imce_create_tree(&$imce) {
  $paths = array();
  //rearrange paths as arg0=>arg1=>...
  foreach ($imce['directories'] as $path => $arr) {
    $tmp =& $paths;
    if ($path != '.') {
      $args = explode('/', $path);
      foreach ($args as $arg) {
        if (!isset($tmp[$arg])) {
          $tmp[$arg] = array();
        }
        $tmp =& $tmp[$arg];
      }
      $tmp[':access:'] = TRUE;
    }
    if ("$path" == $imce['dir']) {
      $tmp[':active:'] = TRUE;
      foreach ($imce['subdirectories'] as $arg) {
        $tmp[$arg][':access:'] = TRUE;
      }
    }
  }
  //set root branch
  $root = theme('imce_root_text', array('imce' => &$imce));
  $q = $imce['clean'] ? '?' : '&';
  if (isset($imce['directories']['.'])) {
    $root = '<a href="'. $imce['url'] . $q .'dir=." title="." class="folder'. ($imce['dir'] == '.' ? ' active' : '') .'">'. $root .'</a>';
  }
  else {
    $root = '<a title="." class="folder disabled">'. $root .'</a>';
  }

  return $root . imce_tree_html($imce, $paths, $q);
}

/**
 * Return tree html.
 * This is not themable because it is complex and needs to be in a proper format for js processing.
 */
function imce_tree_html(&$imce, $paths, $q = '?', $prefix = '', $eprefix = '') {
  unset($paths[':access:'], $paths[':active:']);
  $html = '';
  foreach ($paths as $arg => $children) {
    $path = $prefix . $arg;
    $earg = rawurlencode($arg);
    $epath = $eprefix . $earg;
    if (isset($children[':access:']) || imce_directory_info($path, $imce)) {
      $a = '<a href="'. $imce['url'] . $q .'dir='. $epath .'" title="'. $epath .'" class="folder'. (isset($children[':active:']) ? ' active' : '') .'">'. $earg .'</a>';
    }
    else {
      $a = '<a title="'. $epath .'" class="folder disabled">'. $earg .'</a>';
    }
    $ul = imce_tree_html($imce, $children, $q, $path .'/', $epath .'/');
    $class = $ul ? ' class="expanded"' : (isset($children[':active:']) ? ' class="leaf"' : '');
    $html .= '<li'. $class .'>'. $a . $ul .'</li>';
  }
  if ($html) {
    $html = '<ul>'. $html .'</ul>';
  }
  return $html;
}

/**
 * Returns the text for the root directory in a directory tree.
 */
function theme_imce_root_text($imce_ref) {
  //$imce = &$imce_ref['imce'];
  return '&lt;' . t('root') . '&gt;';
}

/**
 * Returns the html for user's file browser tab.
 */
function theme_imce_user_page($account) {
  global $user;
  $options = array();
  //switch to account's active folder
  if ($user->uid == 1 && $account->uid != 1) {
    $imce = imce_initiate_profile($account);
    $options['query'] = array('dir' => $imce['dir']);
  }
  return '<iframe src="' . url('imce', $options) . '" frameborder="0" style="border: 1px solid #eee; width: 99%; height: 520px" class="imce-frame"></iframe>';
}

/**
 * Registers the file as an IMCE file.
 */
function imce_file_register($file) {
  return $file->fid && @db_query('INSERT INTO {imce_files} (fid) VALUES(%d)', $file->fid);
}
