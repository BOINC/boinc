<?php

/**
 * @file
 * ImageField formatter hooks and callbacks.
 */

function theme_imagefield_formatter_image_plain($element) {
  // Inside a view $element may contain null data. In that case, just return.
  if (empty($element['#item']['fid'])) {
    return '';
  }

  $field = content_fields($element['#field_name']);
  $item = $element['#item'];

  $item['data']['alt'] = isset($item['data']['alt']) ? $item['data']['alt'] : '';
  $item['data']['title'] = isset($item['data']['title']) ? $item['data']['title'] : NULL;

  $class = 'imagefield imagefield-'. $field['field_name'];
  return  theme('imagefield_image', $item, $item['data']['alt'], $item['data']['title'], array('class' => $class));
}

function theme_imagefield_formatter_image_nodelink($element) {
  // Inside a view $element may contain null data. In that case, just return.
  if (empty($element['#item']['fid'])) {
    return '';
  }

  $node = $element['#node'];
  $imagetag = theme('imagefield_formatter_image_plain', $element);
  $class = 'imagefield imagefield-nodelink imagefield-'. $element['#field_name'];
  return l($imagetag, 'node/'. $node->nid, array('attributes' => array('class' => $class), 'html' => TRUE));
}

function theme_imagefield_formatter_image_imagelink($element) {
  // Inside a view $element may contain null data. In that case, just return.
  if (empty($element['#item']['fid'])) {
    return '';
  }

  $item = $element['#item'];
  $imagetag = theme('imagefield_formatter_image_plain', $element);
  $original_image_url = file_create_url($item['filepath']);
  $class = 'imagefield imagefield-imagelink imagefield-'. $element['#field_name'];
  return l($imagetag, $original_image_url, array('attributes' => array('class' => $class), 'html' => TRUE));
}
