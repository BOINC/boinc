<?php

/**
 * @file
 * hook_file and imagefield file functions.
 */

/**
 * Implementation of hook_file_insert().
 */
function imagefield_file_insert($file) {
  // Currently empty. Thumbnails are now generated on preview.
}

/**
 * Implementation of hook_file_delete().
 *
 * Delete the admin thumbnail when the original is deleted.
 */
function imagefield_file_delete($file) {
  if (imagefield_file_is_image($file)) {
    file_delete(imagefield_file_admin_thumb_path($file, FALSE));
  }
}

/**
 * Implementation of hook_file_references().
 */
function imagefield_file_references($file) {
  $file = (object) $file;
  $count = 0;

  // We currently only check if images are in use when deleting from content
  // type default images. All other reference counting is left to FileField.
  if (isset($file->imagefield_type_name)) {
    $content_types = content_types();
    foreach ($content_types as $type_name => $content_type) {
      if ($type_name != $file->imagefield_type_name) {
        foreach ($content_type['fields'] as $field_name => $field) {
          if ($field['widget']['default_image'] && $field['widget']['default_image']['fid'] == $file->fid) {
            $count++;
          }
        }
      }
    }
  }
  return $count ? array('imagefield' => $count) : NULL;
}

/**
 * Simple utility function to check if a file is an image.
 */
function imagefield_file_is_image($file) {
  $file = (object)$file;
  return in_array($file->filemime, array('image/jpg', 'image/pjpeg', 'image/jpeg', 'image/png', 'image/gif'));
}

/**
 * Given a file, return the path the image thumbnail used while editing.
 */
function imagefield_file_admin_thumb_path($file, $create_thumb = TRUE) {
  $file = (object)$file;
  $short_path = preg_replace('/^' . preg_quote(file_directory_path(), '/') . '/', '', $file->filepath);
  $filepath = file_directory_path() . '/imagefield_thumbs' . $short_path;

  if ($create_thumb) {
    imagefield_create_admin_thumb($file->filepath, $filepath);
  }

  return $filepath;
}

/**
 * Create a thumbnail to be shown while editing an image.
 */
function imagefield_create_admin_thumb($source, $destination) {
  if (!is_file($source)) {
    return FALSE;
  }

  $info = image_get_info($source);
  $size = explode('x', variable_get('imagefield_thumb_size', '100x100'));

  // Check if the destination image needs to be regenerated to match a new size.
  if (is_file($destination)) {
    $thumb_info = image_get_info($destination);
    if ($thumb_info['width'] != $size[0] && $thumb_info['height'] != $size[1] && ($info['width'] > $size[0] || $info['height'] > $size[1])) {
      unlink($destination);
    }
    else {
      return;
    }
  }

  // Ensure the destination directory exists and is writable.
  $directories = explode('/', $destination);
  array_pop($directories); // Remove the file itself.
  // Get the file system directory.
  $file_system = file_directory_path();
  foreach ($directories as $directory) {
    $full_path = isset($full_path) ? $full_path . '/' . $directory : $directory;
    // Don't check directories outside the file system path.
    if (strpos($full_path, $file_system) === 0) {
      field_file_check_directory($full_path, FILE_CREATE_DIRECTORY);
    }
  }

  // Create the thumbnail.
  if ($info['width'] <= $size[0] && $info['height'] <= $size[1]) {
    file_copy($source, $destination);
  }
  // Use ImageAPI, if installed.
  elseif (module_exists('imageapi') && imageapi_default_toolkit()) {
    $res = imageapi_image_open($source);
    imageapi_image_scale($res, $size[0], $size[1]);
    imageapi_image_close($res, $destination);
  }
  elseif (image_get_toolkit() && image_scale($source, $destination, $size[0], $size[1])) {
    // Set permissions. This is done for us when using file_copy().
    @chmod($destination, 0664);
  }
  else {
    drupal_set_message(t('An image thumbnail was not able to be created.'), 'error');
  }
}
