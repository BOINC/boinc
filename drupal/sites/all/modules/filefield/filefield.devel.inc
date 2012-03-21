<?php

/**
 * @file
 * Utility functions for generating FileField content. Note that image
 * generation support requires the GD toolkit.
 */

/**
 * Private function used by filefield_content_generate().
 */
function _filefield_content_generate($node, $field) {
  if ($source = _filefield_generate_file($field)) {
    $file = field_file_save_file($source, array(), filefield_widget_file_path($field));

    $item = (array) $file;
    $item['list'] = 1;
    $item['data']['alt'] = devel_create_greeking(4);
    $item['data']['title'] = devel_create_greeking(10);
  }
  else {
    $item = array();
  }

  return $item;
}

/**
 * Generate an image based on the properties of a field.
 *
 * This is made to work with ImageField, and inspects the minimum and maximum
 * image sizes and makes sure the generated image matches the requirements.
 *
 * @return
 *   The path to the new file, in the temporary directory.
 */
function _filefield_generate_file($field) {
  if (empty($field['widget']['file_extensions'])) {
    $field['widget']['file_extensions'] = 'png jpg txt';
  }

  $extensions = array_intersect(explode(' ', $field['widget']['file_extensions']), array('png', 'jpg', 'txt'));
  $extension = array_rand(drupal_map_assoc($extensions));

  if ($extension == 'txt') {
    $filesize = empty($field['widget']['max_filesize_per_file']) ? 1024 : parse_size($field['widget']['max_filesize_per_file']);
    return _filefield_generate_textfile($filesize);
  }
  elseif (in_array($extension, array('png', 'jpg')) && function_exists('imagecreate')) {
    $min_resolution = empty($field['widget']['min_resolution']) ? '100x100' : $field['widget']['min_resolution'];
    $max_resolution = empty($field['widget']['max_resolution']) ? '600x600' : $field['widget']['max_resolution'];
    return _filefield_generate_image($extension, $min_resolution, $max_resolution);
  }

  return FALSE;
}

/**
 * Private function for generating a random text file.
 */
function _filefield_generate_textfile($filesize = 1024) {
  static $filesizes = array();

  $temp_file = FALSE;
  if (isset($filesizes[$filesize])) {
    $temp_file = $filesizes[$filesize];
  }
  elseif ($temp_file = tempnam(file_directory_temp(), 'filefield_')) {
    file_move($temp_file, $temp_file .'.txt');

    $fp = fopen($temp_file, 'w');
    fwrite($fp, str_repeat('01', $filesize/2));
    fclose($fp);
    $filesizes[$filesize] = $temp_file;
  }

  return $temp_file;
}

/**
 * Private function for creating a random image.
 *
 * This function only works with the GD toolkit. ImageMagick is not supported.
 */
function _filefield_generate_image($extension = 'png', $min_resolution, $max_resolution) {
  static $images = array();

  // Generate a max of 5 different images.
  if (!isset($images[$extension][$min_resolution][$max_resolution]) || count($images[$extension][$min_resolution][$max_resolution]) < 5) {
    if ($temp_file = tempnam(file_directory_temp(), 'filefield_')) {
      file_move($temp_file, $temp_file .'.'. $extension);

      $min = explode('x', $min_resolution);
      $max = explode('x', $max_resolution);

      $width = rand((int)$min[0], (int)$max[0]);
      $height = rand((int)$min[0], (int)$max[0]);

      // Make a image split into 4 sections with random colors.
      $im = imagecreate($width, $height);
      for ($n = 0; $n < 4; $n++) {
        $color = imagecolorallocate($im, rand(0, 255), rand(0, 255), rand(0, 255));
        $x = $width/2 * ($n % 2);
        $y = $height/2 * (int) ($n >= 2);
        imagefilledrectangle($im, $x, $y, $x + $width/2, $y + $height/2, $color);
      }

      // Make a perfect circle in the image middle.
      $color = imagecolorallocate($im, rand(0, 255), rand(0, 255), rand(0, 255));
      $smaller_dimension = min($width, $height);
      $smaller_dimension = ($smaller_dimension % 2) ? $smaller_dimension : $smaller_dimension;
      imageellipse($im, $width/2, $height/2, $smaller_dimension, $smaller_dimension, $color);

      $save_function = 'image'. ($extension == 'jpg' ? 'jpeg' : $extension);
      $save_function($im, $temp_file);

      $images[$extension][$min_resolution][$max_resolution][$temp_file] = $temp_file;
    }
  }
  // Select one of the images we've already generated for this field.
  else {
    $temp_file = array_rand($images[$extension][$min_resolution][$max_resolution]);
  }

  return $temp_file;
}
