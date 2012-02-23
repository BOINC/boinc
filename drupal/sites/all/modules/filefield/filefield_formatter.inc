<?php
// $Id: filefield_formatter.inc,v 1.20 2010/06/29 23:49:11 quicksketch Exp $
/**
 * @file
 * FileField: Defines a CCK file field type.
 *
 * Uses content.module to store the fid and field specific metadata,
 * and Drupal's {files} table to store the actual file data.
 *
 * This file contains CCK formatter related functionality.
 */

/**
 * Theme function for the 'default' filefield formatter.
 */
function theme_filefield_formatter_default($element) {
  $file = $element['#item'];
  $field = content_fields($element['#field_name']);
  $output = theme('filefield_item', $file, $field);
  return $output;
}

/**
 * Theme function for the 'path_plain' formatter.
 */
function theme_filefield_formatter_path_plain($element) {
  // Inside a View this function may be called with null data. In that case,
  // just return.
  if (empty($element['#item'])) {
    return '';
  }

  $field = content_fields($element['#field_name']);
  $item = $element['#item'];
  // If there is no image on the database, use default.
  if (empty($item['fid']) && $field['use_default_file']) {
    $item = $field['default_file'];
  }
  if (empty($item['filepath']) && !empty($item['fid'])) {
    $item = array_merge($item, field_file_load($item['fid']));
  }
  return empty($item['filepath']) ? '' : check_plain(file_create_path($item['filepath']));
}

/**
 * Theme function for the 'url_plain' formatter.
 */
function theme_filefield_formatter_url_plain($element) {
  // Inside a View this function may be called with null data. In that case,
  // just return.
  if (empty($element['#item'])) {
    return '';
  }

  $field = content_fields($element['#field_name']);
  $item = $element['#item'];
  // If there is no image on the database, use default.
  if (empty($item['fid']) && $field['use_default_file']) {
    $item = $field['default_file'];
  }
  if (empty($item['filepath']) && !empty($item['fid'])) {
    $item = array_merge($item, field_file_load($item['fid']));
  }

  if (empty($item['filepath'])) {
    return '';
  }

  // Encode the parts of the path to ensure URLs operate within href attributes.
  // Private file paths are urlencoded for us inside of file_create_url().
  if (variable_get('file_downloads', FILE_DOWNLOADS_PUBLIC) == FILE_DOWNLOADS_PUBLIC) {
    $parts = explode('/', $item['filepath']);
    foreach ($parts as $index => $part) {
      $parts[$index] = rawurlencode($part);
    }
    $item['filepath'] = implode('/', $parts);
  }

  return file_create_url($item['filepath']);
}

/**
 * Theme function for any file that is managed by FileField.
 *
 * It doesn't really format stuff by itself but rather redirects to other
 * formatters that are telling us they want to handle the concerned file.
 *
 * This function checks if the file may be shown and returns an empty string
 * if viewing the file is not allowed for any reason. If you need to display it
 * in any case, please use theme('filefield_file') instead.
 */
function theme_filefield_item($file, $field) {
  if (filefield_view_access($field['field_name']) && filefield_file_listed($file, $field)) {
    return theme('filefield_file', $file);
  }
  return '';
}

/**
 * Return whether a file should be listed when viewing the node.
 *
 * @param $file
 *   A populated FileField item.
 * @param $field
 *   A CCK field instance array.
 */
function filefield_file_listed($file, $field) {
  if (!empty($field['list_field'])) {
    return !empty($file['list']);
  }
  return TRUE;
}

/**
 * Theme function for the 'generic' single file formatter.
 */
function theme_filefield_file($file) {
  // Views may call this function with a NULL value, return an empty string.
  if (empty($file['fid'])) {
    return '';
  }

  $path = $file['filepath'];
  $url = file_create_url($path);
  $icon = theme('filefield_icon', $file);

  // Set options as per anchor format described at
  // http://microformats.org/wiki/file-format-examples
  // TODO: Possibly move to until I move to the more complex format described 
  // at http://darrelopry.com/story/microformats-and-media-rfc-if-you-js-or-css
  $options = array(
    'attributes' => array(
      'type' => $file['filemime'] . '; length=' . $file['filesize'],
    ),
  );

  // Use the description as the link text if available.
  if (empty($file['data']['description'])) {
    $link_text = $file['filename'];
  }
  else {
    $link_text = $file['data']['description'];
    $options['attributes']['title'] = $file['filename'];
  }

  return '<div class="filefield-file">'. $icon . l($link_text, $url, $options) .'</div>';
}
