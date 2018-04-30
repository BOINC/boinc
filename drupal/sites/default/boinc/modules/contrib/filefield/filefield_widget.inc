<?php

/**
 * @file
 * This file contains CCK widget related functionality.
 *
 * Uses content.module to store the fid and field specific metadata,
 * and Drupal's files table to store the actual file data.
 */

/**
 * Implementation of CCK's hook_widget_settings($op == 'form').
 */
function filefield_widget_settings_form($widget) {
  $form = array();

  // Convert the extensions list to be a human-friendly comma-separated list.
  $extensions = is_string($widget['file_extensions']) ? $widget['file_extensions'] : 'txt';
  $form['file_extensions'] = array(
    '#type' => 'textfield',
    '#title' => t('Permitted upload file extensions'),
    '#default_value' => $extensions,
    '#size' => 64,
    '#maxlength' => 512,
    '#description' => t('Extensions a user can upload to this field. Separate extensions with a space and do not include the leading dot. Leaving this blank will allow users to upload a file with any extension.'),
    '#element_validate' => array('_filefield_widget_settings_extensions_validate'),
    '#pre_render' => array('_filefield_widget_settings_extensions_value'),
    '#weight' => 1,
  );

  $form['progress_indicator'] = array(
    '#type' => 'radios',
    '#title' => t('Progress indicator'),
    '#options' => array(
      'bar' => t('Bar with progress meter'),
      'throbber' => t('Throbber'),
    ),
    '#default_value' => empty($widget['progress_indicator']) ? 'bar' : $widget['progress_indicator'],
    '#description' => t('Your server supports upload progress capabilities. The "throbber" display does not indicate progress but takes up less room on the form, you may want to use it if you\'ll only be uploading small files or if experiencing problems with the progress bar.'),
    '#weight' => 5,
    '#access' => filefield_progress_implementation(),
  );

  $form['path_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Path settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 6,
  );
  $form['path_settings']['file_path'] = array(
    '#type' => 'textfield',
    '#title' => t('File path'),
    '#default_value' => is_string($widget['file_path']) ? $widget['file_path'] : '',
    '#description' => t('Optional subdirectory within the "%directory" directory where files will be stored. Do not include preceding or trailing slashes.', array('%directory' => variable_get('file_directory_path', 'files') . '/')),
    '#element_validate' => array('_filefield_widget_settings_file_path_validate'),
    '#suffix' => theme('token_help', 'user'),
  );

  $form['max_filesize'] = array(
    '#type' => 'fieldset',
    '#title' => t('File size restrictions'),
    '#description' => t('Limits for the size of files that a user can upload. Note that these settings only apply to newly uploaded files, whereas existing files are not affected.'),
    '#weight' => 6,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['max_filesize']['max_filesize_per_file'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size per file'),
    '#default_value' => is_string($widget['max_filesize_per_file'])
                        ? $widget['max_filesize_per_file']
                         : '',
    '#description' => t('Specify the size limit that applies to each file separately. Enter a value like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes) in order to restrict the allowed file size. If you leave this empty the file sizes will be limited only by PHP\'s maximum post and file upload sizes (current limit <strong>%limit</strong>).', array('%limit' => format_size(file_upload_max_size()))),
    '#element_validate' => array('_filefield_widget_settings_max_filesize_per_file_validate'),
  );
  $form['max_filesize']['max_filesize_per_node'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum upload size per node'),
    '#default_value' => is_string($widget['max_filesize_per_node'])
                        ? $widget['max_filesize_per_node']
                        : '',
    '#description' => t('Specify the total size limit for all files in field on a given node. Enter a value like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes) in order to restrict the total size of a node. Leave this empty if there should be no size restriction.'),
    '#element_validate' => array('_filefield_widget_settings_max_filesize_per_node_validate'),
  );

  return $form;
}

/**
 * Implementation of CCK's hook_widget_settings($op == 'save').
 */
function filefield_widget_settings_save($widget) {
  return array('file_extensions', 'file_path', 'progress_indicator', 'max_filesize_per_file', 'max_filesize_per_node');
}

/**
 * A FAPI #pre_render() function to set a cosmetic default value for extensions.
 */
function _filefield_widget_settings_extensions_value($element) {
  $element['#value'] = implode(', ', array_filter(explode(' ', str_replace(',', ' ', $element['#value']))));
  return $element;
}

/**
 * A FAPI #element_validate callback to strip commas from extension lists.
 */
function _filefield_widget_settings_extensions_validate($element, &$form_state) {
  // Remove commas and leading dots from file extensions.
  $value = str_replace(',', ' ', $element['#value']);
  $value = str_replace(' .', ' ', $value);
  $value = array_filter(explode(' ', $value));
  $value = implode(' ', $value);
  form_set_value($element, $value, $form_state);
}

function _filefield_widget_settings_file_path_validate($element, &$form_state) {
  // Strip slashes from the beginning and end of $widget['file_path']
  $form_state['values']['file_path'] = trim($form_state['values']['file_path'], '\\/');

  // Do not allow the file path to be the same as the file_directory_path().
  // This causes all sorts of problems with things like file_create_url().
  if (strpos($form_state['values']['file_path'], file_directory_path()) === 0) {
    form_error($element, t('The file path (@file_path) cannot start with the system files directory (@files_directory), as this may cause conflicts when building file URLs.', array('@file_path' => $form_state['values']['file_path'], '@files_directory' => file_directory_path())));
  }
}

function _filefield_widget_settings_max_filesize_per_file_validate($element, &$form_state) {
  if (empty($form_state['values']['max_filesize_per_file'])) {
    return; // Empty means no size restrictions, so don't throw an error.
  }
  elseif (!is_numeric(parse_size($form_state['values']['max_filesize_per_file']))) {
    form_error($element, t('The "@field" option must contain a valid value. You can either leave the text field empty or enter a string like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes).', array('@field' => t('Maximum upload size per file'))));
  }
}

function _filefield_widget_settings_max_filesize_per_node_validate($element, &$form_state) {
  if (empty($form_state['values']['max_filesize_per_node'])) {
    return; // Empty means no size restrictions, so don't throw an error.
  }
  elseif (!is_numeric(parse_size($form_state['values']['max_filesize_per_node']))) {
    form_error($element, t('The "@field" option must contain a valid value. You can either leave the text field empty or enter a string like "512" (bytes), "80K" (kilobytes) or "50M" (megabytes).', array('@field' => t('Maximum upload size per node'))));
  }
}

/**
 * Determine the widget's files directory
 *
 * @param $field
 *   A CCK field array.
 * @param $account
 *   The user account object to calculate the file path for.
 * @return
 *   The files directory path, with any tokens replaced.
 */
function filefield_widget_file_path($field, $account = NULL) {
  $account = isset($account) ? $account : $GLOBALS['user'];
  $dest = $field['widget']['file_path'];
  // Replace user level tokens.
  // Node level tokens require a lot of complexity like temporary storage
  // locations when values don't exist. See the filefield_paths module.
  if (module_exists('token')) {
    $dest = token_replace($dest, 'user', $account);
  }
  // Replace nasty characters in the path if possible.
  if (module_exists('transliteration')) {
    module_load_include('inc', 'transliteration');
    $dest_array = array_filter(explode('/', $dest));
    foreach ($dest_array as $key => $directory) {
      $dest_array[$key] = transliteration_clean_filename($directory);
    }
    $dest = implode('/', $dest_array);
  }

  return file_directory_path() .'/'. $dest;
}

/**
 * Given a FAPI element, save any files that may have been uploaded into it.
 *
 * This function should only be called during validate, submit, or
 * value_callback functions.
 *
 * @param $element
 *   The FAPI element whose values are being saved.
 */
function filefield_save_upload($element) {
  $upload_name = implode('_', $element['#array_parents']);
  $field = content_fields($element['#field_name'], $element['#type_name']);

  if (empty($_FILES['files']['name'][$upload_name])) {
    return 0;
  }

  $dest = filefield_widget_file_path($field);
  if (!field_file_check_directory($dest, FILE_CREATE_DIRECTORY)) {
    watchdog('filefield', 'The upload directory %directory for the file field %field (content type %type) could not be created or is not accessible. A newly uploaded file could not be saved in this directory as a consequence, and the upload was canceled.', array('%directory' => $dest, '%field' => $element['#field_name'], '%type' => $element['#type_name']));
    form_set_error($upload_name, t('The file could not be uploaded.'));
    return 0;
  }

  if (!$file = field_file_save_upload($upload_name, $element['#upload_validators'], $dest)) {
    watchdog('filefield', 'The file upload failed. %upload', array('%upload' => $upload_name));
    form_set_error($upload_name, t('The file in the @field field was unable to be uploaded.', array('@field' => $element['#title'])));
    return 0;
  }
  return $file['fid'];
}

/**
 * The #value_callback for the filefield_widget type element.
 */
function filefield_widget_value($element, $edit = FALSE) {
  if (!$edit) {
    $file = field_file_load($element['#default_value']['fid']);
    $item = $element['#default_value'];
  }
  else {
    $item = $edit;
    $field = content_fields($element['#field_name'], $element['#type_name']);

    // Uploads take priority over value of fid text field.
    if ($fid = filefield_save_upload($element)) {
      $item['fid'] = $fid;
    }
    // Check for #filefield_value_callback values.
    // Because FAPI does not allow multiple #value_callback values like it does
    // for #element_validate and #process, this fills the missing functionality
    // to allow FileField to be extended purely through FAPI.
    elseif (isset($element['#filefield_value_callback'])) {
      foreach ($element['#filefield_value_callback'] as $callback) {
        $callback($element, $item);
      }
    }

    // Load file if the FID has changed so that it can be saved by CCK.
    $file = isset($item['fid']) ? field_file_load($item['fid']) : NULL;

    // If the file entry doesn't exist, don't save anything.
    if (empty($file)) {
      $item = array();
      $file = array();
    }

    // Checkboxes loose their value when empty.
    // If the list field is present make sure its unchecked value is saved.
    if (!empty($field['list_field']) && empty($edit['list'])) {
      $item['list'] = 0;
    }
  }
  // Merge file and item data so it is available to all widgets.
  if (isset($item['data']) && isset($file['data'])) {
    $file['data'] = array_merge($item['data'], $file['data']);
  }
  $item = array_merge($item, $file);

  return $item;
}

/**
 * An element #process callback for the filefield_widget field type.
 *
 * Expands the filefield_widget type to include the upload field, upload and
 * remove buttons, and the description field.
 */
function filefield_widget_process($element, $edit, &$form_state, $form) {
  static $settings_added;

  $item = $element['#value'];
  $field_name = $element['#field_name'];
  $delta = $element['#delta'];
  $element['#theme'] = 'filefield_widget_item';

  $field = $form['#field_info'][$field_name];

  // The widget is being presented, so apply the JavaScript.
  drupal_add_js(drupal_get_path('module', 'filefield') .'/filefield.js');
  if (!isset($settings_added[$field_name]) && isset($element['#upload_validators']['filefield_validate_extensions'])) {
    $settings_added[$field_name] = TRUE;
    $settings = array(
      'filefield' => array(
        $field_name => $element['#upload_validators']['filefield_validate_extensions'][0],
      ),
    );
    drupal_add_js($settings, 'setting');
  }

  // Title is not necessary for each individual field.
  if ($field['multiple'] > 0) {
    unset($element['#title']);
  }

  // Set up the buttons first since we need to check if they were clicked.
  $element['filefield_upload'] = array(
    '#type' => 'submit',
    '#value' => t('Upload'),
    '#submit' => array('node_form_submit_build_node'),
    '#ahah' => array( // with JavaScript
       'path' => 'filefield/ahah/'.   $element['#type_name'] .'/'. $element['#field_name'] .'/'. $element['#delta'],
       'wrapper' => $element['#id'] .'-ahah-wrapper',
       'method' => 'replace',
       'effect' => 'fade',
    ),
    '#field_name' => $element['#field_name'],
    '#delta' => $element['#delta'],
    '#type_name' => $element['#type_name'],
    '#upload_validators' => $element['#upload_validators'],
    '#weight' => 100,
    '#post' => $element['#post'],
  );
  $element['filefield_remove'] = array(
    // With default CCK edit forms, $element['#parents'] is array($element['#field_name'], $element['#delta']).
    // However, if some module (for example, flexifield) places our widget deeper in the tree, we want to
    // use that information in constructing the button name.
    '#name' => implode('_', $element['#parents']) .'_filefield_remove',
    '#type' => 'submit',
    '#value' => t('Remove'),
    '#submit' => array('node_form_submit_build_node'),
    '#ahah' => array( // with JavaScript
      'path' => 'filefield/ahah/'.   $element['#type_name'] .'/'. $element['#field_name'] .'/'. $element['#delta'],
      'wrapper' => $element['#id'] .'-ahah-wrapper',
      'method' => 'replace',
      'effect' => 'fade',
    ),
    '#field_name' => $element['#field_name'],
    '#delta' => $element['#delta'],
    '#weight' => 101,
    '#post' => $element['#post'],
  );

  // Because the output of this field changes depending on the button clicked,
  // we need to ask FAPI immediately if the remove button was clicked.
  // It's not good that we call this private function, but
  // $form_state['clicked_button'] is only available after this #process
  // callback is finished.
  if (_form_button_was_clicked($element['filefield_remove'])) {
    // Delete the file if it is currently unused. Note that field_file_delete()
    // does a reference check in addition to our basic status check.
    if (isset($edit['fid'])) {
      $removed_file = field_file_load($edit['fid']);
      // Users are only allowed to remove their own files.
      if ($removed_file['status'] == 0  && $GLOBALS['user']->uid == $removed_file['uid']) {
        field_file_delete($removed_file);
      }
    }
    $item = array('fid' => 0, 'list' => $field['list_default'], 'data' => array('description' => ''));
  }

  // Add progress bar support to the upload if possible.
  $progress_indicator = isset($field['widget']['progress_indicator']) ? $field['widget']['progress_indicator'] : 'bar';
  if ($progress_indicator != 'throbber' && $implementation = filefield_progress_implementation()) {
    $upload_progress_key = md5(mt_rand());

    if ($implementation == 'uploadprogress') {
      $element['UPLOAD_IDENTIFIER'] = array(
        '#type' => 'hidden',
        '#value' => $upload_progress_key,
        '#attributes' => array('class' => 'filefield-progress'),
      );
    }
    elseif ($implementation == 'apc') {
      $element['APC_UPLOAD_PROGRESS'] = array(
        '#type' => 'hidden',
        '#value' => $upload_progress_key,
        '#attributes' => array('class' => 'filefield-progress'),
      );
    }

    // Add the upload progress callback.
    $element['filefield_upload']['#ahah']['progress']['type'] = 'bar';
    $element['filefield_upload']['#ahah']['progress']['path'] = 'filefield/progress/' . $upload_progress_key;
  }

  // Set the FID.
  $element['fid'] = array(
    '#type' => 'hidden',
    '#value' => isset($item['fid']) ? $item['fid'] : 0,
  );

  if (!empty($item['fid'])) {
    if (variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PRIVATE && function_exists('file_download_access') && !empty($item['status']) && !file_download_access($item['filepath'])) {
      $element['preview'] = array(
        '#type' => 'markup',
        '#value' => t('Access denied'),
      );
    }
    else {
      $element['preview'] = array(
        '#type' => 'markup',
        '#value' => theme('filefield_widget_preview', $item),
      );
    }
  }

  // Grant access to temporary files.
  if (!empty($item['fid']) && isset($item['status']) && $item['status'] == 0 && variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PRIVATE) {
    $_SESSION['filefield_access'][] = $item['fid'];
  }

  // placeholder.. will be serialized into the data column. this is a place for widgets
  // to put additional data.
  $element['data'] = array(
    '#tree' => 'true',
    '#access' => !empty($item['fid']),
  );

  if (!empty($field['description_field'])) {
    $element['data']['description'] = array(
      '#title' => t('Description'),
      '#value' => isset($item['data']['description']) ? $item['data']['description'] : '',
      '#type' => variable_get('filefield_description_type', 'textfield'),
      '#maxlength' => variable_get('filefield_description_length', 128),
    );
  }

  if (!empty($field['list_field'])) {
    $element['list'] = array(
      '#type' => empty($item['fid']) ? 'hidden' : 'checkbox',
      '#title' => t('List'),
      '#value' => isset($item['list']) && !empty($item['fid']) ? $item['list'] : $field['list_default'],
      '#attributes' => array('class' => 'filefield-list'),
    );
  }
  else {
    $element['list'] = array(
      '#type' => 'hidden',
      '#value' => '1',
    );
  }

  foreach ($element['#upload_validators'] as $callback => $arguments) {
    $help_func = $callback .'_help';
    if (function_exists($help_func)) {
      $desc[] = call_user_func_array($help_func, $arguments);
    }
  }

  $element['upload'] = array(
    '#name' => 'files[' . implode('_', $element['#array_parents']) . ']',
    '#type' => 'file',
    '#description' => implode('<br />', $desc),
    '#size' => 22,
    '#access' => empty($item['fid']),
  );

  $element['#attributes']['id'] = $element['#id'] .'-ahah-wrapper';
  $element['#prefix'] = '<div '. drupal_attributes($element['#attributes']) .'>';
  $element['#suffix'] = '</div>';

  return $element;
}

function filefield_widget_after_build($element, &$form_state) {
  // Set access on the buttons.
  $element['filefield_upload']['#access'] = empty($element['fid']['#value']);
  $element['filefield_remove']['#access'] = !empty($element['fid']['#value']);
  return $element;
}

/**
 * An #element_validate callback for the filefield_widget field.
 */
function filefield_widget_validate(&$element, &$form_state) {
  // If referencing an existing file, only allow if there are existing
  // references. This prevents unmanaged files (outside of FileField) from
  // being deleted if this node were to be deleted.
  if (!empty($element['fid']['#value'])) {
    $field = content_fields($element['#field_name'], $element['#type_name']);
    if ($file = field_file_load($element['fid']['#value'])) {
      $file = (object) $file;
      if ($file->status == FILE_STATUS_PERMANENT) {
        if (field_file_references($file) == 0 || (variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PRIVATE && function_exists('file_download_access') && !file_download_access($file->filepath))) {
          form_error($element, t('Referencing to the file used in the %field field is not allowed.', array('%field' => $element['#title'])));
        }
      }
    }
    else {
      form_error($element, t('The file referenced by the %field field does not exist.', array('%field' => $element['#title'])));
    }
  }
}

/**
 * FormAPI theme function. Theme the output of an image field.
 */
function theme_filefield_widget($element) {
  $element['#id'] .= '-upload'; // Link the label to the upload field.
  return theme('form_element', $element, $element['#children']);
}

function theme_filefield_widget_preview($item) {
  // Remove the current description so that we get the filename as the link.
  if (isset($item['data']) && is_array($item['data']) && isset($item['data']['description'])) {
    unset($item['data']['description']);
  }

  return '<div class="filefield-file-info">'.
           '<div class="filename">'. theme('filefield_file', $item) .'</div>'.
           '<div class="filesize">'. format_size($item['filesize']) .'</div>'.
           '<div class="filemime">'. $item['filemime'] .'</div>'.
         '</div>';
}

function theme_filefield_widget_item($element) {
  // Put the upload button directly after the upload field.
  $element['upload']['#field_suffix'] = drupal_render($element['filefield_upload']);
  $element['upload']['#theme'] = 'filefield_widget_file';

  $output = '';
  $output .= '<div class="filefield-element clear-block">';

  if ($element['fid']['#value'] != 0) {
    $output .= '<div class="widget-preview">';
    $output .= drupal_render($element['preview']);
    $output .= '</div>';
  }

  $output .= '<div class="widget-edit">';
  $output .=  drupal_render($element);
  $output .= '</div>';
  $output .= '</div>';

  return $output;
}

/**
 * Custom theme function for FileField upload elements.
 *
 * This function allows us to put the "Upload" button immediately after the
 * file upload field by respecting the #field_suffix property.
 */
function theme_filefield_widget_file($element) {
  $output = '';

  $output .= '<div class="filefield-upload clear-block">';

  if (isset($element['#field_prefix'])) {
    $output .= $element['#field_prefix'];
  }

  _form_set_class($element, array('form-file'));
  $output .= '<input type="file" name="'. $element['#name'] .'"'. ($element['#attributes'] ? ' '. drupal_attributes($element['#attributes']) : '') .' id="'. $element['#id'] .'" size="'. $element['#size'] ."\" />\n";

  if (isset($element['#field_suffix'])) {
    $output .= $element['#field_suffix'];
  }

  $output .= '</div>';

  return theme('form_element', $element, $output);
}

/**
 * Additional #validate handler for the node form.
 *
 * This function checks the #required properties on file fields and calculates
 * node upload totals for all file fields. The #required property is not
 * properly supported on file fields by Drupal core, so we do this manually.
 */
function filefield_node_form_validate($form, &$form_state) {
  foreach ($form['#field_info'] as $field_name => $field) {
    if (!(in_array($field['module'], array('imagefield', 'filefield')))) continue;
    $empty = $field['module'] .'_content_is_empty';
    $valid = FALSE;
    $total_filesize = 0;
    if (!empty($form_state['values'][$field_name])) {
      foreach ($form_state['values'][$field_name] as $delta => $item) {
        if ($empty($item, $field)) continue;
        $valid = TRUE;
        $total_filesize += (int)$item['filesize'];
      }
    }

    if (!$valid && $field['required'] && filefield_edit_access($field['type_name'], $field_name, $form['#node'])) {
      form_set_error($field_name, t('%title field is required.', array('%title' => t($field['widget']['label']))));
    }
    $max_filesize = parse_size($field['widget']['max_filesize_per_node']);
    if ($max_filesize && $total_filesize > $max_filesize) {
      form_set_error($field_name, t('Total filesize for %title, %tsize, exceeds field settings of %msize.',
                                    array(
                                      '%title' => t($field['widget']['label']),
                                      '%tsize' => format_size($total_filesize),
                                      '%msize' => format_size($max_filesize)
                                    )
                                  ));
    }
  }
}
